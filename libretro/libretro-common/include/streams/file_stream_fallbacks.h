/* Copyright  (C) 2010-2017 The RetroArch team
*
* ---------------------------------------------------------------------------------------
* The following license statement only applies to this file (file_stream.h).
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

#ifndef __LIBRETRO_SDK_FILE_STREAM_FALLBACKS_H
#define __LIBRETRO_SDK_FILE_STREAM_FALLBACKS_H

#include <stdint.h>

typedef struct libretro_file_fallback libretro_file_fallback;

libretro_file_fallback *fallback_filestream_open(const char *path, unsigned mode);

int fallback_filestream_close(libretro_file_fallback *stream);

int fallback_filestream_error(libretro_file_fallback *stream);

int64_t fallback_filestream_tell(libretro_file_fallback *stream);

int64_t fallback_filestream_seek(libretro_file_fallback *stream, int64_t offset, int whence);

int64_t fallback_filestream_read(libretro_file_fallback *stream, void *s, uint64_t len);

int64_t fallback_filestream_write(libretro_file_fallback *stream, const void *s, uint64_t len);

int fallback_filestream_flush(libretro_file_fallback *stream);

const char *fallback_filestream_get_path(libretro_file_fallback *stream);

#endif