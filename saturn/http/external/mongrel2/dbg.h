/**
 *
 * Copyright (c) 2010, Zed A. Shaw and Mongrel2 Project Contributors.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 *     * Neither the name of the Mongrel2 Project, Zed A. Shaw, nor the names
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 #ifndef __dbg_h__
 #define __dbg_h__
 
 #include <stdio.h>
 #include <errno.h>
 #include <string.h>
 #include <stdarg.h>
 
 void dbg_set_log(FILE *log_file);
 FILE *dbg_get_log();
 void dbg_set_log_level(int log_level);
 void write_log_message(FILE *log_file, int level, const char *format, ...);
 
 #define LOG_LEVEL_ERROR   0
 #define LOG_LEVEL_WARNING 1
 #define LOG_LEVEL_INFO    2
 #define LOG_LEVEL_DEBUG   3
 
 #define debug(M, ...) write_log_message(dbg_get_log(), LOG_LEVEL_DEBUG, "[DEBUG] %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
 
 // do not try to be smart and make this go away on NDEBUG, the _debug
 // here means that it just doesn't print a message, it still does the
 // check.  MKAY?
 #define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }
 
 #define clean_errno() (errno == 0 ? "None" : strerror(errno))
 
 #ifdef NO_LINENOS
 // versions that don't feature line numbers
 #define log_err(M, ...) write_log_message(dbg_get_log(), LOG_LEVEL_ERROR, "[ERROR] (errno: %s) " M "\n", clean_errno(), ##__VA_ARGS__)
 #define log_warn(M, ...) write_log_message(dbg_get_log(), LOG_LEVEL_WARNING, "[WARN] (errno: %s) " M "\n", clean_errno(), ##__VA_ARGS__)
 #define log_info(M, ...) write_log_message(dbg_get_log(), LOG_LEVEL_INFO, "[INFO] " M "\n", ##__VA_ARGS__)
 #else
 #define log_err(M, ...) write_log_message(dbg_get_log(), LOG_LEVEL_ERROR, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
 #define log_warn(M, ...) write_log_message(dbg_get_log(), LOG_LEVEL_WARNING, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
 #define log_info(M, ...) write_log_message(dbg_get_log(), LOG_LEVEL_INFO, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
 #endif
 
 #define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }
 
 #define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); errno=0; goto error; }
 
 #define check_mem(A) check((A), "Out of memory.")
 
 #define TRACE(C,E) debug("--> %s(%s:%d) %s:%d ", "" #C, State_event_name(E), E, __FUNCTION__, __LINE__)
 
 #define error_response(F, C, M, ...)  {Response_send_status(F, &HTTP_##C); sentinel(M, ##__VA_ARGS__);}
 
 #define error_unless(T, F, C, M, ...) if(!(T)) error_response(F, C, M, ##__VA_ARGS__)
 
 
 #endif