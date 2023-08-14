#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#endif
#include <codecvt>
#include <locale>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "AlususOSCommon.hpp"

namespace AlususOSCommon {

std::string WideStringToUTF8(const wchar_t *wideStr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.to_bytes(wideStr);
}

bool getUTF8Argv(char *const **argv, char *const *currArgv) {

#if defined(ALUSUS_WIN32_UNICODE) && (defined(_WIN32) || defined(WIN32))

  static std::vector<char *> newArgv;
  static std::vector<std::string> newArgvData;
  static bool ret = false;
  std::once_flag flag;

  std::call_once(flag, []() {
    int nArgs;
    std::unique_ptr<LPWSTR[], decltype(&LocalFree)> argsMemory(
        CommandLineToArgvW(GetCommandLineW(), &nArgs), LocalFree);
    auto szArglist = argsMemory.get();
    if (!szArglist) {
      return;
    }

    for (int i = 0; i < nArgs; i++) {
      auto arg = szArglist[i];
      newArgvData.push_back(WideStringToUTF8(arg));
      newArgv.push_back((char *)newArgvData.back().c_str());
    }

    ret = true;
  });

  if (ret) {
    *argv = newArgv.data();
  }

  return ret;

#else

  *argv = currArgv;
  return true;

#endif
}

#if defined(ALUSUS_WIN32_UNICODE) && (defined(_WIN32) || defined(WIN32))

UTF8CodePage::UTF8CodePage()
    : m_oldCP(GetConsoleCP()), m_oldOutputCP(GetConsoleOutputCP()) {
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
}

UTF8CodePage::~UTF8CodePage() {
  SetConsoleCP(m_oldCP);
  SetConsoleOutputCP(m_oldOutputCP);
}

#else

UTF8CodePage::UTF8CodePage() : m_oldCP(0), m_oldOutputCP(0) {}

UTF8CodePage::~UTF8CodePage() {}

#endif

} // Namespace AlususOSCommon.
