/**
 * @file AlususOSAL/AlususOSAL.cpp
 * Contains the definitions for the UTF-8-aware OSAL (Operating System
 * Abstraction Layer) used by Alusus.
 *
 * @copyright Copyright (C) 2023 Faisal Al-Humaimidi
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#include "AlususDefs.h"
#include <cstddef>
#include <string.h>
#if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) &&               \
    !defined(__CYGWIN__)
#define ALUSUS_WIN32
#elif defined(__APPLE__)
#define ALUSUS_APPLE
#elif defined(__linux__)
#define ALUSUS_LINUX
#else
#error "Unsupported platform for AlususOSAL."
#endif

// Unicode supported == Either Windows with Unicode support or no Windows
#if (defined(ALUSUS_WIN32) && defined(ALUSUS_WIN32_UNICODE)) ||                \
    !defined(ALUSUS_WIN32)
#define ALUSUS_UNICODE_SUPPORTED
#endif

#if defined(ALUSUS_WIN32)
// Windows includes.
#include <Windows.h>
#else
// Unix includes.
#if defined(ALUSUS_LINUX)
// Linux includes.
#include <limits.h>
#include <unistd.h>
#elif defined(ALUSUS_APPLE)
// Apple includes.
#include <mach-o/dyld.h>
#endif
#include <dlfcn.h>
static constexpr void *(*__dlopen)(const char *, int) = dlopen;
static constexpr char *(*__dlerror)() = dlerror;
static constexpr void *(*__dlsym)(void *, const char *) = dlsym;
static constexpr int (*__dlclose)(void *) = dlclose;
#endif
#include <codecvt>
#include <locale>
#include <memory>
#include <mutex>
#if defined(ALUSUS_UNICODE_SUPPORTED)
#include <nowide/args.hpp>
#include <nowide/cstdio.hpp>
#include <nowide/cstdlib.hpp>
#include <nowide/fstream.hpp>
#include <nowide/iostream.hpp>
#else
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#if defined(ALUSUS_WIN32)
#include <cstring>
static int __setenv(const char *key, const char *value, int overwrite) {
  if (!overwrite) {
    return (std::getenv(key) ? 0 : -1);
  }
  return (SetEnvironmentVariable(key, value) ? 0 : -1);
}

static int __unsetenv(const char *key) {
  return (SetEnvironmentVariable(key, NULL) ? 0 : -1);
}

static int __putenv(char *c_string) {
  // Get the key.
  size_t key_start = 0;
  size_t key_end = 0;
  size_t current_idx = 0;
  size_t c_string_len = std::strlen(c_string);
  while (c_string[current_idx] != '=' && c_string[current_idx] != '\0') {
    current_idx++;
  }
  // Make sure there is a key ('=' is not the first character) and there exists
  // a '=' character after the key.
  if (current_idx == 0 || current_idx == c_string_len) {
    return -1;
  }
  key_end = current_idx;
  std::string keyString(c_string, key_start, key_end);
  auto key = keyString.c_str();

  // Get the value.
  auto value_start = current_idx + 1;
  auto value_end = c_string_len;
  std::string valueString(c_string, value_start, value_end);
  auto value = valueString.c_str();

  return (SetEnvironmentVariable(key, value) ? 0 : -1);
}
#else
static constexpr int (*__unsetenv)(const char *) = unsetenv;
static constexpr int (*__putenv)(const char *) = putenv;
static constexpr int (*__setenv)(const char *, const char *, int) = setenv;
#endif
#include <fstream>
#include <iostream>
#endif
#include <string>
#include <thread>
#include <vector>

#include "AlususOSAL.hpp"

namespace AlususOSAL {

// String conversion local functions for Windows with Unicode support.
#if defined(ALUSUS_WIN32_UNICODE)
static std::string toUTF8String(const wchar_t *wideCString) {
  return nowide::narrow(wideCString, nowide::utf::strlen(wideCString));
}
static std::string toUTF8String(const char *narrowCString) {
  return std::string(narrowCString);
}
static std::string toUTF8String(const std::wstring &wideString) {
  return nowide::narrow(wideString.c_str(), wideString.size());
}
static std::string toUTF8String(const std::string &narrowString) {
  return narrowString;
}

static std::wstring toWideString(const char *narrowCString) {
  return nowide::widen(narrowCString, nowide::utf::strlen(narrowCString));
}
static std::wstring toWideString(const wchar_t *wideCString) {
  return std::wstring(wideCString);
}
static std::wstring toWideString(const std::string &narrowString) {
  return nowide::widen(narrowString.c_str(), narrowString.size());
}
static std::wstring toWideString(const std::wstring &wideString) {
  return wideString;
}
#endif

#if defined(ALUSUS_UNICODE_SUPPORTED)

class Args::ArgsData : public nowide::args {
public:
  using nowide::args::args;
};

#else

class Args::ArgsData {};

#endif

Args::Args(int &argc, char **&argv) {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  m_data = std::make_unique<Args::ArgsData>(argc, argv);
#endif
}

Args::Args(int &argc, char **&argv, char **&en) {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  m_data = std::make_unique<Args::ArgsData>(argc, argv, en);
#endif
}

Args::~Args() {}

#if defined(ALUSUS_WIN32_UNICODE)

struct UTF8CodePage::UTF8CodePageData {
  unsigned int m_oldCP;
  unsigned int m_oldOutputCP;
};

UTF8CodePage::UTF8CodePage() {
  m_data = std::make_unique<UTF8CodePage::UTF8CodePageData>();
  m_data->m_oldCP = GetConsoleCP();
  m_data->m_oldOutputCP = GetConsoleOutputCP();
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
}

UTF8CodePage::~UTF8CodePage() {
  SetConsoleCP(m_data->m_oldCP);
  SetConsoleOutputCP(m_data->m_oldOutputCP);
}

#else

struct UTF8CodePage::UTF8CodePageData {};

UTF8CodePage::UTF8CodePage() {}

UTF8CodePage::~UTF8CodePage() {}

#endif

#if defined(ALUSUS_WIN32)

// Returns the last Win32 error, in string format. Returns an empty string if
// there is no error. Adapted from https://stackoverflow.com/a/17387176
// [accessed August 8th, 2023].
static std::string GetLastErrorAsString() {
  // Get the error message ID, if any.
  DWORD errorMessageID = ::GetLastError();
  if (errorMessageID == 0) {
    return std::string(); // No error message has been recorded
  }

  LPSTR messageBuffer = nullptr;

  // Ask Win32 to give us the string version of that message ID.
  // The parameters we pass in, tell Win32 to create the buffer that holds the
  // message for us (because we don't yet know how long the message string will
  // be).
  size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPSTR)&messageBuffer, 0, NULL);

  // Using std::unique_ptr to manage the messageBuffer memory.
  std::unique_ptr<char, decltype(&LocalFree)> bufferPtr(messageBuffer,
                                                        LocalFree);

  // Copy the error message into a std::string.
  std::string message(messageBuffer, size);

  return message;
}

// To store last Windows error related to library loading, per thread.
thread_local static std::string lastDLError;

#endif

void *dlopen(const char *__file, int __mode) noexcept(true) {

#if defined(ALUSUS_WIN32)

#if defined(ALUSUS_WIN32_UNICODE)

  auto wideStringPath = toWideString(__file);
  HMODULE handle = LoadLibraryW(wideStringPath.c_str());

#else

  HMODULE handle = LoadLibraryA(__file);

#endif

  if (!handle) {
    lastDLError = GetLastErrorAsString();
  }
  return handle;

#else

  return __dlopen(__file, __mode);

#endif
}

char *dlerror() noexcept(true) {

#if defined(ALUSUS_WIN32)

  return (char *)lastDLError.c_str();

#else

  return __dlerror();

#endif
}

void *dlsym(void *__restrict __handle,
            const char *__restrict __name) noexcept(true) {

#if defined(ALUSUS_WIN32)

  auto address = GetProcAddress((HMODULE)__handle, __name);
  if (!address) {
    lastDLError = GetLastErrorAsString();
  }
  return (void *)address;

#else

  return __dlsym(__handle, __name);

#endif
}

int dlclose(void *__handle) noexcept(true) {

#if defined(ALUSUS_WIN32)

  auto ret = FreeLibrary((HMODULE)__handle);
  if (!ret) {
    lastDLError = GetLastErrorAsString();
  }
  return !ret;

#else

  return __dlclose(__handle);

#endif
}

struct Path::PathData {
  std::filesystem::path osPath;

#if defined(ALUSUS_WIN32)

  std::optional<std::string> pathString;

#endif

  std::mutex mx;
};

static std::filesystem::path cleanup_path(std::filesystem::path const &path) {
  auto newPath = std::filesystem::weakly_canonical(path);
  if (!newPath.has_filename()) {
    newPath = newPath.parent_path();
  }
  return newPath;
}

Path::Path() { m_data = std::make_unique<Path::PathData>(); }

Path::Path(char *path) {
  m_data = std::make_unique<Path::PathData>();
#if defined(ALUSUS_WIN32_UNICODE)
  auto widePath = nowide::widen(std::string(path));
  auto osPath = std::filesystem::path(widePath);
#else
  auto osPath = std::filesystem::path(std::string(path));
#endif
  m_data->osPath = cleanup_path(osPath);
}

Path::Path(std::string &path) : Path::Path(path.c_str()) {}

Path::Path(const Path &other) : Path::Path(other.m_data->osPath) {}

Path::Path(const std::filesystem::path &other) {
  m_data = std::make_unique<Path::PathData>();
  m_data->osPath = cleanup_path(other);
}

Path::~Path() {}

Path &Path::operator=(const char *other) {
  std::unique_lock lock(m_data->mx);
#if defined(ALUSUS_WIN32_UNICODE)
  auto widePath = nowide::widen(std::string(other));
  auto osPath = std::filesystem::path(widePath);
#else
  auto osPath = std::filesystem::path(std::string(other));
#endif
  auto newPath = cleanup_path(osPath);
  m_data.reset();
  m_data = std::make_unique<Path::PathData>();
  m_data->osPath = newPath;
  return *this;
}

Path &Path::operator=(std::string &other) {
  std::unique_lock lock(m_data->mx);
#if defined(ALUSUS_WIN32_UNICODE)
  auto widePath = nowide::widen(other);
  auto osPath = std::filesystem::path(widePath);
#else
  auto osPath = std::filesystem::path(other);
#endif
  auto newPath = cleanup_path(osPath);
  m_data.reset();
  m_data = std::make_unique<Path::PathData>();
  m_data->osPath = newPath;
  return *this;
}

Path &Path::operator=(const Path &other) {
  if (this != &other) {
    std::unique_lock lock(m_data->mx);
    auto newPath = cleanup_path(other.m_data->osPath);
    m_data.reset();
    m_data = std::make_unique<Path::PathData>();
    m_data->osPath = newPath;
  }
  return *this;
}

Path &Path::operator=(const std::filesystem::path &other) {
  std::unique_lock lock(m_data->mx);
  auto newPath = cleanup_path(other);
  m_data.reset();
  m_data = std::make_unique<Path::PathData>();
  m_data->osPath = newPath;
  return *this;
}

Path Path::operator/(const char *other) {
  std::unique_lock lock(m_data->mx);
#if defined(ALUSUS_WIN32_UNICODE)
  auto widePath = nowide::widen(std::string(other));
  auto inputOSPath = std::filesystem::path(widePath);
#else
  auto inputOSPath = std::filesystem::path(std::string(other));
#endif
  auto osPath = m_data->osPath;
  osPath /= inputOSPath;
  return Path(osPath);
}

Path Path::operator/(std::string &other) {
  std::unique_lock lock(m_data->mx);
#if defined(ALUSUS_WIN32_UNICODE)
  auto widePath = nowide::widen(other);
  auto inputOSPath = std::filesystem::path(widePath);
#else
  auto inputOSPath = std::filesystem::path(other);
#endif
  auto osPath = m_data->osPath;
  osPath /= inputOSPath;
  return Path(osPath);
}

Path Path::operator/(const Path &other) {
  std::unique_lock lock(m_data->mx);
  auto osPath = m_data->osPath;
  osPath /= other.m_data->osPath;
  return Path(osPath);
}

Path Path::operator/(const std::filesystem::path &other) {
  std::unique_lock lock(m_data->mx);
  auto osPath = m_data->osPath;
  osPath /= other;
  return Path(osPath);
}

Path &Path::operator/=(const char *other) {
  std::unique_lock lock(m_data->mx);
#if defined(ALUSUS_WIN32_UNICODE)
  auto widePath = nowide::widen(std::string(other));
  auto inputOSPath = std::filesystem::path(widePath);
#else
  auto inputOSPath = std::filesystem::path(std::string(other));
#endif
  auto newOSPath = m_data->osPath;
  newOSPath /= inputOSPath;
  newOSPath = cleanup_path(newOSPath);
  m_data.reset();
  m_data = std::make_unique<Path::PathData>();
  m_data->osPath = newOSPath;
  return *this;
}

Path &Path::operator/=(std::string &other) {
  std::unique_lock lock(m_data->mx);
#if defined(ALUSUS_WIN32_UNICODE)
  auto widePath = nowide::widen(other);
  auto inputOSPath = std::filesystem::path(widePath);
#else
  auto inputOSPath = std::filesystem::path(other);
#endif
  auto newOSPath = m_data->osPath;
  newOSPath /= inputOSPath;
  newOSPath = cleanup_path(newOSPath);
  m_data.reset();
  m_data = std::make_unique<Path::PathData>();
  m_data->osPath = newOSPath;
  return *this;
}

Path &Path::operator/=(const Path &other) {
  std::unique_lock lock(m_data->mx);
  auto newOSPath = m_data->osPath;
  newOSPath /= other.m_data->osPath;
  newOSPath = cleanup_path(newOSPath);
  m_data.reset();
  m_data = std::make_unique<Path::PathData>();
  m_data->osPath = newOSPath;
  return *this;
}

Path &Path::operator/=(const std::filesystem::path &other) {
  std::unique_lock lock(m_data->mx);
  auto newOSPath = m_data->osPath;
  newOSPath /= other;
  newOSPath = cleanup_path(newOSPath);
  m_data.reset();
  m_data = std::make_unique<Path::PathData>();
  m_data->osPath = newOSPath;
  return *this;
}

std::string Path::string() const { return std::string(this->c_str()); }

const char *Path::c_str() const {
  std::unique_lock lock(m_data->mx);

#if defined(ALUSUS_WIN32)

  if (!m_data->pathString.has_value()) {

#if defined(ALUSUS_WIN32_UNICODE)

    m_data->pathString = toUTF8String(m_data->osPath.wstring());

#else

    m_data->pathString = m_data->osPath.string();

#endif
  }
  return m_data->pathString.value().c_str();
#else
  return m_data->osPath.c_str();
#endif
}

Path Path::parent_path() const {
  std::unique_lock lock(m_data->mx);
  auto parentPath = m_data->osPath.parent_path();
  return Path(parentPath);
}

Path Path::filename() const {
  std::unique_lock lock(m_data->mx);
  auto retFilename = m_data->osPath.filename();
  return Path(retFilename);
}

Path Path::extension() const {
  std::unique_lock lock(m_data->mx);
  auto retFilename = m_data->osPath.extension();
  return Path(retFilename);
}

bool Path::exists() const {
  std::unique_lock lock(m_data->mx);
  return std::filesystem::exists(m_data->osPath);
}

bool Path::is_regular_file() const {
  std::unique_lock lock(m_data->mx);
  return std::filesystem::is_regular_file(m_data->osPath);
}

bool Path::is_directory() const {
  std::unique_lock lock(m_data->mx);
  return std::filesystem::is_directory(m_data->osPath);
}

bool Path::is_symlink() const {
  std::unique_lock lock(m_data->mx);
  return std::filesystem::is_symlink(m_data->osPath);
}

bool Path::is_absolute() const {
  std::unique_lock lock(m_data->mx);
  return m_data->osPath.is_absolute();
}

bool Path::empty() const {
  std::unique_lock lock(m_data->mx);
  return m_data->osPath.empty();
}

Path Path::absolute() const {
  std::unique_lock lock(m_data->mx);
  return Path(std::filesystem::absolute(m_data->osPath));
}

Path Path::canonical() const {
  std::unique_lock lock(m_data->mx);
  return Path(std::filesystem::canonical(m_data->osPath));
}

static std::string _getModuleDirectory() {
#if defined(ALUSUS_WIN32)
  std::string emptyExePath;
  char *exePath;
#if defined(ALUSUS_WIN32_UNICODE)
  wchar_t *wExePath;
  std::string exePathString;
  if (_get_wpgmptr(&wExePath) == 0) {
    exePathString = toUTF8String(wExePath);
    exePath = (char *)exePathString.c_str();
  } else
#else
  if (_get_pgmptr(&exePath) != 0)
#endif
    exePath = (char *)emptyExePath.c_str();
#elif defined(ALUSUS_LINUX)
  char exePath[PATH_MAX];
  ssize_t len = ::readlink("/proc/self/exe", exePath, sizeof(exePath));
  if (len == -1 || len == sizeof(exePath))
    len = 0;
  exePath[len] = '\0';
#elif defined(ALUSUS_APPLE)
  char exePath[PATH_MAX];
  uint32_t len = sizeof(exePath);
  if (_NSGetExecutablePath(exePath, &len) != 0) {
    exePath[0] = '\0'; // buffer too small (!)
  } else {
    // resolve symlinks, ., .. if possible
    char *canonicalPath = realpath(exePath, NULL);
    if (canonicalPath != NULL) {
      strncpy(exePath, canonicalPath, len);
      free(canonicalPath);
    }
  }
#endif
  return std::string(exePath);
}

const Path &getModuleDirectory() {
  thread_local static AlususOSAL::Path path =
      Path(_getModuleDirectory()).parent_path();
  return path;
}

Path getWorkingDirectory() { return std::filesystem::current_path(); }

const std::vector<char *> &getAlususPackageLibDirNames() {
  static std::vector<char *> libDirNames = {
    (char *)ALUSUS_LIB_DIR_NAME
#if defined(ALUSUS_WIN32)
    ,
    (char *)ALUSUS_BIN_DIR_NAME
#endif
  };
  return libDirNames;
}

std::unique_ptr<std::basic_istream<char>>
ifstreamOpenFile(char const *filename) {
  auto fileHandle =
#if defined(ALUSUS_UNICODE_SUPPORTED)
      std::make_unique<nowide::ifstream>(filename);
#else
      std::make_unique<std::ifstream>(filename);
#endif
  return fileHandle;
}

std::unique_ptr<std::basic_istream<char>>
ifstreamOpenFile(std::string const &filename) {
  return ifstreamOpenFile(filename.c_str());
}

std::unique_ptr<std::basic_ostream<char>>
ofstreamOpenFile(char const *filename) {
  auto fileHandle =
#if defined(ALUSUS_UNICODE_SUPPORTED)
      std::make_unique<nowide::ofstream>(filename);
#else
      std::make_unique<std::ofstream>(filename);
#endif
  return fileHandle;
}

std::unique_ptr<std::basic_ostream<char>>
ofstreamOpenFile(std::string const &filename) {
  return ofstreamOpenFile(filename.c_str());
}

std::basic_istream<char> &getCin() {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  return nowide::cin;
#else
  return std::cin;
#endif
}

std::basic_ostream<char> &getCout() {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  return nowide::cout;
#else
  return std::cout;
#endif
}

std::basic_ostream<char> &getCerr() {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  return nowide::cerr;
#else
  return std::cerr;
#endif
}

FILE *freopen(const char *filename, const char *mode, FILE *stream) {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  return nowide::freopen(filename, mode, stream);
#else
  return std::freopen(filename, mode, stream);
#endif
}

FILE *fopen(const char *filename, const char *mode) {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  return nowide::fopen(filename, mode);
#else
  return std::fopen(filename, mode);
#endif
}

int rename(const char *old_name, const char *new_name) {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  return nowide::rename(old_name, new_name);
#else
  return std::rename(old_name, new_name);
#endif
}

int remove(const char *name) {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  return nowide::remove(name);
#else
  return std::remove(name);
#endif
}

char *getenv(const char *key) {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  return nowide::getenv(key);
#else
  return std::getenv(key);
#endif
}

int system(const char *cmd) {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  return nowide::system(cmd);
#else
  return std::system(cmd);
#endif
}

int setenv(const char *key, const char *value, int overwrite) {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  return nowide::setenv(key, value, overwrite);
#else
  return __setenv(key, value, overwrite);
#endif
}

int unsetenv(const char *key) {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  return nowide::unsetenv(key);
#else
  return __unsetenv(key);
#endif
}

int putenv(char *c_string) {
#if defined(ALUSUS_UNICODE_SUPPORTED)
  return nowide::putenv(c_string);
#else
  return __putenv(c_string);
#endif
}

std::vector<std::string> constructShlibNames(char const *libname) {
  std::vector<std::string> libnames;

#if defined(ALUSUS_USE_LOGS)

  // Push debug names.

#if defined(ALUSUS_WIN32)
  libnames.push_back(std::string("lib") + libname + ".dbg.dll");
  libnames.push_back(std::string(libname) + ".dbg.dll");
#elif defined(ALUSUS_APPLE)
  libnames.push_back(std::string("lib") + libname + ".dbg.dylib");
#else
  libnames.push_back(std::string("lib") + libname + ".dbg.so");
#endif

#endif

  // Push release names.

#if defined(ALUSUS_WIN32)
  libnames.push_back(std::string("lib") + libname + ".dll");
  libnames.push_back(std::string(libname) + ".dll");
#elif defined(ALUSUS_APPLE)
  libnames.push_back(std::string("lib") + libname + ".dylib");
#else
  libnames.push_back(std::string("lib") + libname + ".so");
#endif

  return libnames;
}

std::vector<std::string> constructShlibNames(std::string const &libname) {
  return constructShlibNames(libname.c_str());
}

std::vector<std::string> constructShlibNames(Path const &libname) {
  auto filename = libname.filename();
  return constructShlibNames(filename.c_str());
}

std::vector<Path> parsePathVariable(char const *pathVar) {
#if defined(ALUSUS_WIN32)
  constexpr char PATH_SEPARATOR = ';';
  constexpr char QUOTE_CHAR = '"';
#else
  constexpr char PATH_SEPARATOR = ':';
#endif

  char *currPathVar = (char *)pathVar;
  std::vector<Path> paths;
  std::string currPath;

  if (!pathVar) {
    return paths;
  }

  while (*currPathVar != '\0') {
    if (*currPathVar == PATH_SEPARATOR) {
      paths.push_back(currPath);
      currPath.clear();
    }
#if defined(ALUSUS_WIN32)
    // Windows also supports escaping the Path separator.
    else if (*currPathVar == QUOTE_CHAR) {
      currPathVar++; // Move past the opening quote.
      while (*currPathVar != '\0' && *currPathVar != QUOTE_CHAR) {
        currPath += *currPathVar;
        currPathVar++;
      }
      if (*currPathVar == QUOTE_CHAR) {
        currPathVar++; // Move past the closing quote.
      }
    }
#endif
    else {
      currPath += *currPathVar;
    }

    currPathVar++;
  }

  if (!currPath.empty()) {
    paths.push_back(currPath);
  }

  return paths;
}

std::vector<Path> parsePathVariable(std::string const &pathVar) {
  return parsePathVariable(pathVar.c_str());
}

} // Namespace AlususOSAL.
