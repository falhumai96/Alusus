#ifndef ALUSUS_OS_COMMON
#define ALUSUS_OS_COMMON

#include <string>

namespace AlususOSCommon {

// Convert wchar_t* C string to UTF-8 C++ string.
std::string toUTF8String(const wchar_t *wideStr);

// Convert wchar_t* C string to UTF-8 C++ string.
std::string toUTF8String(const char *str);

// Get UTF-8 argv (will return false on failure).
bool getUTF8Argv(char *const **argv, char *const *currArgv);

// Initialize CMD code page. Must be called in "main".
class UTF8CodePage {
public:
  UTF8CodePage();
  ~UTF8CodePage();

private:
  unsigned int m_oldCP;
  unsigned int m_oldOutputCP;
};

} // Namespace AlususOSCommon.

#endif
