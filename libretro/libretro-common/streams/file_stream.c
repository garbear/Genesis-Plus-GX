/* Copyright  (C) 2010-2017 The RetroArch team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (file_stream.c).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <streams/file_stream.h>
#include <vfs/vfs_implementation.h>

/* Callbacks */

retro_vfs_file_get_path_t filestream_get_path_cb = NULL;
retro_vfs_file_open_t filestream_open_cb = NULL;
retro_vfs_file_close_t filestream_close_cb = NULL;
retro_vfs_file_error_t filestream_error_cb = NULL;
retro_vfs_file_size_t filestream_size_cb = NULL;
retro_vfs_file_tell_t filestream_tell_cb = NULL;
retro_vfs_file_seek_t filestream_seek_cb = NULL;
retro_vfs_file_truncate_t filestream_truncate_cb = NULL;
retro_vfs_file_read_t filestream_read_cb = NULL;
retro_vfs_file_write_t filestream_write_cb = NULL;
retro_vfs_file_flush_t filestream_flush_cb = NULL;

/* VFS Initialization */

void filestream_vfs_init(retro_environment_t env_cb)
{
	filestream_get_path_cb = NULL;
	filestream_open_cb = NULL;
	filestream_close_cb = NULL;
	filestream_error_cb = NULL;
	filestream_tell_cb = NULL;
	filestream_size_cb = NULL;
	filestream_seek_cb = NULL;
	filestream_truncate_cb = NULL;
	filestream_read_cb = NULL;
	filestream_write_cb = NULL;
	filestream_flush_cb = NULL;

	if (env_cb == NULL)
	{
		return;
	}

	struct retro_vfs_interface_info vfs_info;
	vfs_info.required_interface_version = 1;
	vfs_info.iface = NULL;
	env_cb(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &vfs_info);

	struct retro_vfs_interface* vfs_iface = vfs_info.iface;
	if (vfs_iface == NULL)
	{
		return;
	}

	filestream_get_path_cb = vfs_iface->retro_vfs_file_get_path;
	filestream_open_cb = vfs_iface->retro_vfs_file_open;
	filestream_close_cb = vfs_iface->retro_vfs_file_close;
	filestream_error_cb = vfs_iface->retro_vfs_file_error;
	filestream_size_cb = vfs_iface->retro_vfs_file_size;
	filestream_tell_cb = vfs_iface->retro_vfs_file_tell;
	filestream_seek_cb = vfs_iface->retro_vfs_file_seek;
	filestream_truncate_cb = vfs_iface->retro_vfs_file_truncate;
	filestream_read_cb = vfs_iface->retro_vfs_file_read;
	filestream_write_cb = vfs_iface->retro_vfs_file_write;
	filestream_flush_cb = vfs_iface->retro_vfs_file_flush;
}

/* Callback wrappers */

RFILE *filestream_open(const char *path, retro_file_access access, bool binary_mode, bool create_new, bool replace_existing)
{
	if (filestream_open_cb != NULL)
	{
		return filestream_open_cb(path, access, binary_mode, create_new, replace_existing);
	}

	return (RFILE*)retro_vfs_file_open_impl(path, access, binary_mode, create_new, replace_existing);
}

int filestream_close(RFILE *stream)
{
	if (filestream_close_cb != NULL)
	{
		return filestream_close_cb(stream);
	}

	return retro_vfs_file_close_impl((libretro_vfs_file*)stream);
}

int filestream_error(RFILE *stream)
{
	if (filestream_error_cb != NULL)
	{
		return filestream_error_cb(stream);
	}

	return retro_vfs_file_error_impl((libretro_vfs_file*)stream);
}

int64_t filestream_get_size(RFILE *stream)
{
	if (filestream_size_cb != NULL)
	{
		return filestream_size_cb(stream);
	}

	return retro_vfs_file_size_impl((libretro_vfs_file*)stream);
}


int64_t filestream_tell(RFILE *stream)
{
	if (filestream_tell_cb != NULL)
	{
		return filestream_tell_cb(stream);
	}

	return retro_vfs_file_tell_impl((libretro_vfs_file*)stream);
}

int64_t filestream_seek(RFILE *stream, int64_t offset)
{
	if (filestream_seek_cb != NULL)
	{
		return filestream_seek_cb(stream, offset);
	}

	return retro_vfs_file_seek_impl((libretro_vfs_file*)stream, offset);
}

int64_t filestream_truncate(RFILE *stream, uint64_t size)
{
	if (filestream_truncate_cb != NULL)
	{
		return filestream_truncate_cb(stream, size);
	}

	return retro_vfs_file_truncate_impl((libretro_vfs_file*)stream, size);
}

int64_t filestream_read(RFILE *stream, void *s, uint64_t len)
{
	if (filestream_read_cb != NULL)
	{
		return filestream_read_cb(stream, s, len);
	}

	return retro_vfs_file_read_impl((libretro_vfs_file*)stream, s, len);
}

int64_t filestream_write(RFILE *stream, const void *s, uint64_t len)
{
	if (filestream_write_cb != NULL)
	{
		return filestream_write_cb(stream, s, len);
	}

	return retro_vfs_file_write_impl((libretro_vfs_file*)stream, s, len);
}

int filestream_flush(RFILE *stream)
{
	if (filestream_flush_cb != NULL)
	{
		return filestream_flush_cb(stream);
	}

	return retro_vfs_file_flush_impl((libretro_vfs_file*)stream);
}

const char *filestream_get_path(RFILE *stream)
{
	if (filestream_get_path_cb != NULL)
	{
		return filestream_get_path_cb(stream);
	}

	return retro_vfs_file_get_path_impl((libretro_vfs_file*)stream);
}

/* Wrapper-based Implementations */

const char *filestream_get_ext(RFILE *stream)
{
	const char* path;
	const char* output;

	path = filestream_get_path(stream);
	output = strrchr(path, '.');
	return output;
}

int filestream_eof(RFILE *stream)
{
	int64_t current_position = filestream_tell(stream);
	int64_t end_position = filestream_get_size(stream);

	if (current_position >= end_position)
		return 1;
	return 0;
}

void filestream_rewind(RFILE *stream)
{
	filestream_seek(stream, 0);
}

char *filestream_getline(RFILE *stream)
{
   char* newline     = (char*)malloc(9);
   char* newline_tmp = NULL;
   size_t cur_size   = 8;
   size_t idx        = 0;
   int in            = filestream_getc(stream);

   if (!newline)
      return NULL;

   while (in != EOF && in != '\n')
   {
      if (idx == cur_size)
      {
         cur_size *= 2;
         newline_tmp = (char*)realloc(newline, cur_size + 1);

         if (!newline_tmp)
         {
            free(newline);
            return NULL;
         }

         newline = newline_tmp;
      }

      newline[idx++] = in;
      in             = filestream_getc(stream);
   }

   newline[idx] = '\0';
   return newline; 
}

char *filestream_gets(RFILE *stream, char *s, uint64_t len)
{
   if (!stream)
      return NULL;

   if(filestream_read(stream,s,len)==len)
      return s;
   return NULL;
}

int filestream_getc(RFILE *stream)
{
   char c = 0;
   (void)c;

   if (!stream)
      return 0;

    if(filestream_read(stream, &c, 1) == 1)
       return (int)c;
    return EOF;
}

int filestream_putc(RFILE *stream, int c)
{
   if (!stream)
      return EOF;

#if defined(HAVE_BUFFERED_IO)
   return fputc(c, stream->fp);
#else
   /* unimplemented */
   return EOF;
#endif
}

uint64_t filestream_vprintf(RFILE *stream, const char* format, va_list args)
{
	static char buffer[8 * 1024];
	uint64_t numChars = vsprintf(buffer, format, args);

	if (numChars < 0)
		return -1;
	else if (numChars == 0)
		return 0;

	return filestream_write(stream, buffer, numChars);
}

uint64_t filestream_printf(RFILE *stream, const char* format, ...)
{
	uint64_t result;
	va_list vl;
	va_start(vl, format);
	result = filestream_vprintf(stream, format, vl);
	va_end(vl);
	return result;
}

/**
 * filestream_read_file:
 * @path             : path to file.
 * @buf              : buffer to allocate and read the contents of the
 *                     file into. Needs to be freed manually.
 *
 * Read the contents of a file into @buf.
 *
 * Returns: number of items read, -1 on error.
 */
int filestream_read_file(const char *path, void **buf, uint64_t *len)
{
   int64_t ret              = 0;
   int64_t content_buf_size = 0;
   void *content_buf        = NULL;
   RFILE *file              = filestream_open(path, RFILE_ACCESS_READ_ONLY, true, false, false);

   if (!file)
   {
      fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
      goto error;
   }

   int64_t size = filestream_get_size(file);
   if (filestream_seek(file, size) != 0)
      goto error;

   content_buf_size = filestream_tell(file);
   if (content_buf_size < 0)
      goto error;

   filestream_rewind(file);

   content_buf = malloc((size_t)content_buf_size + 1);

   if (!content_buf)
      goto error;

   ret = filestream_read(file, content_buf, content_buf_size);
   if (ret < 0)
   {
      fprintf(stderr, "Failed to read %s: %s\n", path, strerror(errno));
      goto error;
   }

   filestream_close(file);

   *buf    = content_buf;

   /* Allow for easy reading of strings to be safe.
    * Will only work with sane character formatting (Unix). */
   ((char*)content_buf)[ret] = '\0';

   if (len)
      *len = ret;

   return 1;

error:
   if (file)
      filestream_close(file);
   if (content_buf)
      free(content_buf);
   if (len)
      *len = -1;
   *buf = NULL;
   return 0;
}

/**
 * filestream_write_file:
 * @path             : path to file.
 * @data             : contents to write to the file.
 * @size             : size of the contents.
 *
 * Writes data to a file.
 *
 * Returns: true (1) on success, false (0) otherwise.
 */
bool filestream_write_file(const char *path, const void *data, uint64_t size)
{
   int64_t ret   = 0;
   RFILE *file   = filestream_open(path, RFILE_ACCESS_READ_WRITE, true, true, true);
   if (!file)
      return false;

   ret = filestream_write(file, data, size);
   filestream_close(file);

   if (ret != size)
      return false;

   return true;
}
