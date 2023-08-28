/**
 * @file AlususOSAL/AlususOSAL.hpp
 * Contains the declarations for the UTF-8-aware OSAL (Operating System
 * Abstraction Layer) used by Alusus.
 *
 * @copyright Copyright (C) 2023 Faisal Al-Humaimidi
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#ifndef ALUSUS_OSAL
#define ALUSUS_OSAL

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

// UTF-8-aware OSAL (Operating System Abstraction Layer) used by Alusus.
namespace AlususOSAL {

// Initialize the UTF-8 code page. Must be instantiated early in "main".
class UTF8CodePage {
public:
  UTF8CodePage();
  ~UTF8CodePage();

private:
  struct UTF8CodePageData;
  std::unique_ptr<UTF8CodePageData> m_data;
};

// Convert "main" arguments to use UTF-8. Must be instantiated early in "main".
class Args {
public:
  Args(int &argc, char **&argv);
  Args(int &argc, char **&argv, char **&en);
  ~Args();

private:
  class ArgsData;
  std::unique_ptr<ArgsData> m_data;
};

// DL functions drop-in replacement.
void *dlopen(const char *__file, int __mode) noexcept(true);
char *dlerror() noexcept(true);
void *dlsym(void *__restrict __handle,
            const char *__restrict __name) noexcept(true);
int dlclose(void *__handle) noexcept(true);

// std::filesystem::path drop-in replacement.
class Path {
public:
  Path();
  Path(char *path);
  Path(std::string &path);
  Path(const Path &other);
  Path(const std::filesystem::path &other);
  ~Path();
  Path &operator=(const char *other);
  Path &operator=(std::string &other);
  Path &operator=(const Path &other);
  Path &operator=(const std::filesystem::path &other);
  Path operator/(const char *other);
  Path operator/(std::string &other);
  Path operator/(const Path &other);
  Path operator/(const std::filesystem::path &other);
  Path &operator/=(const char *other);
  Path &operator/=(std::string &other);
  Path &operator/=(const Path &other);
  Path &operator/=(const std::filesystem::path &other);
  std::string string() const;
  const char *c_str() const;
  Path parent_path() const;
  Path filename() const;
  Path extension() const;
  bool exists() const;
  bool is_regular_file() const;
  bool is_directory() const;
  bool is_symlink() const;
  bool is_absolute() const;
  bool empty() const;
  Path absolute() const;
  Path canonical() const;
  // Add more std::filesystem::path method replacement as necessary.

private:
  struct PathData;
  std::unique_ptr<PathData> m_data;
};

// Get the directory where the module is.
const Path &getModuleDirectory();

// Get the current working directory.
Path getWorkingDirectory();

// Get the directory names to be added to the Alusus search path from within the
// installed package directory. These are defined at compile time and different
// between OSes (e.g., binary and library directories to be added for Windows
// (shared libraries are stored in the binary directory), while library
// directory to be added only for Unix).
const std::vector<char *> &getAlususPackageLibDirNames();

// File stream.
std::unique_ptr<std::basic_istream<char>>
ifstreamOpenFile(char const *filename);
std::unique_ptr<std::basic_istream<char>>
ifstreamOpenFile(std::string const &filename);
std::unique_ptr<std::basic_ostream<char>>
ofstreamOpenFile(char const *filename);
std::unique_ptr<std::basic_ostream<char>>
ofstreamOpenFile(std::string const &filename);

// STD stream.
std::basic_istream<char> &getCin();
std::basic_ostream<char> &getCout();
std::basic_ostream<char> &getCerr();

// CSTDIO functions.
FILE *freopen(const char *filename, const char *mode, FILE *stream);
FILE *fopen(const char *filename, const char *mode);
int rename(const char *old_name, const char *new_name);
int remove(const char *name);

// CSTDLIB functions.
char *getenv(const char *key);
int system(const char *cmd);
int setenv(const char *key, const char *value, int overwrite);
int unsetenv(const char *key);
int putenv(char *c_string);

// Create possible OS lib names based on the OS being compiled for and whether
// or not this is a debug build.
std::vector<std::string> constructShlibNames(char const *libname);
std::vector<std::string> constructShlibNames(std::string const &libname);
std::vector<std::string> constructShlibNames(Path const &libname);

// Parse a string of the format found in PATH/Path (Unix/Windows) environment
// variable into a list of <Path>s.
std::vector<Path> parsePathVariable(char const *pathVar);
std::vector<Path> parsePathVariable(std::string const &pathVar);

} // Namespace AlususOSAL.

#endif
