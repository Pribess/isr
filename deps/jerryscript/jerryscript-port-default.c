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
#include "jerryscript-port-default.h"

#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

/**
 * Determines the size of the given file.
 * @return size of the file
 */
static size_t
jerry_port_get_file_size (FILE *file_p) /**< opened file */
{
  fseek (file_p, 0, SEEK_END);
  long size = ftell (file_p);
  fseek (file_p, 0, SEEK_SET);

  return (size_t) size;
} /* jerry_port_get_file_size */

/**
 * Opens file with the given path and reads its source.
 * @return the source of the file
 */
uint8_t *
jerry_port_read_source (const char *file_name_p, /**< file name */
                        size_t *out_size_p) /**< [out] read bytes */
{
  struct stat stat_buffer;
  if (stat (file_name_p, &stat_buffer) == -1 || S_ISDIR (stat_buffer.st_mode))
  {
    jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Error: Failed to open file: %s\n", file_name_p);
    return NULL;
  }

  FILE *file_p = fopen (file_name_p, "rb");

  if (file_p == NULL)
  {
    jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Error: Failed to open file: %s\n", file_name_p);
    return NULL;
  }

  size_t file_size = jerry_port_get_file_size (file_p);
  uint8_t *buffer_p = (uint8_t *) malloc (file_size);

  if (buffer_p == NULL)
  {
    fclose (file_p);

    jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Error: Failed to allocate memory for file: %s\n", file_name_p);
    return NULL;
  }

  size_t bytes_read = fread (buffer_p, 1u, file_size, file_p);

  if (bytes_read != file_size)
  {
    fclose (file_p);
    free (buffer_p);

    jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Error: Failed to read file: %s\n", file_name_p);
    return NULL;
  }

  fclose (file_p);
  *out_size_p = bytes_read;

  return buffer_p;
} /* jerry_port_read_source */

/**
 * Release the previously opened file's content.
 */
void
jerry_port_release_source (uint8_t *buffer_p) /**< buffer to free */
{
  free (buffer_p);
} /* jerry_port_release_source */

/**
 * Computes the end of the directory part of a path.
 *
 * @return end of the directory part of a path.
 */
static size_t
jerry_port_get_directory_end (const jerry_char_t *path_p) /**< path */
{
  const jerry_char_t *end_p = path_p + strlen ((const char *) path_p);

  while (end_p > path_p)
  {
#if defined (_WIN32)
    if (end_p[-1] == '/' || end_p[-1] == '\\')
    {
      return (size_t) (end_p - path_p);
    }
#else /* !_WIN32 */
    if (end_p[-1] == '/')
    {
      return (size_t) (end_p - path_p);
    }
#endif /* _WIN32 */

    end_p--;
  }

  return 0;
} /* jerry_port_get_directory_end */

/**
 * Normalize a file path.
 *
 * @return a newly allocated buffer with the normalized path if the operation is successful,
 *         NULL otherwise
 */
static jerry_char_t *
jerry_port_normalize_path (const jerry_char_t *in_path_p, /**< path to the referenced module */
                           size_t in_path_length, /**< length of the path */
                           const jerry_char_t *base_path_p, /**< base path */
                           size_t base_path_length) /**< length of the base path */
{
  char *path_p;

  if (base_path_length > 0)
  {
    path_p = (char *) malloc (base_path_length + in_path_length + 1);

    if (path_p == NULL)
    {
      return NULL;
    }

    memcpy (path_p, base_path_p, base_path_length);
    memcpy (path_p + base_path_length, in_path_p, in_path_length);
    path_p[base_path_length + in_path_length] = '\0';
  }
  else
  {
    path_p = (char *) malloc (in_path_length + 1);

    if (path_p == NULL)
    {
      return NULL;
    }

    memcpy (path_p, in_path_p, in_path_length);
    path_p[in_path_length] = '\0';
  }

#if defined (_WIN32)
  char full_path[_MAX_PATH];

  if (_fullpath (full_path, path_p, _MAX_PATH) != NULL)
  {
    free (path_p);

    size_t full_path_len = strlen (full_path);

    path_p = (char *) malloc (full_path_len + 1);

    if (path_p == NULL)
    {
      return NULL;
    }

    memcpy (path_p, full_path, full_path_len + 1);
  }
#elif defined (__unix__) || defined (__APPLE__)
  char *norm_p = realpath (path_p, NULL);

  if (norm_p != NULL)
  {
    free (path_p);
    path_p = norm_p;
  }
#endif /* _WIN32 */

  return (jerry_char_t *) path_p;
} /* jerry_port_normalize_path */

/**
 * A module descriptor.
 */
typedef struct jerry_port_module_t
{
  struct jerry_port_module_t *next_p; /**< next_module */
  jerry_char_t *path_p; /**< path to the module */
  size_t base_path_length; /**< base path length for relative difference */
  jerry_value_t realm; /**< the realm of the module */
  jerry_value_t module; /**< the module itself */
} jerry_port_module_t;

/**
 * Native info descriptor for modules.
 */
static const jerry_object_native_info_t jerry_port_module_native_info =
{
  .free_cb = NULL,
};

/**
 * Default module manager.
 */
typedef struct
{
  jerry_port_module_t *module_head_p; /**< first module */
} jerry_port_module_manager_t;

/**
 * Release known modules.
 */
static void
jerry_port_module_free (jerry_port_module_manager_t *manager_p, /**< module manager */
                        const jerry_value_t realm) /**< if this argument is object, release only those modules,
                                                    *   which realm value is equal to this argument. */
{
  jerry_port_module_t *module_p = manager_p->module_head_p;

  bool release_all = !jerry_value_is_object (realm);

  jerry_port_module_t *prev_p = NULL;

  while (module_p != NULL)
  {
    jerry_port_module_t *next_p = module_p->next_p;

    if (release_all || module_p->realm == realm)
    {
      free (module_p->path_p);
      jerry_release_value (module_p->realm);
      jerry_release_value (module_p->module);

      free (module_p);

      if (prev_p == NULL)
      {
        manager_p->module_head_p = next_p;
      }
      else
      {
        prev_p->next_p = next_p;
      }
    }
    else
    {
      prev_p = module_p;
    }

    module_p = next_p;
  }
} /* jerry_port_module_free */

/**
 * Initialize the default module manager.
 */
static void
jerry_port_module_manager_init (void *user_data_p)
{
  ((jerry_port_module_manager_t *) user_data_p)->module_head_p = NULL;
} /* jerry_port_module_manager_init */

/**
 * Deinitialize the default module manager.
 */
static void
jerry_port_module_manager_deinit (void *user_data_p) /**< context pointer to deinitialize */
{
  jerry_value_t undef = jerry_create_undefined ();
  jerry_port_module_free ((jerry_port_module_manager_t *) user_data_p, undef);
  jerry_release_value (undef);
} /* jerry_port_module_manager_deinit */

/**
 * Declare the context data manager for modules.
 */
static const jerry_context_data_manager_t jerry_port_module_manager =
{
  .init_cb = jerry_port_module_manager_init,
  .deinit_cb = jerry_port_module_manager_deinit,
  .bytes_needed = sizeof (jerry_port_module_manager_t)
};

/**
 * Default module resolver.
 *
 * @return a module object if resolving is successful, an error otherwise
 */
jerry_value_t
jerry_port_module_resolve (const jerry_value_t specifier, /**< module specifier string */
                           const jerry_value_t referrer, /**< parent module */
                           void *user_p) /**< user data */
{
  (void) user_p;

  jerry_port_module_t *module_p;
  const jerry_char_t *base_path_p = NULL;
  size_t base_path_length = 0;

  if (jerry_get_object_native_pointer (referrer, (void **) &module_p, &jerry_port_module_native_info))
  {
    base_path_p = module_p->path_p;
    base_path_length = module_p->base_path_length;
  }

  jerry_size_t in_path_length = jerry_get_utf8_string_size (specifier);
  jerry_char_t *in_path_p = (jerry_char_t *) malloc (in_path_length + 1);
  jerry_string_to_utf8_char_buffer (specifier, in_path_p, in_path_length);
  in_path_p[in_path_length] = '\0';

  jerry_char_t *path_p = jerry_port_normalize_path (in_path_p, in_path_length, base_path_p, base_path_length);

  if (path_p == NULL)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Out of memory");
  }

  jerry_value_t realm = jerry_get_global_object ();

  jerry_port_module_manager_t *manager_p;
  manager_p = (jerry_port_module_manager_t *) jerry_get_context_data (&jerry_port_module_manager);

  module_p = manager_p->module_head_p;

  while (module_p != NULL)
  {
    if (module_p->realm == realm
        && strcmp ((const char *) module_p->path_p, (const char *) path_p) == 0)
    {
      free (path_p);
      free (in_path_p);
      jerry_release_value (realm);
      return jerry_acquire_value (module_p->module);
    }

    module_p = module_p->next_p;
  }

  size_t source_size;
  uint8_t *source_p = jerry_port_read_source ((const char *) path_p, &source_size);

  if (source_p == NULL)
  {
    free (path_p);
    free (in_path_p);
    jerry_release_value (realm);
    /* TODO: This is incorrect, but makes test262 module tests pass
     * (they should throw SyntaxError, but not because the module cannot be found). */
    return jerry_create_error (JERRY_ERROR_SYNTAX, (const jerry_char_t *) "Module file not found");
  }

  jerry_parse_options_t parse_options;
  parse_options.options = JERRY_PARSE_MODULE | JERRY_PARSE_HAS_RESOURCE;
  parse_options.resource_name = jerry_create_string_sz ((const jerry_char_t *) in_path_p, in_path_length);

  jerry_value_t ret_value = jerry_parse (source_p,
                                         source_size,
                                         &parse_options);
  jerry_release_value (parse_options.resource_name);

  jerry_port_release_source (source_p);
  free (in_path_p);

  if (jerry_value_is_error (ret_value))
  {
    free (path_p);
    jerry_release_value (realm);
    return ret_value;
  }

  module_p = (jerry_port_module_t *) malloc (sizeof (jerry_port_module_t));

  module_p->next_p = manager_p->module_head_p;
  module_p->path_p = path_p;
  module_p->base_path_length = jerry_port_get_directory_end (module_p->path_p);
  module_p->realm = realm;
  module_p->module = jerry_acquire_value (ret_value);

  jerry_set_object_native_pointer (ret_value, module_p, &jerry_port_module_native_info);
  manager_p->module_head_p = module_p;

  return ret_value;
} /* jerry_port_module_resolve */

/**
 * Release known modules.
 */
void
jerry_port_module_release (const jerry_value_t realm) /**< if this argument is object, release only those modules,
                                                       *   which realm value is equal to this argument. */
{
  jerry_port_module_free ((jerry_port_module_manager_t *) jerry_get_context_data (&jerry_port_module_manager),
                          realm);
} /* jerry_port_module_release */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


/**
 * Actual log level
 */
static jerry_log_level_t jerry_port_default_log_level = JERRY_LOG_LEVEL_ERROR;

/**
 * Get the log level
 *
 * @return current log level
 */
jerry_log_level_t
jerry_port_default_get_log_level (void)
{
  return jerry_port_default_log_level;
} /* jerry_port_default_get_log_level */

/**
 * Set the log level
 */
void
jerry_port_default_set_log_level (jerry_log_level_t level) /**< log level */
{
  jerry_port_default_log_level = level;
} /* jerry_port_default_set_log_level */

/**
 * Default implementation of jerry_port_log. Prints log message to the standard
 * error with 'vfprintf' if message log level is less than or equal to the
 * current log level.
 *
 * If debugger support is enabled, printing happens first to an in-memory buffer,
 * which is then sent both to the standard error and to the debugger client.
 */
void
jerry_port_log (jerry_log_level_t level, /**< message log level */
                const char *format, /**< format string */
                ...)  /**< parameters */
{
  if (level <= jerry_port_default_log_level)
  {
    va_list args;
    va_start (args, format);
#if defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)
    int length = vsnprintf (NULL, 0, format, args);
    va_end (args);
    va_start (args, format);

    JERRY_VLA (char, buffer, length + 1);
    vsnprintf (buffer, (size_t) length + 1, format, args);

    fprintf (stderr, "%s", buffer);
    jerry_debugger_send_log (level, (jerry_char_t *) buffer, (jerry_size_t) length);
#else /* If jerry-debugger isn't defined, libc is turned on */
    vfprintf (stderr, format, args);
#endif /* defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1) */
    va_end (args);
  }
} /* jerry_port_log */

#if defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)

#define DEBUG_BUFFER_SIZE (256)
static char debug_buffer[DEBUG_BUFFER_SIZE];
static int debug_buffer_index = 0;

#endif /* defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1) */

/**
 * Default implementation of jerry_port_print_char. Uses 'putchar' to
 * print a single character to standard output.
 */
void
jerry_port_print_char (char c) /**< the character to print */
{
  putchar (c);

#if defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)
  debug_buffer[debug_buffer_index++] = c;

  if ((debug_buffer_index == DEBUG_BUFFER_SIZE) || (c == '\n'))
  {
    jerry_debugger_send_output ((jerry_char_t *) debug_buffer, (jerry_size_t) debug_buffer_index);
    debug_buffer_index = 0;
  }
#endif /* defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1) */
} /* jerry_port_print_char */

#include <stdlib.h>


/**
 * Default implementation of jerry_port_fatal. Calls 'abort' if exit code is
 * non-zero, 'exit' otherwise.
 */
void jerry_port_fatal (jerry_fatal_code_t code) /**< cause of error */
{
  if (code != 0
      && code != ERR_OUT_OF_MEMORY)
  {
    abort ();
  }

  exit ((int) code);
} /* jerry_port_fatal */


/**
 * Pointer to the current context.
 * Note that it is a global variable, and is not a thread safe implementation.
 */
static jerry_context_t *current_context_p = NULL;

/**
 * Set the current_context_p as the passed pointer.
 */
void
jerry_port_default_set_current_context (jerry_context_t *context_p) /**< points to the created context */
{
  current_context_p = context_p;
} /* jerry_port_default_set_current_context */

/**
 * Get the current context.
 *
 * @return the pointer to the current context
 */
jerry_context_t *
jerry_port_get_current_context (void)
{
  return current_context_p;
} /* jerry_port_get_current_context */

#if !defined (_XOPEN_SOURCE) || _XOPEN_SOURCE < 500
#undef _XOPEN_SOURCE
/* Required macro for sleep functions (nanosleep or usleep) */
#define _XOPEN_SOURCE 500
#endif

#ifdef _WIN32
#include <windows.h>
#else /* !_WIN32 */
#include <unistd.h>
#endif /* _WIN32 */


/**
 * Default implementation of jerry_port_sleep. Uses 'usleep' if available on the
 * system, does nothing otherwise.
 */
void jerry_port_sleep (uint32_t sleep_time) /**< milliseconds to sleep */
{
#ifdef _WIN32
  Sleep (sleep_time);
#else /* !_WIN32 */
  usleep ((useconds_t) sleep_time * 1000);
#endif /* _WIN32 */
} /* jerry_port_sleep */

#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <winbase.h>
#include <winnt.h>
#endif /* _WIN32 */

#if defined (__GNUC__) || defined (__clang__)
#include <sys/time.h>
#endif /* __GNUC__ || __clang__ */


#ifdef _WIN32
static const LONGLONG UnixEpochInTicks = 116444736000000000; /* difference between 1970 and 1601 */
static const LONGLONG TicksPerMs = 10000; /* 1 tick is 100 nanoseconds */

/* https://support.microsoft.com/en-us/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime */
static void UnixTimeMsToFileTime (double t, LPFILETIME pft)
{
  LONGLONG ll = (LONGLONG) t * TicksPerMs + UnixEpochInTicks;
  pft->dwLowDateTime = (DWORD) ll;
  pft->dwHighDateTime = (DWORD) (ll >> 32);
} /* UnixTimeMsToFileTime */

static double FileTimeToUnixTimeMs (FILETIME ft)
{
  ULARGE_INTEGER date;
  date.HighPart = ft.dwHighDateTime;
  date.LowPart = ft.dwLowDateTime;
  return (double) (((LONGLONG) date.QuadPart - UnixEpochInTicks) / TicksPerMs);
} /* FileTimeToUnixTimeMs */

#endif /* _WIN32 */

/**
 * Default implementation of jerry_port_get_local_time_zone_adjustment.
 *
 * @return offset between UTC and local time at the given unix timestamp, if
 *         available. Otherwise, returns 0, assuming UTC time.
 */
double jerry_port_get_local_time_zone_adjustment (double unix_ms,  /**< ms since unix epoch */
                                                  bool is_utc)  /**< is the time above in UTC? */
{
#if defined (HAVE_TM_GMTOFF)
  struct tm tm;
  time_t now = (time_t) (unix_ms / 1000);
  localtime_r (&now, &tm);

  if (!is_utc)
  {
    now -= tm.tm_gmtoff;
    localtime_r (&now, &tm);
  }

  return ((double) tm.tm_gmtoff) * 1000;
#elif defined (_WIN32)
  FILETIME fileTime, localFileTime;
  SYSTEMTIME systemTime, localSystemTime;
  ULARGE_INTEGER time, localTime;

  UnixTimeMsToFileTime (unix_ms, &fileTime);
  /* If time is earlier than year 1601, then always using year 1601 to query time zone adjustment */
  if (fileTime.dwHighDateTime >= 0x80000000)
  {
    fileTime.dwHighDateTime = 0;
    fileTime.dwLowDateTime = 0;
  }

  if (FileTimeToSystemTime (&fileTime, &systemTime)
      && SystemTimeToTzSpecificLocalTime (0, &systemTime, &localSystemTime)
      && SystemTimeToFileTime (&localSystemTime, &localFileTime))
  {
    time.LowPart = fileTime.dwLowDateTime;
    time.HighPart = fileTime.dwHighDateTime;
    localTime.LowPart = localFileTime.dwLowDateTime;
    localTime.HighPart = localFileTime.dwHighDateTime;
    return (double) (((LONGLONG) localTime.QuadPart - (LONGLONG) time.QuadPart) / TicksPerMs);
  }
  return 0.0;
#elif defined (__GNUC__) || defined (__clang__)
  time_t now_time = (time_t) (unix_ms / 1000);
  double tza_s = 0.0;

  while (true)
  {
    struct tm now_tm;
    if (!gmtime_r (&now_time, &now_tm))
    {
      break;
    }
    now_tm.tm_isdst = -1; /* if not overridden, DST will not be taken into account */
    time_t local_time = mktime (&now_tm);
    if (local_time == (time_t) -1)
    {
      break;
    }
    tza_s = difftime (now_time, local_time);

    if (is_utc)
    {
      break;
    }
    now_time -= (time_t) tza_s;
    is_utc = true;
  }

  return tza_s * 1000;
#else /* !HAVE_TM_GMTOFF && !_WIN32 && !__GNUC__ && !__clang__ */
  (void) unix_ms; /* unused */
  (void) is_utc; /* unused */
  return 0.0;
#endif /* HAVE_TM_GMTOFF */
} /* jerry_port_get_local_time_zone_adjustment */

/**
 * Default implementation of jerry_port_get_current_time. Uses 'gettimeofday' if
 * available on the system, does nothing otherwise.
 *
 * @return milliseconds since Unix epoch - if 'gettimeofday' is available and
 *                                         executed successfully,
 *         0 - otherwise.
 */
double jerry_port_get_current_time (void)
{
#ifdef _WIN32
  FILETIME ft;
  GetSystemTimeAsFileTime (&ft);
  return FileTimeToUnixTimeMs (ft);
#elif defined (__GNUC__) || defined (__clang__)
  struct timeval tv;

  if (gettimeofday (&tv, NULL) == 0)
  {
    return ((double) tv.tv_sec) * 1000.0 + ((double) tv.tv_usec) / 1000.0;
  }
  return 0.0;
#else /* !_WIN32 && !__GNUC__ && !__clang__ */
  return 0.0;
#endif /* _WIN32 */
} /* jerry_port_get_current_time */
