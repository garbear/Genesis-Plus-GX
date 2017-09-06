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

#include "libretro.h"

#include <streams/file_stream.h>

// Callbacks

retro_file_get_name_t filestream_get_name_cb = NULL;
retro_file_get_ext_t filestream_get_ext_cb = NULL;
retro_file_get_size_t filestream_get_size_cb = NULL;
retro_file_open_t filestream_open_cb = NULL;
retro_file_close_t filestream_close_cb = NULL;
retro_file_error_t filestream_error_cb = NULL;
retro_file_tell_t filestream_tell_cb = NULL;
retro_file_seek_t filestream_seek_cb = NULL;
retro_file_read_t filestream_read_cb = NULL;
retro_file_write_t filestream_write_cb = NULL;
retro_file_flush_t filestream_flush_cb = NULL;

void retro_set_file_get_name(retro_file_get_name_t cb)
{
	filestream_get_name_cb = cb;
}

void retro_set_file_get_ext(retro_file_get_ext_t cb)
{
	filestream_get_ext_cb = cb;
}

void retro_set_file_get_size(retro_file_get_size_t cb)
{
	filestream_get_size_cb = cb;
}

void retro_set_file_open(retro_file_open_t cb)
{
	filestream_open_cb = cb;
}

void retro_set_file_close(retro_file_close_t cb)
{
	filestream_close_cb = cb;
}

void retro_set_file_error(retro_file_error_t cb)
{
	filestream_error_cb = cb;
}

void retro_set_file_tell(retro_file_tell_t cb)
{
	filestream_tell_cb = cb;
}

void retro_set_file_seek(retro_file_seek_t cb)
{
	filestream_seek_cb = cb;
}

void retro_set_file_read(retro_file_read_t cb)
{
	filestream_read_cb = cb;
}

void retro_set_file_write(retro_file_write_t cb)
{
	filestream_write_cb = cb;
}

void retro_set_file_flush(retro_file_flush_t cb)
{
	filestream_flush_cb = cb;
}

// Callback wrappers

const char *filestream_get_name(RFILE *stream)
{
	return filestream_get_name_cb(stream);
}

const char *filestream_get_ext(RFILE *stream)
{
	return filestream_get_ext_cb(stream);
}

long long int filestream_get_size(RFILE *stream)
{
	return filestream_get_size_cb(stream);
}

RFILE *filestream_open(const char *path, unsigned mode)
{
	return filestream_open_cb(path, mode);
}

int filestream_close(RFILE *stream)
{
	return filestream_close_cb(stream);
}

int filestream_error(RFILE *stream)
{
	return filestream_error_cb(stream);
}

int64_t filestream_tell(RFILE *stream)
{
	return filestream_tell_cb(stream);
}

int64_t filestream_seek(RFILE *stream, int64_t offset, int whence)
{
	return filestream_seek_cb(stream, offset, whence);
}

int64_t filestream_read(RFILE *stream, void *s, uint64_t len)
{
	return filestream_read_cb(stream, s, len);
}

int64_t filestream_write(RFILE *stream, const void *s, uint64_t len)
{
	return filestream_write_cb(stream, s, len);
}

int filestream_flush(RFILE *stream)
{
	return filestream_flush_cb(stream);
}

// Wrapper-based Implementations

int filestream_eof(RFILE *stream)
{
	size_t current_position = filestream_tell(stream);
	size_t end_position = filestream_seek(stream, 0, SEEK_END);

	filestream_seek(stream, current_position, SEEK_SET);

	if (current_position >= end_position)
		return 1;
	return 0;
}

void filestream_rewind(RFILE *stream)
{
	filestream_seek(stream, 0L, SEEK_SET);
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

int filestream_vprintf(RFILE *stream, const char* format, va_list args)
{
	static char buffer[8 * 1024];
	int numChars = vsprintf(buffer, format, args);

	if (numChars < 0)
		return -1;
	else if (numChars == 0)
		return 0;

	return filestream_write(stream, buffer, numChars);
}

int filestream_printf(RFILE *stream, const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	int result = filestream_vprintf(stream, format, vl);
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
   RFILE *file              = filestream_open(path, RFILE_MODE_READ);

   if (!file)
   {
      fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
      goto error;
   }

   if (filestream_seek(file, 0, SEEK_END) != 0)
      goto error;

   content_buf_size = filestream_tell(file);
   if (content_buf_size < 0)
      goto error;

   filestream_rewind(file);

   content_buf = malloc(content_buf_size + 1);

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
   RFILE *file   = filestream_open(path, RFILE_MODE_WRITE);
   if (!file)
      return false;

   ret = filestream_write(file, data, size);
   filestream_close(file);

   if (ret != size)
      return false;

   return true;
}
