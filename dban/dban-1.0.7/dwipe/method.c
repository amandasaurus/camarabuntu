/*
 *  method.c: Method implementations for dwipe.
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
 */


/* HOWTO:  Add another wipe method.
 *
 *  1.  Create a new function here and add the prototype to the 'dwipe.h' file.
 *  2.  Update dwipe_method_label() appropriately.
 *  3.  Put the passes that you wish to run into a dwipe_patterns_t array.
 *  4.  Call dwipe_runmethod() with your array of patterns.
 *  5.  Cut-and-paste within the 'options.c' file so that the new method can be invoked.
 *  6.  Optionally try to plug your function into 'gui.c'.
 *
 *
 * WARNING: Remember to pad all pattern arrays with { 0, NULL }.
 *
 * WARNING: Never change dwipe_options after calling a method.
 *
 * NOTE: The dwipe_runmethod function appends a final pass to all methods.
 *
 */


#include "dwipe.h"
#include "context.h"
#include "method.h"
#include "prng.h"
#include "options.h"
#include "pass.h"
#include "logging.h"


/*
 * Comment Legend
 *
 *   "method"  An ordered set of patterns.
 *   "pattern" The magic bits that will be written to a device.
 *   "pass"    Reading or writing one pattern to an entire device.
 *   "rounds"  The number of times that a method will be applied to a device.
 *
 */

const char* dwipe_dod522022m_label = "DoD 5220.22-M";
const char* dwipe_dodshort_label   = "DoD Short";
const char* dwipe_gutmann_label    = "Gutmann Wipe";
const char* dwipe_ops2_label       = "RCMP TSSIT OPS-II";
const char* dwipe_random_label     = "PRNG Stream";
const char* dwipe_zero_label       = "Quick Erase";

const char* dwipe_unknown_label    = "Unknown Method (FIXME)";

const char* dwipe_method_label( dwipe_method_t method )
{
/**
 *  Returns a pointer to the name of the method function.
 *
 */

	if( method == &dwipe_dod522022m ) { return dwipe_dod522022m_label; }
	if( method == &dwipe_dodshort   ) { return dwipe_dodshort_label;   }
	if( method == &dwipe_gutmann    ) { return dwipe_gutmann_label;    }
	if( method == &dwipe_ops2       ) { return dwipe_ops2_label;       }
	if( method == &dwipe_random     ) { return dwipe_random_label;     }
	if( method == &dwipe_zero       ) { return dwipe_zero_label;       }

	/* else */
	return dwipe_unknown_label;

} /* dwipe_method_label */


int dwipe_zero( DWIPE_METHOD_SIGNATURE )
{
/**
 * Fill the device with zeroes.
 *
 */

	/* Do nothing because dwipe_runmethod appends a zero-fill. */
	dwipe_pattern_t patterns [] =
	{
		{ 0, NULL }
	};

	/* Run the method. */
	return dwipe_runmethod( c, patterns );

} /* dwipe_zero */



int dwipe_dod522022m( DWIPE_METHOD_SIGNATURE )
{
/**
 * United States Department of Defense 5220.22-M standard wipe.
 * 
 */

	/* A result holder. */
	int r;

	/* Random characters. (Elements 2 and 6 are unused.) */
	u8 dod [7];

	dwipe_pattern_t patterns [] =
	{
		{  1, &dod[0] }, /* Pass 1: A random character.               */
		{  1, &dod[1] }, /* Pass 2: The bitwise complement of pass 1. */
		{ -1, ""      }, /* Pass 3: A random stream.                  */
		{  1, &dod[3] }, /* Pass 4: A random character.               */
		{  1, &dod[4] }, /* Pass 5: A random character.               */
		{  1, &dod[5] }, /* Pass 6: The bitwise complement of pass 5. */
		{ -1, ""      }, /* Pass 7: A random stream.                  */
		{  0, NULL   }
	};

	/* Load the array with random characters. */
	r = read( c->entropy_fd, &dod, sizeof( dod ) );

	/* NOTE: Only the random data in dod[0], dod[3], and dod[4] is actually used. */

	/* Check the result. */
	if( r != sizeof( dod ) )
	{
		r = errno;
		dwipe_perror( r, __FUNCTION__, "read" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to seed the %s method.", dwipe_dod522022m_label );

		/* Ensure a negative return. */
		if( r < 0 ) { return  r; }
		else        { return -1; }

	}

	/* Pass 2 is the bitwise complement of Pass 1. */
	dod[1] = ~ dod[0];

	/* Pass 4 is the bitwise complement of Pass 3. */
	dod[5] = ~ dod[4];

	/* Run the DoD 5220.22-M method. */
	return dwipe_runmethod( c, patterns );

} /* dwipe_dod522022m */



int dwipe_dodshort( DWIPE_METHOD_SIGNATURE )
{
/**
 * United States Department of Defense 5220.22-M short wipe.
 * This method is comprised of passes 1,2,7 from the standard wipe.
 * 
 */

	/* A result holder. */
	int r;

	/* Random characters. (Element 3 is unused.) */
	u8 dod [3];

	dwipe_pattern_t patterns [] =
	{
		{  1, &dod[0] }, /* Pass 1: A random character.               */
		{  1, &dod[1] }, /* Pass 2: The bitwise complement of pass 1. */
		{ -1, ""      }, /* Pass 3: A random stream.                  */
		{  0, NULL   }
	};

	/* Load the array with random characters. */
	r = read( c->entropy_fd, &dod, sizeof( dod ) );

	/* NOTE: Only the random data in dod[0] is actually used. */

	/* Check the result. */
	if( r != sizeof( dod ) )
	{
		r = errno;
		dwipe_perror( r, __FUNCTION__, "read" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to seed the %s method.", dwipe_dodshort_label );

		/* Ensure a negative return. */
		if( r < 0 ) { return  r; }
		else        { return -1; }

	}

	/* Pass 2 is the bitwise complement of Pass 1. */
	dod[1] = ~ dod[0];

	/* Run the DoD 5220.022-M short method. */
	return dwipe_runmethod( c, patterns );

} /* dwipe_dodshort */



int dwipe_gutmann( DWIPE_METHOD_SIGNATURE )
{
/**
 * Peter Gutmann's wipe.
 *
 */

	/* A result buffer. */
	int r;

	/* The number of patterns in the Guttman Wipe, also used to index the 'patterns' array. */
	int i = 35;

	/* An index into the 'book' array. */
	int j;

	/* The N-th element that has not been used. */
	int n;

	/* Define the Gutmann method. */
	dwipe_pattern_t book [] =
	{
		{ -1, ""             }, /* Random pass.                                      */
		{ -1, ""             }, /* Random pass.                                      */
		{ -1, ""             }, /* Random pass.                                      */
		{ -1, ""             }, /* Random pass.                                      */
		{  3, "\x55\x55\x55" }, /* Static pass: 0x555555  01010101 01010101 01010101 */
		{  3, "\xAA\xAA\xAA" }, /* Static pass: 0XAAAAAA  10101010 10101010 10101010 */
		{  3, "\x92\x49\x24" }, /* Static pass: 0x924924  10010010 01001001 00100100 */
		{  3, "\x49\x24\x92" }, /* Static pass: 0x492492  01001001 00100100 10010010 */
		{  3, "\x24\x92\x49" }, /* Static pass: 0x249249  00100100 10010010 01001001 */
		{  3, "\x00\x00\x00" }, /* Static pass: 0x000000  00000000 00000000 00000000 */
		{  3, "\x11\x11\x11" }, /* Static pass: 0x111111  00010001 00010001 00010001 */
		{  3, "\x22\x22\x22" }, /* Static pass: 0x222222  00100010 00100010 00100010 */
		{  3, "\x33\x33\x33" }, /* Static pass: 0x333333  00110011 00110011 00110011 */
		{  3, "\x44\x44\x44" }, /* Static pass: 0x444444  01000100 01000100 01000100 */
		{  3, "\x55\x55\x55" }, /* Static pass: 0x555555  01010101 01010101 01010101 */
		{  3, "\x66\x66\x66" }, /* Static pass: 0x666666  01100110 01100110 01100110 */
		{  3, "\x77\x77\x77" }, /* Static pass: 0x777777  01110111 01110111 01110111 */
		{  3, "\x88\x88\x88" }, /* Static pass: 0x888888  10001000 10001000 10001000 */
		{  3, "\x99\x99\x99" }, /* Static pass: 0x999999  10011001 10011001 10011001 */
		{  3, "\xAA\xAA\xAA" }, /* Static pass: 0xAAAAAA  10101010 10101010 10101010 */
		{  3, "\xBB\xBB\xBB" }, /* Static pass: 0xBBBBBB  10111011 10111011 10111011 */
		{  3, "\xCC\xCC\xCC" }, /* Static pass: 0xCCCCCC  11001100 11001100 11001100 */
		{  3, "\xDD\xDD\xDD" }, /* Static pass: 0xDDDDDD  11011101 11011101 11011101 */
		{  3, "\xEE\xEE\xEE" }, /* Static pass: 0xEEEEEE  11101110 11101110 11101110 */
		{  3, "\xFF\xFF\xFF" }, /* Static pass: 0xFFFFFF  11111111 11111111 11111111 */
		{  3, "\x92\x49\x24" }, /* Static pass: 0x924924  10010010 01001001 00100100 */
		{  3, "\x49\x24\x92" }, /* Static pass: 0x492492  01001001 00100100 10010010 */
		{  3, "\x24\x92\x49" }, /* Static pass: 0x249249  00100100 10010010 01001001 */
		{  3, "\x6D\xB6\xDB" }, /* Static pass: 0x6DB6DB  01101101 10110110 11011011 */
		{  3, "\xB6\xDB\x6D" }, /* Static pass: 0xB6DB6D  10110110 11011011 01101101 */
		{  3, "\xDB\x6D\xB6" }, /* Static pass: 0XDB6DB6  11011011 01101101 10110110 */
		{ -1, ""             }, /* Random pass.                                      */
		{ -1, ""             }, /* Random pass.                                      */
		{ -1, ""             }, /* Random pass.                                      */
		{ -1, ""             }, /* Random pass.                                      */
		{ 0, NULL }
	};

	/* Put the book array into this array in random order. */
	dwipe_pattern_t patterns [36];

	/* An entropy buffer. */
	u16 s [i];

	/* Load the array with random characters. */
	r = read( c->entropy_fd, &s, sizeof( s ) );

	if( r != sizeof( s ) )
	{
		r = errno;
		dwipe_perror( r, __FUNCTION__, "read" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to seed the %s method.", dwipe_gutmann_label );
		
		/* Ensure a negative return. */
		if( r < 0 ) { return  r; }
		else        { return -1; }
	}


	while( --i >= 0 )
	{

		/* Get a random integer that is less than the first index 'i'. */
		n = (int)( (double)( s[i] ) / (double)( 0x0000FFFF + 1 ) * (double)( i + 1 ) );

		/* Initialize the secondary index. */
		j = -1;

		while( n-- >= 0 )
		{
			/* Advance 'j' by 'n' positions... */
			j += 1;

			/* ... but don't count 'book' elements that have already been copied. */
			while( book[j].length == 0 ) { j += 1; }
		}

		/* Copy the element. */
		patterns[i] = book[j];

		/* Mark this element as having been used. */
		book[j].length = 0;

		dwipe_log( DWIPE_LOG_DEBUG, "dwipe_gutmann: Set patterns[%i] = book[%i].", i, j );
	}

	/* Ensure that the array is terminated. */
	patterns[35].length = 0;
	patterns[35].s = NULL;

	/* Run the Gutmann method. */
	return dwipe_runmethod( c, patterns );

} /* dwipe_gutmann */



int dwipe_ops2( DWIPE_METHOD_SIGNATURE )
{
/**
 *  Royal Canadian Mounted Police
 *  Technical Security Standard for Information Technology
 *  Appendix OPS-II: Media Sanitization
 *
 *  NOTE: The last pass of this method is specially handled by dwipe_runmethod. 
 *
 */

	/* A generic array index. */
	int i;

	/* A generic result buffer. */
	int r;

	/* A buffer for random characters. */
	char* s;

	/* A buffer for the bitwise complements of 's'. */
	char* t;

	/* The element count of 's' and 't'. */
	u32 u;

	/* The pattern array for this method is dynamically allocated. */
	dwipe_pattern_t* patterns;

	/* The element count of 'patterns'. */
	u32 q;


	/* We need one random character per round. */
	u = 1 * dwipe_options.rounds;

	/* Allocate the array of random characters. */
	s = malloc( sizeof( char ) * u );

	if( s == NULL )
	{
		dwipe_perror( errno, __FUNCTION__, "malloc" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to allocate the random character array." );
		return -1;
	}

	/* Allocate the array of complement characters. */
	t = malloc( sizeof( char ) * u );

	if( s == NULL )
	{
		dwipe_perror( errno, __FUNCTION__, "malloc" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to allocate the complement character array." );
		return -1;
	}


	/* We need eight pattern elements per round, plus one for padding. */
	q = 8 * u + 1;

	/* Allocate the pattern array. */
	patterns = malloc( sizeof( dwipe_pattern_t ) * q );

	if( patterns == NULL )
	{
		dwipe_perror( errno, __FUNCTION__, "malloc" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to allocate the pattern array." );
		return -1;
	}


	/* Load the array of random characters. */
	r = read( c->entropy_fd, s, u );

	if( r != u )
	{
		r = errno;
		dwipe_perror( r, __FUNCTION__, "read" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to seed the %s method.", dwipe_ops2_label );
		
		/* Ensure a negative return. */
		if( r < 0 ) { return  r; }
		else        { return -1; }
	}


	for( i = 0 ; i < u ; i += 1 )
	{
		/* Populate the array of complements. */
		t[i] = ~s[i];
	}


	for( i = 0 ; i < u ; i += 8 )
	{
		/* Populate the array of patterns. */

		/* Even elements point to the random characters. */
		patterns[i*4 +0].length = 1;
		patterns[i*4 +0].s = &s[i];
		patterns[i*4 +2].length = 1;
		patterns[i*4 +2].s = &s[i];
		patterns[i*4 +4].length = 1;
		patterns[i*4 +4].s = &s[i];
		patterns[i*4 +6].length = 1;
		patterns[i*4 +6].s = &s[i];

		/* Odd elements point to the complement characters. */
		patterns[i*4 +1].length = 1;
		patterns[i*4 +1].s = &t[i];
		patterns[i*4 +3].length = 1;
		patterns[i*4 +3].s = &t[i];
		patterns[i*4 +5].length = 1;
		patterns[i*4 +5].s = &t[i];
		patterns[i*4 +7].length = 1;
		patterns[i*4 +7].s = &t[i];
	}

	/* Ensure that the array is terminated. */
	patterns[q-1].length = 0;
	patterns[q-1].s = NULL;
	
	/* Run the TSSIT OPS-II method. */
	r = dwipe_runmethod( c, patterns );

	/* Release the random character buffer. */
	free( s );

	/* Release the complement character buffer */
	free( t );

	/* Release the pattern buffer. */
	free( patterns );

	/* We're done. */
	return r;

} /* dwipe_ops2 */



int dwipe_random( DWIPE_METHOD_SIGNATURE )
{
/**
 * Fill the device with a stream from the PRNG.
 *
 */

	/* Define the random method. */
	dwipe_pattern_t patterns [] =
	{
		{ -1, ""   },
		{  0, NULL }
	};

	/* Run the method. */
	return dwipe_runmethod( c, patterns );

} /* dwipe_zero */



int dwipe_runmethod( DWIPE_METHOD_SIGNATURE, dwipe_pattern_t* patterns )
{
/**
 * Writes patterns to the device.
 *
 */

	/* The result holder. */
	int r;

	/* An index variable. */
	int i = 0;

	/* The zero-fill pattern for the final pass of most methods. */
	dwipe_pattern_t pattern_zero = { 1, "\x00" };


	/* Create the PRNG state buffer. */
	c->prng_seed.length = DWIPE_KNOB_PRNG_STATE_LENGTH;
	c->prng_seed.s = malloc( c->prng_seed.length );

	/* Check the memory allocation. */
	if( ! c->prng_seed.s )
	{
		dwipe_perror( errno, __FUNCTION__, "malloc" );
		dwipe_log( DWIPE_LOG_FATAL, "Unable to allocate memory for the prng seed buffer." );
		return -1;
	}

	/* Count the number of patterns in the array. */
	while( patterns[i].length ) { i += 1; }
 

	/* Tell the parent the number of device passes that will be run in one round. */
	c->pass_count = i;

	/* Set the number of bytes that will be written across all passes in one round. */
	c->pass_size = c->pass_count * c->device_size;

	if( dwipe_options.verify == DWIPE_VERIFY_ALL )
	{
		/* We must read back all passes, so double the byte count. */
		c->pass_size *= 2;
	}


	/* Tell the parent the number of rounds that will be run. */
	c->round_count = dwipe_options.rounds;

	/* Set the number of bytes that will be written across all rounds. */
	c->round_size = c->round_count * c->pass_size;

	/* The final pass is always a zero fill, except ops2 which is random. */
	c->round_size += c->device_size;

	if( dwipe_options.verify == DWIPE_VERIFY_LAST || dwipe_options.verify == DWIPE_VERIFY_ALL )
	{
		/* We must read back the last pass to verify it. */
		c->round_size += c->device_size;
	}


	/* Initialize the working round counter. */
	c->round_working = 0;

	dwipe_log( DWIPE_LOG_NOTICE, "Invoking method '%s' on device '%s'.", \
	  dwipe_method_label( dwipe_options.method ), c->device_name );

	while( c->round_working < c->round_count )
	{
		/* Increment the round counter. */
		c->round_working += 1;

		dwipe_log( DWIPE_LOG_NOTICE, "Starting round %i of %i on device '%s'.", \
		  c->round_working, c->round_count, c->device_name );

		/* Initialize the working pass counter. */
		c->pass_working = 0;

		for( i = 0 ; i < c->pass_count ; i++ )
		{
			/* Increment the working pass. */
			c->pass_working += 1;

			dwipe_log( DWIPE_LOG_NOTICE, "Starting pass %i of %i, round %i of %i, on device '%s'.", \
			  c->pass_working, c->pass_count, c->round_working, c->round_count, c->device_name );

			if( patterns[i].length == 0 )
			{
				/* Caught insanity. */
				dwipe_log( DWIPE_LOG_SANITY, "dwipe_runmethod: A non-terminating pattern element has zero length." );
				return -1;
			}
	
			if( patterns[i].length > 0 )
			{

				/* Write a static pass. */
				c->pass_type = DWIPE_PASS_WRITE;
				r = dwipe_static_pass( c, &patterns[i] );
				c->pass_type = DWIPE_PASS_NONE;
	
				/* Check for a fatal error. */
				if( r < 0 ) { return r; }
	
				if( dwipe_options.verify == DWIPE_VERIFY_ALL )
				{

					dwipe_log( DWIPE_LOG_NOTICE, "Verifying pass %i of %i, round %i of %i, on device '%s'.", \
			  		  c->pass_working, c->pass_count, c->round_working, c->round_count, c->device_name );

					/* Verify this pass. */
					c->pass_type = DWIPE_PASS_VERIFY;
					r = dwipe_static_verify( c, &patterns[i] );
					c->pass_type = DWIPE_PASS_NONE;
	
					/* Check for a fatal error. */
					if( r < 0 ) { return r; }

					dwipe_log( DWIPE_LOG_NOTICE, "Verified pass %i of %i, round %i of %i, on device '%s'.", \
			  		  c->pass_working, c->pass_count, c->round_working, c->round_count, c->device_name );
				}
		
			} /* static pass */
	
			else
			{
				c->pass_type = DWIPE_PASS_WRITE;

				/* Seed the PRNG. */
				r = read( c->entropy_fd, c->prng_seed.s, c->prng_seed.length );
	
				/* Check the result. */
				if( r < 0 )
				{
					c->pass_type = DWIPE_PASS_NONE;
					dwipe_perror( errno, __FUNCTION__, "read" );
					dwipe_log( DWIPE_LOG_FATAL, "Unable to seed the PRNG." );
					return -1;
				}
	
				/* Check for a partial read. */
				if( r != c->prng_seed.length )
				{
					/* TODO: Handle partial reads. */
					dwipe_log( DWIPE_LOG_FATAL, "Insufficient entropy is available." );
					return -1;
				}
	
				/* Write the random pass. */
				r = dwipe_random_pass( c );
				c->pass_type = DWIPE_PASS_NONE;
	
				/* Check for a fatal error. */
				if( r < 0 ) { return r; }
	
				if( dwipe_options.verify == DWIPE_VERIFY_ALL )
				{
					dwipe_log( DWIPE_LOG_NOTICE, "Verifying pass %i of %i, round %i of %i, on device '%s'.", \
			  		  c->pass_working, c->pass_count, c->round_working, c->round_count, c->device_name );

					/* Verify this pass. */
					c->pass_type = DWIPE_PASS_VERIFY;
					r = dwipe_random_verify( c );
					c->pass_type = DWIPE_PASS_NONE;
	
					/* Check for a fatal error. */
					if( r < 0 ) { return r; }

					dwipe_log( DWIPE_LOG_NOTICE, "Verified pass %i of %i, round %i of %i, on device '%s'.", \
			  		  c->pass_working, c->pass_count, c->round_working, c->round_count, dwipe_method_label( dwipe_options.method ) );
				}
	
			} /* random pass */
	
			dwipe_log( DWIPE_LOG_NOTICE, "Finished pass %i of %i, round %i of %i, on device '%s'.", \
			  c->pass_working, c->pass_count, c->round_working, c->round_count, c->device_name );

		} /* for passes */

		dwipe_log( DWIPE_LOG_NOTICE, "Finished round %i of %i on device '%s'.", \
		  c->round_working, c->round_count, c->device_name );
	
	} /* while rounds */


	if( dwipe_options.method == &dwipe_ops2 )
	{
		/* NOTE: The OPS-II method specifically requires that a random pattern be left on the device. */

		/* Tell the parent that we are running the final pass. */
		c->pass_type = DWIPE_PASS_FINAL_OPS2;

		/* Seed the PRNG. */
		r = read( c->entropy_fd, c->prng_seed.s, c->prng_seed.length );
	
		/* Check the result. */
		if( r < 0 )
		{
			dwipe_perror( errno, __FUNCTION__, "read" );
			dwipe_log( DWIPE_LOG_FATAL, "Unable to seed the PRNG." );
			return -1;
		}
	
		/* Check for a partial read. */
		if( r != c->prng_seed.length )
		{
			/* TODO: Handle partial reads. */
			dwipe_log( DWIPE_LOG_FATAL, "Insufficient entropy is available." );
			return -1;
		}
	
		dwipe_log( DWIPE_LOG_NOTICE, "Writing final random pattern to '%s'.", c->device_name );

		/* The final ops2 pass. */
		r = dwipe_random_pass( c );

		/* Check for a fatal error. */
		if( r < 0 ) { return r; }

		if( dwipe_options.verify == DWIPE_VERIFY_LAST || dwipe_options.verify == DWIPE_VERIFY_ALL )
		{
			dwipe_log( DWIPE_LOG_NOTICE, "Verifying the final random pattern on '%s' is empty.", c->device_name );

			/* Verify the final zero pass. */
			r = dwipe_random_verify( c );

			/* Check for a fatal error. */
			if( r < 0 ) { return r; }

			dwipe_log( DWIPE_LOG_NOTICE, "Verified the final random pattern on '%s' is empty.", c->device_name );
		}

		dwipe_log( DWIPE_LOG_NOTICE, "Wrote final random pattern to '%s'.", c->device_name );

	} /* final ops2 */


	else
	{
		/* Tell the user that we are on the final pass. */
		c->pass_type = DWIPE_PASS_FINAL_BLANK;

		dwipe_log( DWIPE_LOG_NOTICE, "Blanking device '%s'.", c->device_name );

		/* The final zero pass. */
		r = dwipe_static_pass( c, &pattern_zero );
	
		/* Check for a fatal error. */
		if( r < 0 ) { return r; }
	
	
		if( dwipe_options.verify == DWIPE_VERIFY_LAST || dwipe_options.verify == DWIPE_VERIFY_ALL )
		{
			dwipe_log( DWIPE_LOG_NOTICE, "Verifying that '%s' is empty.", c->device_name );
	
			/* Verify the final zero pass. */
			r = dwipe_static_verify( c, &pattern_zero );
	
			/* Check for a fatal error. */
			if( r < 0 ) { return r; }
	
			dwipe_log( DWIPE_LOG_NOTICE, "Verified that '%s' is empty.", c->device_name );
		}

		dwipe_log( DWIPE_LOG_NOTICE, "Blanked device '%s'.", c->device_name );

	} /* final blank */
	
	/* Release the state buffer. */
	c->prng_seed.length = 0;
	free( c->prng_seed.s );
	
	/* Tell the parent that we have fininshed the final pass. */
	c->pass_type = DWIPE_PASS_NONE;

	if( c->verify_errors > 0 )
	{
		/* We finished, but with non-fatal verification errors. */
		dwipe_log( DWIPE_LOG_ERROR, "%llu verification errors on device '%s'.", c->verify_errors, c->device_name );
	}

	if( c->pass_errors > 0 )
	{
		/* We finished, but with non-fatal wipe errors. */
		dwipe_log( DWIPE_LOG_ERROR, "%llu wipe errors on device '%s'.", c->pass_errors, c->device_name );
	}

	/* FIXME: The 'round_errors' context member is not being used. */

	if( c->pass_errors > 0 || c->round_errors > 0 || c->verify_errors > 0 )
	{
		/* We finished, but with non-fatal errors. */
		return 1;
	}
	
	/* We finished successfully. */
	return 0;	
	
} /* dwipe_runmethod */
	

/* eof */
