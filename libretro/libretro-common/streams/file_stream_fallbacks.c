/*
The contents of this file would be part of the front end, not the core itself.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "libretro.h"

#if defined(_WIN32)
#  ifdef _MSC_VER
#    define setmode _setmode
#  endif
#  ifdef _XBOX
#    include <xtl.h>
#    define INVALID_FILE_ATTRIBUTES -1
#  else
#    include <io.h>
#    include <fcntl.h>
#    include <direct.h>
#    include <windows.h>
#  endif
#else
#  if defined(PSP)
#    include <pspiofilemgr.h>
#  endif
#  include <sys/types.h>
#  include <sys/stat.h>
#  if !defined(VITA)
#  include <dirent.h>
#  endif
#  include <unistd.h>
#endif

#ifdef __CELLOS_LV2__
#include <cell/cell_fs.h>
#define O_RDONLY CELL_FS_O_RDONLY
#define O_WRONLY CELL_FS_O_WRONLY
#define O_CREAT CELL_FS_O_CREAT
#define O_TRUNC CELL_FS_O_TRUNC
#define O_RDWR CELL_FS_O_RDWR
#else
#include <fcntl.h>
#endif

#include <streams/file_stream.h>
#include <memmap.h>
#include <retro_miscellaneous.h>

typedef struct libretro_file_fallback
{
	unsigned hints;
	char *path;
	long long int size;
#if defined(PSP)
	SceUID fd;
#else

#define HAVE_BUFFERED_IO 1

#define MODE_STR_READ "r"
#define MODE_STR_READ_UNBUF "rb"
#define MODE_STR_WRITE_UNBUF "wb"
#define MODE_STR_WRITE_PLUS "w+"

#if defined(HAVE_BUFFERED_IO)
	FILE *fp;
#endif
#if defined(HAVE_MMAP)
	uint8_t *mapped;
	uint64_t mappos;
	uint64_t mapsize;
#endif
	int fd;
#endif
} libretro_file_fallback;

libretro_file_fallback *fallback_filestream_open(const char *path, unsigned mode)
{
	int            flags = 0;
	int         mode_int = 0;
#if defined(HAVE_BUFFERED_IO)
	const char *mode_str = NULL;
#endif
	libretro_file_fallback        *stream = (libretro_file_fallback*)calloc(1, sizeof(*stream));

	if (!stream)
		return NULL;

	(void)mode_int;
	(void)flags;

	stream->hints = mode;

#ifdef HAVE_MMAP
	if (stream->hints & RFILE_HINT_MMAP && (stream->hints & 0xff) == RFILE_MODE_READ)
		stream->hints |= RFILE_HINT_UNBUFFERED;
	else
#endif
		stream->hints &= ~RFILE_HINT_MMAP;

	switch (mode & 0xff)
	{
	case RFILE_MODE_READ_TEXT:
#if  defined(PSP)
		mode_int = 0666;
		flags = PSP_O_RDONLY;
#else
#if defined(HAVE_BUFFERED_IO)
		if ((stream->hints & RFILE_HINT_UNBUFFERED) == 0)
			mode_str = MODE_STR_READ;
#endif
		/* No "else" here */
		flags = O_RDONLY;
#endif
		break;
	case RFILE_MODE_READ:
#if  defined(PSP)
		mode_int = 0666;
		flags = PSP_O_RDONLY;
#else
#if defined(HAVE_BUFFERED_IO)
		if ((stream->hints & RFILE_HINT_UNBUFFERED) == 0)
			mode_str = MODE_STR_READ_UNBUF;
#endif
		/* No "else" here */
		flags = O_RDONLY;
#endif
		break;
	case RFILE_MODE_WRITE:
#if  defined(PSP)
		mode_int = 0666;
		flags = PSP_O_CREAT | PSP_O_WRONLY | PSP_O_TRUNC;
#else
#if defined(HAVE_BUFFERED_IO)
		if ((stream->hints & RFILE_HINT_UNBUFFERED) == 0)
			mode_str = MODE_STR_WRITE_UNBUF;
#endif
		else
		{
			flags = O_WRONLY | O_CREAT | O_TRUNC;
#ifndef _WIN32
			flags |= S_IRUSR | S_IWUSR;
#endif
		}
#endif
		break;
	case RFILE_MODE_READ_WRITE:
#if  defined(PSP)
		mode_int = 0666;
		flags = PSP_O_RDWR;
#else
#if defined(HAVE_BUFFERED_IO)
		if ((stream->hints & RFILE_HINT_UNBUFFERED) == 0)
			mode_str = MODE_STR_WRITE_PLUS;
#endif
		else
		{
			flags = O_RDWR;
#ifdef _WIN32
			flags |= O_BINARY;
#endif
		}
#endif
		break;
	}

#if  defined(PSP)
	stream->fd = sceIoOpen(path, flags, mode_int);
#else
#if defined(HAVE_BUFFERED_IO)
	if ((stream->hints & RFILE_HINT_UNBUFFERED) == 0 && mode_str)
	{
		stream->fp = fopen(path, mode_str);
		if (!stream->fp)
			goto error;
	}
	else
#endif
	{
		/* FIXME: HAVE_BUFFERED_IO is always 1, but if it is ever changed, open() needs to be changed to _wopen() for WIndows. */
		stream->fd = open(path, flags, mode_int);
		if (stream->fd == -1)
			goto error;
#ifdef HAVE_MMAP
		if (stream->hints & RFILE_HINT_MMAP)
		{
			stream->mappos = 0;
			stream->mapped = NULL;
			stream->mapsize = filestream_seek(stream, 0, SEEK_END);

			if (stream->mapsize == (uint64_t)-1)
				goto error;

			filestream_rewind(stream);

			stream->mapped = (uint8_t*)mmap((void*)0,
				stream->mapsize, PROT_READ, MAP_SHARED, stream->fd, 0);

			if (stream->mapped == MAP_FAILED)
				stream->hints &= ~RFILE_HINT_MMAP;
		}
#endif
	}
#endif

#if  defined(PSP)
	if (stream->fd == -1)
		goto error;
#endif

	{
		stream->path = strdup(path);
	}

	fseek(stream->fp, 0, SEEK_END);
	stream->size = ftell(stream->fp);
	fseek(stream->fp, 0, SEEK_SET);

	return stream;

error:
	filestream_close(stream);
	return NULL;
}

int fallback_filestream_close(libretro_file_fallback *stream)
{
	if (!stream)
		goto error;

#if  defined(PSP)
	if (stream->fd > 0)
		sceIoClose(stream->fd);
#else
#if defined(HAVE_BUFFERED_IO)
	if ((stream->hints & RFILE_HINT_UNBUFFERED) == 0)
	{
		if (stream->fp)
			fclose(stream->fp);
	}
	else
#endif
#ifdef HAVE_MMAP
		if (stream->hints & RFILE_HINT_MMAP)
			munmap(stream->mapped, stream->mapsize);
#endif

	if (stream->fd > 0)
		close(stream->fd);
#endif
	free(stream);

	return 0;

error:
	return -1;
}

int fallback_filestream_error(libretro_file_fallback *stream)
{
#if defined(HAVE_BUFFERED_IO)
	return ferror(stream->fp);
#else
	/* stub */
	return 0;
#endif
}

int64_t fallback_filestream_tell(libretro_file_fallback *stream)
{
	if (!stream)
		goto error;
#if  defined(PSP)
	if (sceIoLseek(stream->fd, 0, SEEK_CUR) < 0)
		goto error;
#else
#if defined(HAVE_BUFFERED_IO)
	if ((stream->hints & RFILE_HINT_UNBUFFERED) == 0)
		return ftell(stream->fp);
#endif
#ifdef HAVE_MMAP
	/* Need to check stream->mapped because this function
	* is called in filestream_open() */
	if (stream->mapped && stream->hints & RFILE_HINT_MMAP)
		return stream->mappos;
#endif
	if (lseek(stream->fd, 0, SEEK_CUR) < 0)
		goto error;
#endif

	return 0;

error:
	return -1;
}

int64_t fallback_filestream_seek(libretro_file_fallback *stream, int64_t offset, int whence)
{
	if (!stream)
		goto error;

#if  defined(PSP)
	if (sceIoLseek(stream->fd, (SceOff)offset, whence) == -1)
		goto error;
#else

#if defined(HAVE_BUFFERED_IO)
	if ((stream->hints & RFILE_HINT_UNBUFFERED) == 0)
		return fseek(stream->fp, (long)offset, whence);
#endif

#ifdef HAVE_MMAP
	/* Need to check stream->mapped because this function is
	* called in filestream_open() */
	if (stream->mapped && stream->hints & RFILE_HINT_MMAP)
	{
		/* fseek() returns error on under/overflow but allows cursor > EOF for
		read-only file descriptors. */
		switch (whence)
		{
		case SEEK_SET:
			if (offset < 0)
				goto error;

			stream->mappos = offset;
			break;

		case SEEK_CUR:
			if ((offset < 0 && stream->mappos + offset > stream->mappos) ||
				(offset > 0 && stream->mappos + offset < stream->mappos))
				goto error;

			stream->mappos += offset;
			break;

		case SEEK_END:
			if (stream->mapsize + offset < stream->mapsize)
				goto error;

			stream->mappos = stream->mapsize + offset;
			break;
		}
		return stream->mappos;
	}
#endif

	if (lseek(stream->fd, offset, whence) < 0)
		goto error;

#endif

	return 0;

error:
	return -1;
}

int64_t fallback_filestream_read(libretro_file_fallback *stream, void *s, uint64_t len)
{
	if (!stream || !s)
		goto error;
#if  defined(PSP)
	return sceIoRead(stream->fd, s, len);
#else
#if defined(HAVE_BUFFERED_IO)
	if ((stream->hints & RFILE_HINT_UNBUFFERED) == 0)
		return fread(s, 1, len, stream->fp);
#endif
#ifdef HAVE_MMAP
	if (stream->hints & RFILE_HINT_MMAP)
	{
		if (stream->mappos > stream->mapsize)
			goto error;

		if (stream->mappos + len > stream->mapsize)
			len = stream->mapsize - stream->mappos;

		memcpy(s, &stream->mapped[stream->mappos], len);
		stream->mappos += len;

		return len;
	}
#endif
	return read(stream->fd, s, len);
#endif

error:
	return -1;
}

int64_t fallback_filestream_write(libretro_file_fallback *stream, const void *s, uint64_t len)
{
	if (!stream)
		goto error;
#if  defined(PSP)
	return sceIoWrite(stream->fd, s, len);
#else
#if defined(HAVE_BUFFERED_IO)
	if ((stream->hints & RFILE_HINT_UNBUFFERED) == 0)
		return fwrite(s, 1, len, stream->fp);
#endif
#ifdef HAVE_MMAP
	if (stream->hints & RFILE_HINT_MMAP)
		goto error;
#endif
	return write(stream->fd, s, len);
#endif

error:
	return -1;
}

int fallback_filestream_flush(libretro_file_fallback *stream)
{
#if defined(HAVE_BUFFERED_IO)
	return fflush(stream->fp);
#else
	return 0;
#endif
}

const char *fallback_filestream_get_path(libretro_file_fallback *stream)
{
	if (!stream)
		return NULL;
	return stream->path;
}
