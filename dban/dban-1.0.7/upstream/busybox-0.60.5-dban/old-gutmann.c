/*
  Licensed under the GNU Public License.
  Copyright (C) 1998-2002 by Thomas M. Vier, Jr. All Rights Reserved.

  wipe is free software.
  See LICENSE for more information.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include "config.h"

#ifdef STAT_MACROS_BROKEN
/* just in case, so we don't unlink a directory,
   we don't currently handle broken stat macros */
# define unlink(x) remove(x)
#endif

#ifndef HAVE_UNLINK
# define unlink(x) remove(x)
#endif

#include "std.h"
#include "str.h"
#include "io.h"
#include "main.h"
#include "percent.h"
#include "file.h"
#include "lock.h"
#include "dir.h"
#include "rand.h"
#include "wipe.h"

extern int errno;
extern int exit_code;
extern char *argvzero;
extern struct opt_s options;

private void pass_pause(void);
private int random_pass(struct file_s *f, int passes);
private int write3_pass(struct file_s *f, const unsigned char byte1, const unsigned char byte2, const unsigned char byte3);

private int zero(struct file_s *f);
private int wipe(struct file_s *f);

/*
  pass_pause -- pause for pass inspection
*/

private void pass_pause(void)
{
#ifdef PAUSE
  printf("Hit a key:");
  fgetc(stdin);
#endif
}

/*
  random_pass -- fills file with random bytes
                 uses it's own commit_pass() code
*/

private int random_pass(struct file_s *f, int passes)
{
  int i=0;

#ifdef PAUSE
  fprintf(stderr, "\rentering random_pass()\n");
#endif

  while (passes--)
    {
      if (options.seclevel == 2)
	prng_seed();

      if (!options.until_full)
	{
	  while (i++ < f->loop)
	    {
	      prng_fillbuf(options.seclevel, f->buf, f->bufsize);
	      if (do_write(f->name, f->fd, f->buf, f->bufsize)) return FAILED;

	      if (f->percent.display)
		{
		  if (do_fsync(f->name, f->fd)) return FAILED;
		  percent_update(&f->percent);
		}
	    }

	  if (f->loopr)
	    {
	      prng_fillbuf(options.seclevel, f->buf, f->loopr);
	      if (do_write(f->name, f->fd, f->buf, f->loopr)) return FAILED;
	      /* no percent update */
	    }
	}
      else
	{
	  while (1)
	    {
	      prng_fillbuf(options.seclevel, f->buf, f->bufsize);
	      i = do_write(f->name, f->fd, f->buf, f->bufsize);

	      if (i == ENOSPC)
		break;
	    }
	}

      if (do_fwb(f->name, f->fd)) return FAILED;

      pass_pause();
      if (do_lseek(f->name, f->fd, 0, SEEK_SET)) return FAILED; /* rewind */
    }

#ifdef PAUSE
  fprintf(stderr, "\rleaving random_pass()\n");
#endif

  return 0;
}

/*
  write_pass -- fill file with given byte
*/

public int write_pass(struct file_s *f, const unsigned char byte)
{
  int i=0;

  memset(f->buf, byte, f->bufsize);

  if (!options.until_full)
    {
      while (i++ < f->loop)
	{
	  if (do_write(f->name, f->fd, f->buf, f->bufsize)) return FAILED;

	  if (f->percent.display)
	    {
	      if (do_fsync(f->name, f->fd)) return FAILED;
	      percent_update(&f->percent);
	    }
	}

      if (f->loopr)
	{
	  if (do_write(f->name, f->fd, f->buf, f->loopr)) return FAILED;
	  /* no percent update */
	}
    }
  else
    {
      while (1)
	{
	  i = do_write(f->name, f->fd, f->buf, f->bufsize);

	  if (i == ENOSPC)
	    break;
	}
    }

  if (do_fwb(f->name, f->fd)) return FAILED;

  pass_pause();
  if (do_lseek(f->name, f->fd, 0, SEEK_SET)) return FAILED; /* rewind */

  return 0;
}

/*
  write3_pass -- over writes file with given bytes
*/

private int write3_pass(struct file_s *f, const unsigned char byte1, const unsigned char byte2, const unsigned char byte3)
{
  int i=0, r;
  size_t size;
  off_t offset;
  unsigned char bytes[3], *cbuf;

  /* size must be a multiple of 3 */
  size = f->bufsize - (f->bufsize % 3);
  cbuf = (unsigned char *) f->buf;

  while (i < size)
    {
      cbuf[i++] = byte1;
      cbuf[i++] = byte2;
      cbuf[i++] = byte3;
    }

  r = f->bufsize - size; /* find remainder */

  if (r)
    {
      offset = size; i=0;

      bytes[0] = byte1;
      bytes[1] = byte2;
      bytes[2] = byte3;

      while (r--)
	*(unsigned char *)((void *) f->buf + offset++) = bytes[i++];
    }

  /* 
     ok, we've filled the buffer with the pass image

     see fsetbuf() for why we don't have to worry about
     f->bufsize (or f->fsize) being an interger
     multiple of 3
  */

  if (!options.until_full)
    {
      i=0;
      while (i++ < f->loop)
	{
	  if (do_write(f->name, f->fd, f->buf, f->bufsize)) return FAILED;

	  if (f->percent.display)
	    {
	      if (do_fsync(f->name, f->fd)) return FAILED;
	      percent_update(&f->percent);
	    }
	}

      /* smooth out any wrinkles */
      offset = f->bufsize - size;

      if (f->loopr)
	{
	  if (do_write(f->name, f->fd, (f->buf + offset), f->loopr)) return FAILED;
	  /* no percent update */
	}
    }
  else
    {
      while (1)
	{
	  i = do_write(f->name, f->fd, f->buf, f->bufsize);

	  if (i == ENOSPC)
	    break;
	}
    }

  if (do_fwb(f->name, f->fd)) return FAILED;

  pass_pause();
  if (do_lseek(f->name, f->fd, 0, SEEK_SET)) return FAILED; /* rewind */

  return 0;
}

/*
  destroy -- destroy the file
             calls subroutines wipe() and zero()
*/

public int destroy_file(struct file_s *f)
{
#ifdef SANITY
  if (!S_ISREG(f->st.st_mode))
    {
      fprintf(stderr, "\r%s: destroy(): not a regular file: %s\n",
	      argvzero, f->name);
      abort();
    }

  if (strncmp(f->name, f->real_name, PATH_MAX))
    {
      fprintf(stderr, "\r%s: destroy(): f->name != f->real_name\n",
	      argvzero);
      abort();
    }
#endif

#ifdef DEBUG
  fprintf(stderr, "filename: %s\n", f->name);
#endif

  /*
    shred (GNU fileutils) was using fopen() in write-only
    mode which truncates the file before returning. this
    encouraged reallocation of the file's blocks as shred
    wrote to it.

    one major flaw with file wipers like this one and shred,
    is that nothing guarantees that the FS isn't reallocating
    different blocks to the file. log structured FSes, like
    LFS, will almost never use the same blocks. in that case,
    block wiping must be done in the kernel. in-kernel block
    wiping has it's own problem; wiping is IO intense and
    synchronis.
   */

  if ((f->fd = open(f->real_name, O_WRONLY | O_NOFOLLOW | SYNC)) < 0)
    {
      fprintf(stderr, "\r%s: cannot open `%s': %s\n",
	      argvzero, f->name, strerror(errno));
      exit_code = errno; return FAILED;
    }

  /** lock **/
  if (options.lock)
    if (do_lock(f))
      goto failure;

  /** rename **/
  if (options.delete && options.random) 
    if (wipe_name(f) == FAILED)
      goto failure;

  /**** wipe ****/
  if (wipe(f) == FAILED)
    goto failure;

  /*
    if they don't want random passes, or the file won't
    be unlinked, we'll skip the rename
  */

  if (options.delete)
    {
      /* destroy file references, unless this is debug build */
# ifdef HAVE_FTRUNCATE
      do_ftruncate(f->name, f->fd, 0);
# endif

      /** rename **/
      if (options.random) 
	if (wipe_name(f) == FAILED)
	  goto failure;

#ifndef DEBUG
      if (unlink(f->real_name))
	{
	  fprintf(stderr, "\r%s: cannot unlink `%s': %s\n", 
		  argvzero, f->name, strerror(errno));
	  exit_code = errno; goto failure;
	}
#endif
    }

  if (!options.delete)
    {
      /* restore file mode */
      if (fchmod(f->fd, (f->st.st_mode & 07777)))
	{
	  fprintf(stderr, "\r%s: cannot restore file mode for `%s': %s\n",
		  argvzero, f->name, strerror(errno));
	  exit_code = errno; goto failure;
	}
    }

  fsync(f->fd);

  do_close(f->name, f->fd);
  return 0;

 failure:
  do_close(f->name, f->fd);
  return FAILED;
}

/*
  zero -- zeroes out a file. if options.custom, fills with custom byte
*/

private int zero(struct file_s *f)
{
  fsetbuf(f);

  if (fgetbuf(f))
    return FAILED;

  percent_init(&f->percent, f->name, f->bufsize, f->loop);

  if (write_pass(f, options.custom_byte)) return FAILED;

  percent_done(&f->percent);

  ffreebuf(f);

  return 0;
}

/*
  wipe -- runs wipe passes on a given file
*/

private int wipe(struct file_s *f)
{
  if (f->st.st_size == 0)
    return 0; /* no need to write anything */

  if (options.zero || options.custom) return zero(f); /* cute, eh? */

  fsetbuf(f);

  if (fgetbuf(f))
    return FAILED;

  percent_init(&f->percent, f->name, f->bufsize, f->loop);

  /**** run the passes ****/
  if (wipe_passes(f)) {ffreebuf(f); return FAILED;}

  percent_done(&f->percent);

  ffreebuf(f);

  return 0;
}

/*
  wipe_passes -- runs the actual passes
*/

public int wipe_passes(struct file_s *f)
{
  int i, loop;

  if (STATIC_PASSES != 27)
    {
      /*
      STATIC_PASSES is defined in percent.h and used by percent_init()
      it should only need to be changed if this function is changed

      if this function is changed, STATIC_PASSES in percent_init()
      must be updated and the above constant must be changed to match
      */
      fprintf(stderr, "STATIC_PASSES != 27\npercent code broken\n");
      abort();
    }

  if (options.seclevel == 1)
    if (prng_seed())
      return FAILED;

  loop = options.wipe_multiply;

  while (loop--)
    {
      if (options.random)
	if (random_pass(f, options.random_loop))
	  return FAILED;

      /*
	these patterns where taking from Peter Gutmann's 1996 USENIX article,
	Secure Deletion of Data from Magnetic and Solid-State Memory

	http://www.cs.auckland.ac.nz/~pgut001/secure_del.html

	thanks, peter!
      */

      if (options.statics)
	{
	  /*
	    comment format: 
	    pass number -- binary pattern -- target encoding scheme
	  */

	  /* fifth pass -- 01 -- RLL(1,7) and MFM */
	  if (write_pass(f, 0x55)) return FAILED;

	  /* sixth pass -- 10 -- same */
	  if (write_pass(f, 0xaa)) return FAILED;

	  /* seventh pass -- 10010010 01001001 00100100 -- RLL(2,7) and MFM */
	  if (write3_pass(f, 0x92, 0x49, 0x24)) return FAILED;

	  /* eighth pass -- 01001001 00100100 10010010 -- same */
	  if (write3_pass(f, 0x49, 0x24, 0x92)) return FAILED;

	  /* ninth pass -- 00100100 10010010 01001001 -- same */
	  if (write3_pass(f, 0x24, 0x92, 0x49)) return FAILED;

	  /* tenth pass -- start 0x11 increment passes */
	  for (i = 0x00; i <= 0xff; i += 0x11)
	    if (write_pass(f, i))
	      return FAILED;

	  /* 26 -- RLL(2,7) and MFM passes, again */
	  if (write3_pass(f, 0x92, 0x49, 0x24)) return FAILED;
	  if (write3_pass(f, 0x49, 0x24, 0x92)) return FAILED;
	  if (write3_pass(f, 0x24, 0x92, 0x49)) return FAILED;

	  /* 29 -- 01101101 10110110 11011011 -- RLL(2,7) */
	  if (write3_pass(f, 0x6d, 0xb6, 0xdb)) return FAILED;
	  /* 10110110 11011011 01101101 */
	  if (write3_pass(f, 0xb6, 0xdb, 0x6d)) return FAILED;
	  /* 11011011 01101101 10110110 */
	  if (write3_pass(f, 0xdb, 0x6d, 0xb6)) return FAILED;
	}

      if (options.random)
	if (random_pass(f, options.random_loop))
	  return FAILED;
    }
  return 0;
}
