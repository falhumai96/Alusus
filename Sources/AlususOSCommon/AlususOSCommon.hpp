#ifndef ALUSUS_OS_COMMON
#define ALUSUS_OS_COMMON

#include <string>

namespace AlususOSCommon {

// Convert wchar_t* C string to UTF-8 C++ string.
std::string toUTF8String(const wchar_t *wideStr);

// Convert char* C string to UTF-8 C++ string.
std::string toUTF8String(const char *str);

// Convert char_t* C string to wide C++ string.
std::wstring toWideString(const char *narrowStr);

// Convert wchar_t* C string to wide C++ string.
std::wstring toWideString(const wchar_t *str);

// Get UTF-8 argv (will return false on failure).
bool getUTF8Argv(char *const **argv, char *const *currArgv);

// Initialize CMD code page. Must be called in "main".
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

} // Namespace AlususOSCommon.

#endif
