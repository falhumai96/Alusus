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

#include <filesystem>
#include <string>

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
  // Add more std::filesystem::path method replacement as necessary.

private:
  struct PathData;
  std::unique_ptr<PathData> m_data;
};

const Path &getModuleDirectory();

Path getWorkingDirectory();

} // Namespace AlususOSAL.

#endif
