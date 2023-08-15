#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#else
#include <dlfcn.h>
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

std::string toUTF8String(const wchar_t *wideStr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.to_bytes(wideStr);
}

std::string toUTF8String(const char *str) { return std::string(str); }

std::wstring toWideString(const char *narrowStr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.from_bytes(narrowStr);
}

std::wstring toWideString(const wchar_t *str) { return std::wstring(str); }

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
      newArgvData.push_back(toUTF8String(arg));
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

struct UTF8CodePageData {
  unsigned int m_oldCP;
  unsigned int m_oldOutputCP;
};

UTF8CodePage::UTF8CodePage() {
  m_data = new UTF8CodePageData;
  ((UTF8CodePageData *)m_data)->m_oldCP = GetConsoleCP();
  ((UTF8CodePageData *)m_data)->m_oldOutputCP = GetConsoleOutputCP();
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
}

UTF8CodePage::~UTF8CodePage() {
  SetConsoleCP(((UTF8CodePageData *)m_data)->m_oldCP);
  SetConsoleOutputCP(((UTF8CodePageData *)m_data)->m_oldOutputCP);
  delete ((UTF8CodePageData *)m_data);
  m_data = nullptr;
}

#else

UTF8CodePage::UTF8CodePage() : m_data(nullptr) {}

UTF8CodePage::~UTF8CodePage() {}

#endif

#if defined(_WIN32) || defined(WIN32)

// Returns the last Win32 error, in string format. Returns an empty string if
// there is no error. Adapted from https://stackoverflow.com/a/17387176
// [accessed August 8th, 2023].
std::string GetLastErrorAsString() {
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
thread_local std::string lastDLError;

#endif

void *dlopen(const char *__file, int __mode) noexcept(true) {
#if defined(_WIN32) || defined(WIN32)
#if defined(ALUSUS_WIN32_UNICODE)
  auto wideStringPath = toWideString(__file);
  HMODULE handle = LoadLibraryW(wideStringPath.c_str());
#else
  HMODULE handle = LoadLibraryA(path);
#endif
  if (!handle) {
    lastDLError = GetLastErrorAsString();
  }
  return handle;
#else
  return dlopen(__file, __mode);
#endif
}

char *dlerror() noexcept(true) {
#if defined(_WIN32) || defined(WIN32)
  return (char*) lastDLError.c_str();
#else
  return dlerror();
#endif
}

void *dlsym(void *__restrict __handle,
            const char *__restrict __name) noexcept(true) {
#if defined(_WIN32) || defined(WIN32)
  auto address = GetProcAddress((HMODULE) __handle, __name);
  if (!address) {
    lastDLError = GetLastErrorAsString();
  }
  return (void*) address;
#else
  return dlsym(__handle, __name);
#endif
}

int dlclose(void *__handle) noexcept(true) {
#if defined(_WIN32) || defined(WIN32)
  auto ret = FreeLibrary((HMODULE) __handle);
  if (!ret) {
    lastDLError = GetLastErrorAsString();
  }
  return !ret;
#else
  return dlclose(__handle);
#endif
}

} // Namespace AlususOSCommon.
