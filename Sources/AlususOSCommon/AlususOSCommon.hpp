#ifndef ALUSUS_WIN32_COMMON_HPP
#define ALUSUS_WIN32_COMMON_HPP

#include <string>

namespace AlususOSCommon {

// Convert wchar_t* C string to UTF-8 C++ string.
std::string WideStringToUTF8(const wchar_t *wideStr);

// Get UTF-8 argv (will return false on failure).
bool getUTF8Argv(char *const **argv, char *const *currArgv);

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
