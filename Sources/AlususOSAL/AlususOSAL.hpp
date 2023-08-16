#ifndef ALUSUS_OSAL
#define ALUSUS_OSAL

#include <filesystem>
#include <string>

// UTF-8-aware OSAL (Operating System Abstraction Layer) used by Alusus.
namespace AlususOSAL {

// Convert to UTF-8 C++ string.
std::string toUTF8String(const wchar_t *wideCString);
std::string toUTF8String(const char *narrowCString);
std::string toUTF8String(const std::wstring wideString);
std::string toUTF8String(const std::string narrowString);

// Convert to wide C++ string.
std::wstring toWideString(const char *narrowCString);
std::wstring toWideString(const wchar_t *wideCString);
std::wstring toWideString(const std::string narrowString);
std::wstring toWideString(const std::wstring wideString);

// Get UTF-8 argv (will return false on failure).
bool getUTF8Argv(char *const **argv, char *const *currArgv);

// Initialize the UTF-8 code page. Must be called in "main".
class UTF8CodePage {
public:
  UTF8CodePage();
  ~UTF8CodePage();

private:
  void *m_data;
};

// DL functions.
void *dlopen(const char *__file, int __mode) noexcept(true);
char *dlerror() noexcept(true);
void *dlsym(void *__restrict __handle,
            const char *__restrict __name) noexcept(true);
int dlclose(void *__handle) noexcept(true);

class Path : public std::filesystem::path {
public:
  Path(const Path &other);
  Path(const std::filesystem::path &other);
  Path &operator=(const Path &other);
  Path &operator=(const std::filesystem::path &other);

  // C++17 and earlier UTF-8 path as std::string.
  std::string u8string() const;
};

} // Namespace AlususOSAL.

#endif
