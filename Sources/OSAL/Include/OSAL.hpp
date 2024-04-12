/**
 * @file OSAL/OSAL.hpp
 * Contains the declarations of platform specific code.
 *
 * @copyright Copyright (C) 2024 Faisal Al-Humaimidi
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
 //==============================================================================

#ifndef OSAL_HPP
#define OSAL_HPP

#include <string>
#include <streambuf>
#include <stdexcept>
#include <vector>

// Alusus compiler definitions.
#include "AlususDefs.h"

// Workaround for "windgi.h" for Windows in MinGW defining the same term.
// Unset ERROR if it was defined before include OSAL (record it temporarily before unsetting it).
#ifdef ERROR
#define TMP_ERROR ERROR
#undef ERROR
#endif

// Using APR and Cwalk for the bulk of the OSAL, and anything else missing from APR will be declared afterwards.
#include "apr_allocator.h"
#include "apr_atomic.h"
#include "apr_cstr.h"
#include "apr_dso.h"
#include "apr_env.h"
#include "apr_errno.h"
#include "apr_escape.h"
#include "apr_file_info.h"
#include "apr_file_io.h"
#include "apr_fnmatch.h"
#include "apr_general.h"
#include "apr_getopt.h"
#include "apr_global_mutex.h"
#include "apr_hash.h"
#include "apr_inherit.h"
#include "apr_lib.h"
#include "apr_mmap.h"
// These headers seems to be problematic in MinGW. We will add relevant declarations manually here instead when necessary.
// #include "apr_network_io.h"
// #include "apr_poll.h"
// #include "apr_portable.h"
// #include "apr_skiplist.h"
// #include "apr_support.h"
#include "apr_perms_set.h"
#include "apr_pools.h"
#include "apr_proc_mutex.h"
#include "apr_random.h"
#include "apr_ring.h"
#include "apr_shm.h"
#include "apr_signal.h"
#include "apr_strings.h"
#include "apr_tables.h"
#include "apr_thread_cond.h"
#include "apr_thread_mutex.h"
#include "apr_thread_proc.h"
#include "apr_thread_rwlock.h"
#include "apr_time.h"
#include "apr_user.h"
#include "apr_version.h"
#include "apr_want.h"
#include "apr.h"
#include "cwalk.h"

// Remove ERROR defined from APR/Cwalk.
#ifdef ERROR
#undef ERROR
#endif

// Restore ERROR (from TMP_ERROR) if it was defined earlier.
#ifdef TMP_ERROR
#define ERROR TMP_ERROR
#undef TMP_ERROR
#endif

// Do not call `apr_pool_destroy` manually on the pool.
class AutoAPRPool {
private:
  apr_pool_t* pool;

public:
  AutoAPRPool(apr_pool_t* parent = nullptr);
  ~AutoAPRPool();

  const apr_pool_t* getPool() const;
  apr_pool_t* getPool();
};

// You must not close your file with APR's C API.
// Either you call `close` from this class or let
// it auto-destruct (which will eventually call `close`
// internally).
// Not a thread-safe API.
class AutoAPRFile {
private:
  apr_file_t* file;

public:
  AutoAPRFile(apr_file_t* file);
  ~AutoAPRFile();

  void close();

  const apr_file_t* getFile() const;
  apr_file_t* getFile();
};

// Set the Windows UTF-8 codepages for input/output.
// It doesn't have any effect on other OSes. Not a thread-safe function, but can be
// called many times within one thread. Intended to be called in "main" at the very
// beginning only.
void setUTF8CP();

// Restores the original codepage. Not a thread-safe function, but can be called
// many times within one thread. Intended to be called at exit.
void restoreOriginalCP();

// Wrapper around `apr_file_t` to be used with C++ STD streams.
class APRFilebuf : public std::streambuf {
private:
  apr_file_t* file;
  char buffer;

protected:
  int_type overflow(int_type c) override;
  int_type underflow() override;
  pos_type seekoff(off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode mode) override;
  pos_type seekpos(pos_type pos, std::ios_base::openmode mode) override;

public:
  APRFilebuf(apr_file_t* file);
};

// Supports:
//    - [Default] English: "en".
//    - Arabic: "ar".
const char* getSystemLanguage();

// Follow the passed-in path symlink.
std::string followSymlink(const char* path, int* error = nullptr);

// System shared library extension.
const char* getShlibExt();

// Split PATH-like environment variable string into list of paths using the system
// PATH delimiter as the separator (e.g. `:` on Unix or `;` on Windows).
// It will also escape the delimeter when supported by the underlying system.
std::vector<std::string> splitPath(const char* pathEnv, size_t len = 0);

#endif
