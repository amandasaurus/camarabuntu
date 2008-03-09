/*
 *  device.c:  Device routines for dwipe.
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

#include <netinet/in.h>

#include "dwipe.h"
#include "context.h"
#include "method.h"
#include "options.h"
#include "identify.h"


void dwipe_device_identify( dwipe_context_t* c )
{
	/**
	 * Gets device information and creates the context label.
	 *
	 * @parameter  c         A pointer to a device context.
	 * @modifies   c->label  The menu description that the user sees.
	 *
	 */

	/* FIXME:  We try to infer device information from file names. */
	/*         There is probably an ioctl that does this properly. */

	/* FIXME:  We assume that IDE devices have 'ide' in their file */
	/*         names. This could have unexpected side effects.     */



	/* Scan format for an IDE disk. */
	const char* ide_disk_scan = "/dev/ide/host%i/bus%i/target%i/lun%i/disc";

	/* Label format for an IDE disk. */
	const char* ide_disk_label = "(IDE  %i,%i,%i,-,-) %s";


	/* Scan format for an IDE partition. */
	const char* ide_part_scan = "/dev/ide/host%i/bus%i/target%i/lun%i/part%i";

	/* Label format for an IDE partition. */
	const char* ide_part_label  = "(IDE  %i,%i,%i,-,%i) %s";


	/* Scan format for an SCSI disk. */
	const char* scsi_disk_scan = "/dev/scsi/host%i/bus%i/target%i/lun%i/disc";

	/* Label format for an SCSI disk. */
	const char* scsi_disk_label  = "(SCSI %i,%i,%i,%i,-) %s %s";


	/* Scan format for an SCSI partition. */
	const char* scsi_part_scan = "/dev/scsi/host%i/bus%i/target%i/lun%i/part%i";

	/* Label format for an IDE partition. */
	const char* scsi_part_label = "(SCSI %i,%i,%i,%i,%i) %s";


	/* TODO: Scan format for a Compaq SMART disk. */
	/* const char* s_smart_disk = "/dev/ida/disc%i/disc"; */

	/* TODO: Scan format for a Compaq SMART partition. */
	/* const char* s_smart_part = "/dev/ida/disc%i/part%i"; */



	/* A buffer to pass commands to IDE drives through ioctl. */
	u8 iobuffer[ 4 + sizeof( struct hd_driveid ) ];


	/* Scan buffer for the host number. */
	int bhost;

	/* Scan buffer for the bus number. */
	int bbus;

	/* Scan buffer for the target number. */
	int btarget;

	/* Scan buffer for the logical unit number. */
	int blun;

	/* Scan buffer for the part number. */
	int bpart;


	/* Pointer to change endian order. */
	u16* eggie;


	/* NOTE:  The '/disc' suffix is just a reminder. The scanf family does not
	 *        match literal suffixes.  Run scans with greater cardinality first.
	 */
	

	/* Try to infer details about a SCSI partition. */
	if( sscanf( c->device_name, scsi_part_scan, &bhost, &bbus, &btarget, &blun, &bpart ) == 5 )
	{
		/* Copy device details into the context. */
		c->device_type   = DWIPE_DEVICE_SCSI;
		c->device_host   = bhost;
		c->device_bus    = bbus;
		c->device_target = btarget;
		c->device_lun    = blun; 
		c->device_part   = bpart; 

 		/* Allocate memory for the label. */
 		c->label = malloc( DWIPE_KNOB_LABEL_SIZE );

		snprintf( c->label, DWIPE_KNOB_LABEL_SIZE, scsi_part_label, \
		  c->device_host, c->device_bus, c->device_target, c->device_lun, c->device_part, " Partition" );

		return;

	} /* SCSI partition. */




	/* Try to infer details about an IDE partition. */
	if( sscanf( c->device_name, ide_part_scan, &bhost, &bbus, &btarget, &blun, &bpart ) == 5 )
	{

		/* Copy device details into the context. */
		c->device_type   = DWIPE_DEVICE_IDE;
		c->device_host   = bhost;
		c->device_bus    = bbus;
		c->device_target = btarget;
		c->device_lun    = blun; 
		c->device_part   = bpart; 

 		/* Allocate memory for the label. */
 		c->label = malloc( DWIPE_KNOB_LABEL_SIZE );

		snprintf( c->label, DWIPE_KNOB_LABEL_SIZE, ide_part_label, \
		  c->device_host, c->device_bus, c->device_target, c->device_part, " Partition" );

		return;

	} /* IDE partition. */


	/* Try to infer details about a SCSI disk. */
	if( sscanf( c->device_name, scsi_disk_scan, &bhost, &bbus, &btarget, &blun ) == 4 )
	{
		/* A file pointer for the scsi proc file. */
		FILE* dwipe_fp;

		/* Scan format for a device in the scsi proc file. */
		const char* scsi_proc_scan =  "Host: scsi%i Channel: %i Id: %i Lun: %i ";

		/* A buffer for reading lines from the scsi proc file. */
		char scsi_proc_buffer [81];

		/* Scan buffers for the proc file. */
		int phost;
		int pbus;
		int ptarget;
		int plun;

		/* Copy device details into the context. */
		c->device_type   = DWIPE_DEVICE_SCSI;
		c->device_host   = bhost;
		c->device_bus    = bbus;
		c->device_target = btarget;
		c->device_lun    = blun; 
		c->device_part   = 0; 

		/* Miscellaneous index. */
		int i;


 		/* Allocate memory for the label. */
 		c->label = malloc( DWIPE_KNOB_LABEL_SIZE );

		/* Ensure that the label is null terminated. */
		c->label[0] = 0;


		/* Open the scsi proc file. */
		dwipe_fp = fopen( DWIPE_KNOB_SCSI, "r" );

		/* FIXME: Update this code whenever the /proc/scsi/scsi format changes.
		 *
		 * This is the assumed and expected and proc format.
		 *
		 * 0.........1.........2.........3.........4.........5.........6....
		 *
		 * Host: scsi0 Channel: 00 Id: 00 Lun: 00
		 *   Vendor: 01234567 Model: 012456789ABCDEF Rev: ????
		 *   Type: ????????????????????????????????   ANSI SCSI revision: ??
		 *
		 * The vendor string is at [10..18] and the model string is at [26..40]
		 * on the relevant line.  We are going to match the device numbers and
		 * copy these strings out by their literal offsets.
		 *
		 * Although this seems unwholesome, doing it with SCSI commands through
		 * ioctl() is downright raunchy and just as inflexible.  See the sg_inq
		 * source from the sg3-utils package for an example.
		 *
		 */


		if( dwipe_fp )
		{
			while( ! feof( dwipe_fp ) )
			{
				/* Read the next line. */
				fgets( scsi_proc_buffer, sizeof( scsi_proc_buffer ) -1, dwipe_fp );

				/* FIXME: Ignore impossibly short names, which kludges the write-capable ATAPI device detection problem. */
				if( strlen( scsi_proc_buffer ) < 6 )
				{
					continue;
				}

				/* Try to match a line of device numbers. */
				if( sscanf( scsi_proc_buffer, scsi_proc_scan, &phost, &pbus, &ptarget, &plun ) == 4 )
				{

					if( bhost == phost && bbus == pbus && btarget == ptarget && blun == plun )
					{
						/* Read the vendor/model/rev line. */
						fgets( scsi_proc_buffer, sizeof( scsi_proc_buffer ) -1, dwipe_fp );
						
						if( strlen( scsi_proc_buffer ) > 50 ) 
						{
							/* The padding index of the vendor field. */
							i = 18;

							/* Terminate the vendor substring. */
							while( scsi_proc_buffer[i] == ' ' ) { scsi_proc_buffer[i--] = 0; }

							/* The padding index of the model field. */
							i = 42;

							/* Terminate the model substring. */
							while( scsi_proc_buffer[i] == ' ' ) { scsi_proc_buffer[i--] = 0; }

							/* Print the label. */
	 						snprintf( c->label, DWIPE_KNOB_LABEL_SIZE, scsi_disk_label, \
			  				  c->device_host, c->device_bus, c->device_target, c->device_lun, \
							  &scsi_proc_buffer[10], &scsi_proc_buffer[26] );

							/* Break out. */
							break;
						}

					} /* if */

				} /* if fscanf */

			} /* while not feof */

			fclose( dwipe_fp );

		} /* if dwipe_fp */

		if( c->label[0] == 0 )
		{
	 		snprintf( c->label, DWIPE_KNOB_LABEL_SIZE, scsi_disk_label, \
			  c->device_host, c->device_bus, c->device_target, c->device_lun, "Generic SCSI Disk", "" );
		}

		return;

	} /* SCSI disk. */



	/* Try to infer details about an IDE disk. */
	if( sscanf( c->device_name, ide_disk_scan, &bhost, &bbus, &btarget, &blun ) == 4 )
	{
		/* ASSERT:  ( blun == 0 ) */

		/* IDE command byte. */
		iobuffer[0] = WIN_IDENTIFY;
			  
		/* IDE sector offset parameter. */
		iobuffer[1] = 0;

		/* IDE feature offset paramater. */
		iobuffer[2] = 0;

		/* IDE nsector offset parameter. */
		iobuffer[3] = 1;

		/* Request the IDE device identity block. */
		if( ioctl( c->device_fd, HDIO_DRIVE_CMD, &iobuffer ) )
		{
			/* The ioctl failed. */
			perror( "dwipe_device_identify: ioctl: WIN_IDENTIFY" );
	 		fprintf( stderr, "Error: Unable to identify IDE device %s.\n", c->device_name );
	 		exit( errno );
 		}

		/* Change the endianess of the buffer. */
		for( eggie = (u16*)( &iobuffer ) ; (int)( eggie ) < (int) &iobuffer + (int) sizeof( iobuffer ) ; eggie++ )
		{
			*eggie = ntohs( *eggie );
		}

		/* Copy the identity block into the context. */
		memcpy( &c->device_id, &iobuffer[4], sizeof( c->device_id ) );


		/* Copy inferred device details into the context. */
		c->device_type   = DWIPE_DEVICE_IDE;
		c->device_host   = bhost;
		c->device_bus    = bbus;
		c->device_target = btarget;
		c->device_lun    = blun; 
		c->device_part   = 0;

 		if( c->device_id.model[0] == 0 )
 		{
	 		/* Set a default model name. */
	 		strncpy( c->device_id.model, "Generic IDE Disk", sizeof( c->device_id.model ) );
 		}

 		/* Ensure that the model name will be null-terminated. */
 		c->device_id.model[ sizeof( c->device_id.model ) -1 ] = 0;

 		/* Allocate memory for the label. */
 		c->label = malloc( DWIPE_KNOB_LABEL_SIZE );

		/* Print the label. */
	 	snprintf( c->label, DWIPE_KNOB_LABEL_SIZE, ide_disk_label, \
	  	 c->device_host, c->device_bus, c->device_target, c->device_id.model );

		return;

	} /* IDE disk. */

	/* Else we can probably wipe it, but we don't know what it is. */
	c->device_type = DWIPE_DEVICE_UNKNOWN;

 	/* Allocate memory for the label. */
 	c->label = malloc( DWIPE_KNOB_LABEL_SIZE );

	/* Print the default label. */
	snprintf( c->label, DWIPE_KNOB_LABEL_SIZE, "Unknown Device: %s", c->device_name );

} /* dwipe_device_identify */


int dwipe_device_scan( char*** device_names )
{
	/**
	 * Scans the the filesystem for storage device names.
	 *
	 * @parameter device_names  A reference to a null array pointer.
	 * @modifies  device_names  Populates device_names with an array of strings.
	 * @returns                 The number of strings in the device_names array.
	 *
	 */

	/* The partitions file pointer.  Usually '/proc/partitions'. */
	FILE* fp;

	/* The input buffer. */
	char b [FILENAME_MAX];

	/* Buffer for the major device number. */
	int dmajor;

	/* Buffer for the minor device number. */
	int dminor;

	/* Buffer for the device block count.  */
	loff_t dblocks;

	/* Buffer for the device file name.    */
	char dname [FILENAME_MAX];

	/* Names in the partition file do not have the '/dev/' prefix. */
	char dprefix [] = DWIPE_KNOB_PARTITIONS_PREFIX;

	/* The number of devices that have been found. */
	int dcount = 0;
 
	/* Open the partitions file. */
	fp = fopen( DWIPE_KNOB_PARTITIONS, "r" );

	if( fp == NULL )
	{
		perror( "dwipe_device_scan: fopen" );
		fprintf( stderr, "Error: Unable to open the partitions file '%s'.\n", DWIPE_KNOB_PARTITIONS );
		exit( errno );
	}

	/* Copy the device prefix into the name buffer. */
	strcpy( dname, dprefix );

	/* Sanity check: If device_name is non-null, then it is probably being used. */
	if( *device_names != NULL )
	{
		fprintf( stderr, "Sanity Error: dwipe_device_scan: Non-null device_names pointer.\n" );
		exit( -1 );
	}

	/* Read every line in the partitions file. */
	while( fgets( b, sizeof( b ), fp ) != NULL )
	{
		/* Scan for a device line. */
		if( sscanf( b, "%i %i %lli %s", &dmajor, &dminor, &dblocks, &dname[ strlen( dprefix ) ] ) == 4 )
		{
			/* Increment the device count. */
			dcount += 1;

			/* TODO: Check whether the device numbers are sensible. */

			/* Allocate another name pointer. */
			*device_names = realloc( *device_names, dcount * sizeof(char*) );

			/* Allocate the device name string. */
			(*device_names)[ dcount -1 ] = malloc( strlen( dname ) +1 );
			
			/* Copy the buffer into the device name string. */
			strcpy( (*device_names)[ dcount -1 ], dname );

		} /* if sscanf */

	} /* while fgets */

	/* Pad the array with a null pointer. */
	*device_names = realloc( *device_names, ( dcount +1 ) * sizeof(char*) );
	(*device_names)[ dcount ] = NULL;

	/* Return the number of devices that were found. */
	return dcount;

} /* dwipe_device_scan */

/* eof */
