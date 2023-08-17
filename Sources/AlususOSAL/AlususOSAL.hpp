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

// Get UTF-8 argv (will return false on failure).
bool getUTF8Argv(char *const **argv, char *const *currArgv);

// Initialize the UTF-8 code page. Must be called in "main".
class UTF8CodePage {
public:
  UTF8CodePage();
  ~UTF8CodePage();

private:
  struct UTF8CodePageData;
  std::unique_ptr<UTF8CodePageData> m_data;
};

// DL functions.
void *dlopen(const char *__file, int __mode) noexcept(true);
char *dlerror() noexcept(true);
void *dlsym(void *__restrict __handle,
            const char *__restrict __name) noexcept(true);
int dlclose(void *__handle) noexcept(true);

// UTF-8 path.
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
  std::string u8string() const;
  const char *u8c_str() const;

private:
  struct PathData;
  std::unique_ptr<PathData> m_data;
};

} // Namespace AlususOSAL.

#endif
