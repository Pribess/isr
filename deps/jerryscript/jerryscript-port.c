/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "jerryscript.h"

#if defined(_WIN32)

#include <windows.h>

/**
 * Default implementation of jerry_port_sleep, uses 'Sleep'.
 */
void
jerry_port_sleep (uint32_t sleep_time) /**< milliseconds to sleep */
{
  Sleep (sleep_time);
} /* jerry_port_sleep */

#endif /* defined(_WIN32) */


#if defined(_WIN32)

#include <stdlib.h>
#include <string.h>

/**
 * Normalize a file path.
 *
 * @return a newly allocated buffer with the normalized path if the operation is successful,
 *         NULL otherwise
 */
jerry_char_t *
jerry_port_path_normalize (const jerry_char_t *path_p, /**< input path */
                           jerry_size_t path_size) /**< size of the path */
{
  (void) path_size;

  return (jerry_char_t *) _fullpath (NULL, path_p, _MAX_PATH);
} /* jerry_port_path_normalize */

/**
 * Free a path buffer returned by jerry_port_path_normalize.
 */
void
jerry_port_path_free (jerry_char_t *path_p)
{
  free (path_p);
} /* jerry_port_path_free */

/**
 * Get the end of the directory part of the input path.
 *
 * @param path_p: input zero-terminated path string
 *
 * @return offset of the directory end in the input path string
 */
jerry_size_t
jerry_port_path_base (const jerry_char_t *path_p)
{
  const jerry_char_t *end_p = path_p + strlen ((const char *) path_p);

  while (end_p > path_p)
  {
    if (end_p[-1] == '/' || end_p[-1] == '\\')
    {
      return (jerry_size_t) (end_p - path_p);
    }

    end_p--;
  }

  return 0;
} /* jerry_port_path_base */

#endif /* defined(_WIN32) */


#if defined(_WIN32)

#include <windows.h>

#include <time.h>
#include <winbase.h>
#include <winnt.h>

#define UNIX_EPOCH_IN_TICKS 116444736000000000ull /* difference between 1970 and 1601 */
#define TICKS_PER_MS        10000ull /* 1 tick is 100 nanoseconds */

/**
 * Convert unix time to a FILETIME value.
 *
 * https://support.microsoft.com/en-us/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
 */
static void
unix_time_to_filetime (double t, LPFILETIME ft_p)
{
  LONGLONG ll = (LONGLONG) t * TICKS_PER_MS + UNIX_EPOCH_IN_TICKS;

  /* FILETIME values before the epoch are invalid. */
  if (ll < 0)
  {
    ll = 0;
  }

  ft_p->dwLowDateTime = (DWORD) ll;
  ft_p->dwHighDateTime = (DWORD) (ll >> 32);
} /* unix_time_to_file_time */

/**
 * Convert a FILETIME to a unix time value.
 *
 * @return unix time
 */
static double
filetime_to_unix_time (LPFILETIME ft_p)
{
  ULARGE_INTEGER date;
  date.HighPart = ft_p->dwHighDateTime;
  date.LowPart = ft_p->dwLowDateTime;
  return (double) (((LONGLONG) date.QuadPart - UNIX_EPOCH_IN_TICKS) / TICKS_PER_MS);
} /* FileTimeToUnixTimeMs */

/**
 * Default implementation of jerry_port_local_tza.
 *
 * @return offset between UTC and local time at the given unix timestamp, if
 *         available. Otherwise, returns 0, assuming UTC time.
 */
int32_t
jerry_port_local_tza (double unix_ms)
{
  FILETIME utc;
  FILETIME local;
  SYSTEMTIME utc_sys;
  SYSTEMTIME local_sys;

  unix_time_to_filetime (unix_ms, &utc);

  if (FileTimeToSystemTime (&utc, &utc_sys) && SystemTimeToTzSpecificLocalTime (NULL, &utc_sys, &local_sys)
      && SystemTimeToFileTime (&local_sys, &local))
  {
    double unix_local = filetime_to_unix_time (&local);
    return (int32_t) (unix_local - unix_ms);
  }

  return 0;
} /* jerry_port_local_tza */

/**
 * Default implementation of jerry_port_current_time.
 *
 * @return milliseconds since Unix epoch
 */
double
jerry_port_current_time (void)
{
  FILETIME ft;
  GetSystemTimeAsFileTime (&ft);
  return filetime_to_unix_time (&ft);
} /* jerry_port_current_time */

#endif /* defined(_WIN32) */


#if defined(__unix__) || defined(__APPLE__)

#if !defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 500
#undef _XOPEN_SOURCE
/* Required macro for sleep functions (nanosleep or usleep) */
#define _XOPEN_SOURCE 500
#endif /* !(defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 500) */

#include <unistd.h>

/**
 * Default implementation of jerry_port_sleep, uses 'usleep'.
 */
void
jerry_port_sleep (uint32_t sleep_time) /**< milliseconds to sleep */
{
  usleep ((useconds_t) sleep_time * 1000);
} /* jerry_port_sleep */

#endif /* defined(__unix__) || defined(__APPLE__) */


#if defined(__unix__) || defined(__APPLE__)

#include <stdlib.h>
#include <string.h>

/**
 * Normalize a file path using realpath.
 *
 * @param path_p: input path
 * @param path_size: input path size
 *
 * @return a newly allocated buffer with the normalized path if the operation is successful,
 *         NULL otherwise
 */
jerry_char_t *
jerry_port_path_normalize (const jerry_char_t *path_p, /**< input path */
                           jerry_size_t path_size) /**< size of the path */
{
  (void) path_size;

  return (jerry_char_t *) realpath ((char *) path_p, NULL);
} /* jerry_port_path_normalize */

/**
 * Free a path buffer returned by jerry_port_path_normalize.
 */
void
jerry_port_path_free (jerry_char_t *path_p)
{
  free (path_p);
} /* jerry_port_path_free */

/**
 * Computes the end of the directory part of a path.
 *
 * @return end of the directory part of a path.
 */
jerry_size_t JERRY_ATTR_WEAK
jerry_port_path_base (const jerry_char_t *path_p) /**< path */
{
  const jerry_char_t *basename_p = (jerry_char_t *) strrchr ((char *) path_p, '/') + 1;

  return (jerry_size_t) (basename_p - path_p);
} /* jerry_port_get_directory_end */

#endif /* defined(__unix__) || defined(__APPLE__) */


#if defined(__unix__) || defined(__APPLE__)

#include <sys/time.h>
#include <time.h>

/**
 * Default implementation of jerry_port_local_tza.
 *
 * @return offset between UTC and local time at the given unix timestamp, if
 *         available. Otherwise, returns 0, assuming UTC time.
 */
int32_t
jerry_port_local_tza (double unix_ms)
{
  time_t time = (time_t) unix_ms / 1000;

#if defined(HAVE_TM_GMTOFF)
  struct tm tm;
  localtime_r (&time, &tm);

  return ((int32_t) tm.tm_gmtoff) * 1000;
#else /* !defined(HAVE_TM_GMTOFF) */
  struct tm gmt_tm;
  struct tm local_tm;

  gmtime_r (&time, &gmt_tm);
  localtime_r (&time, &local_tm);

  time_t gmt = mktime (&gmt_tm);

  /* mktime removes the daylight saving time from the result time value, however we want to keep it */
  local_tm.tm_isdst = 0;
  time_t local = mktime (&local_tm);

  return (int32_t) difftime (local, gmt) * 1000;
#endif /* HAVE_TM_GMTOFF */
} /* jerry_port_local_tza */

/**
 * Default implementation of jerry_port_current_time.
 *
 * @return milliseconds since Unix epoch
 */
double
jerry_port_current_time (void)
{
  struct timeval tv;
  gettimeofday (&tv, NULL);

  return ((double) tv.tv_sec) * 1000.0 + ((double) tv.tv_usec) / 1000.0;
} /* jerry_port_current_time */

#endif /* defined(__unix__) || defined(__APPLE__) */

#include <stdlib.h>


/**
 * Default implementation of jerry_port_fatal. Calls 'abort' if exit code is
 * non-zero, 'exit' otherwise.
 */
void JERRY_ATTR_WEAK
jerry_port_fatal (jerry_fatal_code_t code) /**< cause of error */
{
  if (code != 0 && code != JERRY_FATAL_OUT_OF_MEMORY)
  {
    abort ();
  }

  exit ((int) code);
} /* jerry_port_fatal */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**
 * Default implementation of jerry_port_log. Prints log messages to stderr.
 */
void JERRY_ATTR_WEAK
jerry_port_log (const char *message_p) /**< message */
{
  fputs (message_p, stderr);
} /* jerry_port_log */

/**
 * Default implementation of jerry_port_print_byte. Uses 'putchar' to
 * print a single character to standard output.
 */
void JERRY_ATTR_WEAK
jerry_port_print_byte (jerry_char_t byte) /**< the character to print */
{
  putchar (byte);
} /* jerry_port_print_byte */

/**
 * Default implementation of jerry_port_print_buffer. Uses 'jerry_port_print_byte' to
 * print characters of the input buffer.
 */
void JERRY_ATTR_WEAK
jerry_port_print_buffer (const jerry_char_t *buffer_p, /**< string buffer */
                         jerry_size_t buffer_size) /**< string size*/
{
  for (jerry_size_t i = 0; i < buffer_size; i++)
  {
    jerry_port_print_byte (buffer_p[i]);
  }
} /* jerry_port_print_byte */

/**
 * Read a line from standard input as a zero-terminated string.
 *
 * @param out_size_p: length of the string
 *
 * @return pointer to the buffer storing the string,
 *         or NULL if end of input
 */
jerry_char_t *JERRY_ATTR_WEAK
jerry_port_line_read (jerry_size_t *out_size_p)
{
  char *line_p = NULL;
  size_t allocated = 0;
  size_t bytes = 0;

  while (true)
  {
    allocated += 64;
    line_p = realloc (line_p, allocated);

    while (bytes < allocated - 1)
    {
      char ch = (char) fgetc (stdin);

      if (feof (stdin))
      {
        free (line_p);
        return NULL;
      }

      line_p[bytes++] = ch;

      if (ch == '\n')
      {
        *out_size_p = (jerry_size_t) bytes;
        line_p[bytes++] = '\0';
        return (jerry_char_t *) line_p;
      }
    }
  }
} /* jerry_port_line_read */

/**
 * Free a line buffer allocated by jerry_port_line_read
 *
 * @param buffer_p: buffer that has been allocated by jerry_port_line_read
 */
void JERRY_ATTR_WEAK
jerry_port_line_free (jerry_char_t *buffer_p)
{
  free (buffer_p);
} /* jerry_port_line_free */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if defined(__GLIBC__) || defined(_WIN32)
#include <sys/stat.h>

#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) &S_IFMT) == S_IFDIR)
#endif /* !defined(S_ISDIR) */
#endif /* __GLIBC__ */

/**
 * Determines the size of the given file.
 * @return size of the file
 */
static jerry_size_t
jerry_port_get_file_size (FILE *file_p) /**< opened file */
{
  fseek (file_p, 0, SEEK_END);
  long size = ftell (file_p);
  fseek (file_p, 0, SEEK_SET);

  return (jerry_size_t) size;
} /* jerry_port_get_file_size */

/**
 * Opens file with the given path and reads its source.
 * @return the source of the file
 */
jerry_char_t *JERRY_ATTR_WEAK
jerry_port_source_read (const char *file_name_p, /**< file name */
                        jerry_size_t *out_size_p) /**< [out] read bytes */
{
  /* TODO(dbatyai): Temporary workaround for nuttx target
   * The nuttx target builds and copies the jerryscript libraries as a separate build step, which causes linking issues
   * later due to different libc libraries. It should incorporate the amalgam sources into the main nuttx build so that
   * the correct libraries are used, then this guard should be removed from here and also from the includes. */
#if defined(__GLIBC__) || defined(_WIN32)
  struct stat stat_buffer;
  if (stat (file_name_p, &stat_buffer) == -1 || S_ISDIR (stat_buffer.st_mode))
  {
    return NULL;
  }
#endif /* __GLIBC__ */

  FILE *file_p = fopen (file_name_p, "rb");

  if (file_p == NULL)
  {
    return NULL;
  }

  jerry_size_t file_size = jerry_port_get_file_size (file_p);
  jerry_char_t *buffer_p = (jerry_char_t *) malloc (file_size);

  if (buffer_p == NULL)
  {
    fclose (file_p);
    return NULL;
  }

  size_t bytes_read = fread (buffer_p, 1u, file_size, file_p);

  if (bytes_read != file_size)
  {
    fclose (file_p);
    free (buffer_p);
    return NULL;
  }

  fclose (file_p);
  *out_size_p = (jerry_size_t) bytes_read;

  return buffer_p;
} /* jerry_port_source_read */

/**
 * Release the previously opened file's content.
 */
void JERRY_ATTR_WEAK
jerry_port_source_free (uint8_t *buffer_p) /**< buffer to free */
{
  free (buffer_p);
} /* jerry_port_source_free */

/**
 * These functions provide generic implementation for paths and are only enabled when the compiler support weak symbols,
 * and we are not building for a platform that has platform specific versions.
 */
#if defined(JERRY_WEAK_SYMBOL_SUPPORT) && !(defined(__unix__) || defined(__APPLE__) || defined(_WIN32))

/**
 * Normalize a file path.
 *
 * @return a newly allocated buffer with the normalized path if the operation is successful,
 *         NULL otherwise
 */
jerry_char_t *JERRY_ATTR_WEAK
jerry_port_path_normalize (const jerry_char_t *path_p, /**< input path */
                           jerry_size_t path_size) /**< size of the path */
{
  jerry_char_t *buffer_p = (jerry_char_t *) malloc (path_size + 1);

  if (buffer_p == NULL)
  {
    return NULL;
  }

  /* Also copy terminating zero byte. */
  memcpy (buffer_p, path_p, path_size + 1);

  return buffer_p;
} /* jerry_port_normalize_path */

/**
 * Free a path buffer returned by jerry_port_path_normalize.
 *
 * @param path_p: the path to free
 */
void JERRY_ATTR_WEAK
jerry_port_path_free (jerry_char_t *path_p)
{
  free (path_p);
} /* jerry_port_normalize_path */

/**
 * Computes the end of the directory part of a path.
 *
 * @return end of the directory part of a path.
 */
jerry_size_t JERRY_ATTR_WEAK
jerry_port_path_base (const jerry_char_t *path_p) /**< path */
{
  const jerry_char_t *basename_p = (jerry_char_t *) strrchr ((char *) path_p, '/') + 1;

  if (basename_p == NULL)
  {
    return 0;
  }

  return (jerry_size_t) (basename_p - path_p);
} /* jerry_port_get_directory_end */

#endif /* defined(JERRY_WEAK_SYMBOL_SUPPORT) && !(defined(__unix__) || defined(__APPLE__) || defined(_WIN32)) */

#include <stdlib.h>


#ifndef JERRY_GLOBAL_HEAP_SIZE
#define JERRY_GLOBAL_HEAP_SIZE 512
#endif /* JERRY_GLOBAL_HEAP_SIZE */

/**
 * Pointer to the current context.
 * Note that it is a global variable, and is not a thread safe implementation.
 */
static jerry_context_t *current_context_p = NULL;

/**
 * Allocate a new external context.
 *
 * @param context_size: requested context size
 *
 * @return total allcoated size
 */
size_t JERRY_ATTR_WEAK
jerry_port_context_alloc (size_t context_size)
{
  size_t total_size = context_size + JERRY_GLOBAL_HEAP_SIZE * 1024;
  current_context_p = malloc (total_size);

  return total_size;
} /* jerry_port_context_alloc */

/**
 * Get the current context.
 *
 * @return the pointer to the current context
 */
jerry_context_t *JERRY_ATTR_WEAK
jerry_port_context_get (void)
{
  return current_context_p;
} /* jerry_port_context_get */

/**
 * Free the currently allocated external context.
 */
void JERRY_ATTR_WEAK
jerry_port_context_free (void)
{
  free (current_context_p);
} /* jerry_port_context_free */
