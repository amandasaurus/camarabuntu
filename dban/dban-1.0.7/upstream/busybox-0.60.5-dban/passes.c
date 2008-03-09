/*
 *  passes.c: Routines that read and write patterns to block devices.
 *
 *  Copyright Darik Horn <dajhorn-dban@vanadac.com>.
 *  
 *  This program is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free Software
 *  Foundation, version 2.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 *  Ave, Cambridge, MA 02139, USA.
 *
 */


#include "dwipe.h"



int dwipe_random_verify( DWIPE_METHOD_SIGNATURE, dwipe_entropy_t* entropy )
{
/**
 * Verifies that a random pass was correctly written to the device.
 *
 */

	/* The result holder. */
	int r;

	/* The result buffer for calls to lseek. */
	off_t offset;

	/* The input buffer. */
	char* b;

	/* The pattern buffer that is used to check the input buffer. */
	char* d;

	/* A pointer into the output buffer. */
	u32* p;

	/* A pointer into the pattern buffer. */
	u32* q;

	/* The number of bytes remaining in the pass. */
	u64 z = c->device_size;

	if( entropy == NULL )
	{
		/* Caught insanity. */	
		dwipe_log( DWIPE_LOG_SANITY, "dwipe_random_verify: Null entropy pointer." );
		return -1;
	}

	if( entropy->length <= 0 )
	{
		/* Caught insanity. */	
		dwipe_log( DWIPE_LOG_SANITY, "dwipe_random_verify: The entropy length member is %i.", entropy->length );
		return -1;
	}

	/* Create the input buffer. */
	b = malloc( c->device_stat.st_blksize );

	/* Check the memory allocation. */
	if( ! b )
	{
		dwipe_perror( errno, "dwipe_random_verify: malloc:" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to allocate memory for the input buffer." );
		return -1;
	}

	/* Create the pattern buffer */
	d = malloc( c->device_stat.st_blksize );

	/* Check the memory allocation. */
	if( ! d )
	{
		dwipe_perror( errno, "dwipe_random_verify: malloc:" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to allocate memory for the pattern buffer." );
		return -1;
	}

	/* Reset the file pointer. */
	offset = lseek( c->device_fd, 0, SEEK_SET );

	if( offset == (off_t)-1 )
	{
		dwipe_perror( errno, "dwipe_random_verify: lseek:" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to reset the '%s' file offset.", c->device_name );
		return -1;
	}

	if( offset != 0 )
	{
		/* This is system insanity. */
		dwipe_log( DWIPE_LOG_SANITY, "dwipe_random_verify: lseek() returned a bogus offset on '%s'.", c->device_name );
		return -1;
	}

	/* Tell our parent that we are syncing the device. */
	c->sync_status = 1;

	/* Sync the device. */
	r = fdatasync( c->device_fd );

	/* Tell our parent that we have finished syncing the device. */
	c->sync_status = 0;

	if( r != 0 )
	{
		/* FIXME: Is there a better way to handle this? */
		dwipe_perror( errno, "dwipe_random_verify: fdatasync:" );
		dwipe_log( DWIPE_LOG_WARNING, "Buffer flush failure on '%s'.", c->device_name );
	}

	/* Seed the PRNG. */
	init_by_array( (u32*)( entropy->s ), entropy->length / sizeof( u32 ) );


	while( z > 0 )
	{

		for( q = (u32*)d ; q < (u32*)( d + c->device_stat.st_blksize ) ; q++ )
		{
			/* Fill the output buffer with the random pattern. */
			*q = genrand_int32();

			/* TODO: Check whether the compiler is actually incrementing p by sizeof(p). */
		}

		/* Read the buffer in from the device. */
		r = read( c->device_fd, b, MIN( z, c->device_stat.st_blksize ) );

		/* Check the result. */
		if( r < 0 )
		{
			dwipe_perror( errno, "dwipe_random_verify: read:" );
			dwipe_log( DWIPE_LOG_ERROR, "Unable to read from '%s'.", c->device_name );
			return -1;
		}

		/* Check for a partial read. */
		if( r != MIN( z, c->device_stat.st_blksize ) )
		{
			/* TODO: Handle a partial read. */

			/* The number of bytes that were not read. */
			int s = MIN( z, c->device_stat.st_blksize ) - r;

			dwipe_log( DWIPE_LOG_WARNING, "Partial read from '%s', %i bytes short.", c->device_name, s );
			
			/* Increment the error count. */
			c->verify_errors += s;

			/* Bump the file pointer to the next block. */
			offset = lseek( c->device_fd, s, SEEK_CUR );

			if( offset == (off_t)-1 )
			{
				dwipe_perror( errno, "dwipe_random_verify: lseek:" );
				dwipe_log( DWIPE_LOG_ERROR, "Unable to bump the '%s' file offset after a partial read.", c->device_name );
				return -1;
			}

		} /* partial read */

		/* Reset the input buffer pointer. */
		p = (u32*)b;

		/* Reset the pattern buffer pointer. */
		q = (u32*)d;

		/* Check every byte in the buffer. */
		while( p < (u32*)( b + r ) )
		{
			if( *p++ != *q++ ) { c->verify_errors += 1; }
		}

		/* Decrement the bytes remaining in this pass. */
		z -= r;

		/* Increment the total progress counters. */
		c->round_done += r;
		c->pass_done += r;

	} /* while bytes remaining */

	/* Release the buffers. */
	free( b );
	free( d );

			
	/* We're done. */
	return 0;
		
} /* dwipe_random_verify */



int dwipe_random_pass( DWIPE_METHOD_SIGNATURE, dwipe_entropy_t* entropy )
{
/**
 * Writes a random pattern to the device.
 *
 */

	/* The result holder. */
	int r;

	/* The result buffer for calls to lseek. */
	off_t offset;

	/* The output buffer. */
	char* b;

	/* A pointer into the output buffer. */
	u32* p;

	/* The number of bytes remaining in the pass. */
	u64 z = c->device_size;

	if( entropy == NULL )
	{
		/* Caught insanity. */	
		dwipe_log( DWIPE_LOG_SANITY, "dwipe_random_pass: Null entropy pointer." );
		return -1;
	}

	if( entropy->length <= 0 )
	{
		/* Caught insanity. */	
		dwipe_log( DWIPE_LOG_SANITY, "dwipe_random_pass: The entropy length member is %i.", entropy->length );
		return -1;
	}


	/* Create the output buffer. */
	b = malloc( c->device_stat.st_blksize );

	/* Check the memory allocation. */
	if( ! b )
	{
		dwipe_perror( errno, "dwipe_random_pass: malloc:" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to allocate memory for the output buffer." );
		return -1;
	}

	/* Seed the PRNG. */
	init_by_array( (u32*)( entropy->s ), entropy->length / sizeof( u32 ) );

	/* Reset the file pointer. */
	offset = lseek( c->device_fd, 0, SEEK_SET );

	if( offset == (off_t)-1 )
	{
		dwipe_perror( errno, "dwipe_random_pass: lseek:" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to reset the '%s' file offset.", c->device_name );
		return -1;
	}

	if( offset != 0 )
	{
		/* This is system insanity. */
		dwipe_log( DWIPE_LOG_SANITY, "dwipe_random_pass: lseek() returned a bogus offset on '%s'.", c->device_name );
		return -1;
	}


	while( z > 0 )
	{

		for( p = (u32*)b ; p < (u32*)( b + c->device_stat.st_blksize ) ; p++ )
		{
			/* Fill the output buffer with the random pattern. */
			*p = genrand_int32();

			/* TODO: Check whether the compiler is actually incrementing p by sizeof(p). */
		}

		/* Write the next block out to the device. */
		r = write( c->device_fd, b, MIN( z, c->device_stat.st_blksize ) );

		/* Check the result for a fatal error. */
		if( r < 0 )
		{
			dwipe_perror( errno, "dwipe_random_pass: write:" );
			dwipe_log( DWIPE_LOG_FATAL, "Unable to read from '%s'.", c->device_name );
			return -1;
		}

		/* Check for a partial write. */
		if( r != MIN( z, c->device_stat.st_blksize ) )
		{
			/* TODO: Handle a partial write. */

			/* The number of bytes that were not written. */
			int s = MIN( z, c->device_stat.st_blksize ) - r;
			
			/* Increment the error count. */
			c->pass_errors += s;

			dwipe_log( DWIPE_LOG_WARNING, "Partial write on '%s', %i bytes short.", c->device_name, s );

			/* Bump the file pointer to the next block. */
			offset = lseek( c->device_fd, s, SEEK_CUR );

			if( offset == (off_t)-1 )
			{
				dwipe_perror( errno, "dwipe_random_pass: lseek:" );
				dwipe_log( DWIPE_LOG_ERROR, "Unable to bump the '%s' file offset after a partial write.", c->device_name );
				return -1;
			}

		} /* partial write */

		/* Decrement the bytes remaining in this pass. */
		z -= r;

		/* Increment the total progress counters. */
		c->round_done += r;
		c->pass_done += r;

	} /* remaining bytes */

	/* Release the output buffer. */
	free( b );

	/* Tell our parent that we are syncing the device. */
	c->sync_status = 1;

	/* Sync the device. */
	r = fdatasync( c->device_fd );

	/* Tell our parent that we have finished syncing the device. */
	c->sync_status = 0;

	if( r != 0 )
	{
		/* FIXME: Is there a better way to handle this? */
		dwipe_perror( errno, "dwipe_random_pass: fdatasync:" );
		dwipe_log( DWIPE_LOG_WARNING, "Buffer flush failure on '%s'.", c->device_name );
	}

	/* We're done. */
	return 0;	

} /* dwipe_random_pass */



int dwipe_static_verify( DWIPE_METHOD_SIGNATURE, dwipe_pattern_t* pattern )
{
/**
 * Verifies that a static pass was correctly written to the device.
 *
 */

	/* The result holder. */
	int r;

	/* The result buffer for calls to lseek. */
	off_t offset;

	/* The input buffer. */
	char* b;

	/* The pattern buffer that is used to check the input buffer. */
	char* d;

	/* A pointer into the output buffer. */
	char* p;

	/* A pointer into the pattern buffer. */
	char* q;

	/* The pattern buffer window offset. */
	int w = 0;

	/* The number of bytes remaining in the pass. */
	u64 z = c->device_size;

	if( pattern == NULL )
	{
		/* Caught insanity. */	
		dwipe_log( DWIPE_LOG_SANITY, "dwipe_static_verify: Null entropy pointer." );
		return -1;
	}

	if( pattern->length <= 0 )
	{
		/* Caught insanity. */	
		dwipe_log( DWIPE_LOG_SANITY, "dwipe_static_verify: The pattern length member is %i.", pattern->length );
		return -1;
	}

	/* Create the input buffer. */
	b = malloc( c->device_stat.st_blksize );

	/* Check the memory allocation. */
	if( ! b )
	{
		dwipe_perror( errno, "dwipe_static_verify: malloc:" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to allocate memory for the input buffer." );
		return -1;
	}

	/* Create the pattern buffer */
	d = malloc( c->device_stat.st_blksize + pattern->length * 2 );

	/* Check the memory allocation. */
	if( ! d )
	{
		dwipe_perror( errno, "dwipe_static_verify: malloc:" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to allocate memory for the pattern buffer." );
		return -1;
	}

	for( q = d ; q < d + c->device_stat.st_blksize + pattern->length ; q += pattern->length )
	{
		/* Fill the pattern buffer with the pattern. */
		memcpy( q, pattern->s, pattern->length );
	}

	/* Tell our parent that we are syncing the device. */
	c->sync_status = 1;

	/* Sync the device. */
	r = fdatasync( c->device_fd );

	/* Tell our parent that we have finished syncing the device. */
	c->sync_status = 0;

	if( r != 0 )
	{
		/* FIXME: Is there a better way to handle this? */
		dwipe_perror( errno, "dwipe_static_verify: fdatasync:" );
		dwipe_log( DWIPE_LOG_WARNING, "Buffer flush failure on '%s'.", c->device_name );
	}


	/* Reset the file pointer. */
	offset = lseek( c->device_fd, 0, SEEK_SET );

	if( offset == (off_t)-1 )
	{
		dwipe_perror( errno, "dwipe_static_verify: lseek:" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to reset the '%s' file offset.", c->device_name );
		return -1;
	}

	if( offset != 0 )
	{
		/* This is system insanity. */
		dwipe_log( DWIPE_LOG_SANITY, "dwipe_static_verify: lseek() returned a bogus offset on '%s'.", c->device_name );
		return -1;
	}


	while( z > 0 )
	{
		/* Read the buffer in from the device. */
		r = read( c->device_fd, b, MIN( z, c->device_stat.st_blksize ) );

		/* Check the result. */
		if( r < 0 )
		{
			dwipe_perror( errno, "dwipe_static_verify: read:" );
			dwipe_log( DWIPE_LOG_ERROR, "Unable to read from '%s'.", c->device_name );
			return -1;
		}

		/* Check for a partial read. */
		if( r != MIN( z, c->device_stat.st_blksize ) )
		{
			/* TODO: Handle a partial read. */

			/* The number of bytes that were not read. */
			int s = MIN( z, c->device_stat.st_blksize ) - r;
			
			/* Increment the error count. */
			c->verify_errors += s;

			dwipe_log( DWIPE_LOG_WARNING, "Partial read on '%s', %i bytes short.", c->device_name, s );

			/* Bump the file pointer to the next block. */
			offset = lseek( c->device_fd, s, SEEK_CUR );

			if( offset == (off_t)-1 )
			{
				dwipe_perror( errno, "dwipe_static_verify: lseek:" );
				dwipe_log( DWIPE_LOG_ERROR, "Unable to bump the '%s' file offset after a partial read.", c->device_name );
				return -1;
			}

		} /* partial read */

		/* Reset the input buffer pointer. */
		p = b;

		/* Reset the pattern buffer pointer. */
		q = &d[w];

		/* Check every byte in the buffer. */
		while( p < b + r )
		{
			if( *p++ != *q++ ) { c->verify_errors += 1; }
		}

		/* Adjust the window. */
		w = ( c->device_stat.st_blksize + w ) % pattern->length;

		/* Intuition check: 
		 *   If the pattern length evenly divides the block size
		 *   then ( w == 0 ) always.
		 */

		/* Decrement the bytes remaining in this pass. */
		z -= r;

		/* Increment the total progress counters. */
		c->round_done += r;
		c->pass_done += r;

	} /* while bytes remaining */


	/* Release the buffers. */
	free( b );
	free( d );

	/* We're done. */
	return 0;
		
} /* dwipe_static_verify */



int dwipe_static_pass( DWIPE_METHOD_SIGNATURE, dwipe_pattern_t* pattern )
{
/**
 * Writes a static pattern to the device.
 *
 */

	/* The result holder. */
	int r;

	/* The result buffer for calls to lseek. */
	off_t offset;

	/* The output buffer. */
	char* b;

	/* A pointer into the output buffer. */
	char* p;

	/* The output buffer window offset. */
	int w = 0;

	/* The number of bytes remaining in the pass. */
	u64 z = c->device_size;

	if( pattern == NULL )
	{
		/* Caught insanity. */	
		dwipe_log( DWIPE_LOG_SANITY, "dwipe_static_pass: Null pattern pointer." );
		return -1;
	}

	if( pattern->length <= 0 )
	{
		/* Caught insanity. */	
		dwipe_log( DWIPE_LOG_SANITY, "dwipe_static_pass: The pattern length member is %i.", pattern->length );
		return -1;
	}

	/* Create the output buffer. */
	b = malloc( c->device_stat.st_blksize + pattern->length * 2 );

	/* Check the memory allocation. */
	if( ! b )
	{
		dwipe_perror( errno, "dwipe_static_pass: malloc:" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to allocate memory for the pattern buffer." );
		return -1;
	}

	for( p = b ; p < b + c->device_stat.st_blksize + pattern->length ; p += pattern->length )
	{
		/* Fill the output buffer with the pattern. */
		memcpy( p, pattern->s, pattern->length ); 
	}


	/* Reset the file pointer. */
	offset = lseek( c->device_fd, 0, SEEK_SET );

	if( offset == (off_t)-1 )
	{
		dwipe_perror( errno, "dwipe_static_pass: lseek:" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to reset the '%s' file offset.", c->device_name );
		return -1;
	}

	if( offset != 0 )
	{
		/* This is system insanity. */
		dwipe_log( DWIPE_LOG_SANITY, "dwipe_static_pass: lseek() returned a bogus offset on '%s'.", c->device_name );
		return -1;
	}


	while( z > 0 )
	{
		/* Write the next block out to the device. */
		r = write( c->device_fd, &b[w], MIN( z, c->device_stat.st_blksize ) );

		/* Check the result for a fatal error. */
		if( r < 0 )
		{
			dwipe_perror( errno, "dwipe_static_pass: write:" );
			dwipe_log( DWIPE_LOG_FATAL, "Unable to write to '%s'.", c->device_name );
			return -1;
		}

		/* Check for a partial write. */
		if( r != MIN( z, c->device_stat.st_blksize ) )
		{
			/* TODO: Handle a partial write. */

			/* The number of bytes that were not written. */
			int s = MIN( z, c->device_stat.st_blksize ) - r;
			
			/* Increment the error count. */
			c->pass_errors += s;

			dwipe_log( DWIPE_LOG_WARNING, "Partial write on '%s', %i bytes short.", c->device_name, s );

			/* Bump the file pointer to the next block. */
			offset = lseek( c->device_fd, s, SEEK_CUR );

			if( offset == (off_t)-1 )
			{
				dwipe_perror( errno, "dwipe_static_pass: lseek:" );
				dwipe_log( DWIPE_LOG_ERROR, "Unable to bump the '%s' file offset after a partial write.", c->device_name );
				return -1;
			}

		} /* partial write */


		/* Adjust the window. */
		w = ( c->device_stat.st_blksize + w ) % pattern->length;

		/* Intuition check: 
		 *
		 *   If the pattern length evenly divides the block size
		 *   then ( w == 0 ) always.
		 */

		/* Decrement the bytes remaining in this pass. */
		z -= r;

		/* Increment the total progress counterr. */
		c->round_done += r;
		c->pass_done += r;

	} /* remaining bytes */

	/* Tell our parent that we are syncing the device. */
	c->sync_status = 1;

	/* Sync the device. */
	r = fdatasync( c->device_fd );

	/* Tell our parent that we have finished syncing the device. */
	c->sync_status = 0;

	if( r != 0 )
	{
		/* FIXME: Is there a better way to handle this? */
		dwipe_perror( errno, "dwipe_static_pass: fdatasync:" );
		dwipe_log( DWIPE_LOG_WARNING, "Buffer flush failure on '%s'.", c->device_name );
	}

	/* Release the output buffer. */
	free( b );
	
	/* We're done. */
	return 0;	

} /* dwipe_static_pass */

/* eof */
