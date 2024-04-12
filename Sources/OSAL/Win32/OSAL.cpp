/**
 * @file OSAL/Unix/OSAL.cpp
 * Contains the implementations of Unix specific code.
 *
 * @copyright Copyright (C) 2024 Faisal Al-Humaimidi
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
 //==============================================================================

#include <Windows.h>
#include <locale>
#include <codecvt>
#include <memory>
#include <string.h>

#include "OSAL.hpp"

static UINT originalCP = 0;
static UINT originalOutputCP = 0;

void setUTF8CP() {
  if (!originalCP) {
    originalCP = GetConsoleCP();
    SetConsoleCP(CP_UTF8);
  }
  if (!originalOutputCP) {
    originalOutputCP = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
  }
}

void restoreOriginalCP() {
  if (originalCP) {
    SetConsoleCP(originalCP);
    originalCP = 0;
  }
  if (originalOutputCP) {
    SetConsoleOutputCP(originalOutputCP);
    originalOutputCP = 0;
  }
}

const char* getSystemLanguage() {
  // Get the primary language ID for the user default UI language
  LANGID langId = GetUserDefaultUILanguage();

  // Check if language is Arabic
  if (langId == MAKELANGID(LANG_ARABIC, SUBLANG_DEFAULT)) {
    return "ar";
  }

  return "en";
}

// Custom deleter for HANDLE
struct HandleDeleter {
  typedef HANDLE pointer;
  void operator()(HANDLE handle) {
    if (handle != INVALID_HANDLE_VALUE) {
      CloseHandle(handle);
    }
  }
};

std::string followSymlink(const char* path, int* error) {
  std::string result;

  // Check if the error pointer is not NULL and initialize it to 0
  if (error) {
    *error = 0;
  }

  // Windows implementation
#if APR_HAS_UNICODE_FS
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wpath = converter.from_bytes(path);

  // Use std::unique_ptr with custom deleter for the HANDLE
  std::unique_ptr<HANDLE, HandleDeleter> fileHandle(CreateFileW(wpath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL), HandleDeleter());
  if (fileHandle.get() == INVALID_HANDLE_VALUE) {
    if (error) {
      *error = GetLastError();
    }
    return result;
  }

  DWORD len = GetFinalPathNameByHandleW(fileHandle.get(), NULL, 0, VOLUME_NAME_DOS);
  if (len != 0) {
    std::unique_ptr<wchar_t[]> buffer(new wchar_t[len]);
    len = GetFinalPathNameByHandleW(fileHandle.get(), buffer.get(), len, VOLUME_NAME_DOS);
    if (len != 0) {
      result = converter.to_bytes(std::wstring(buffer.get(), len));
    }
    else {
      if (error) {
        *error = GetLastError();
      }
    }
  }
  else {
    if (error) {
      *error = GetLastError();
    }
  }
#else
  // Use std::unique_ptr with custom deleter for the HANDLE
  std::unique_ptr<HANDLE, HandleDeleter> fileHandle(CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL), HandleDeleter());
  if (fileHandle.get() == INVALID_HANDLE_VALUE) {
    if (error) {
      *error = GetLastError();
    }
    return result;
  }

  DWORD len = GetFinalPathNameByHandleA(fileHandle.get(), NULL, 0, VOLUME_NAME_DOS);
  if (len != 0) {
    std::unique_ptr<char[]> buffer(new char[len]);
    len = GetFinalPathNameByHandleA(fileHandle.get(), buffer.get(), len, VOLUME_NAME_DOS);
    if (len != 0) {
      result = std::string(buffer.get(), len);
    }
    else {
      if (error) {
        *error = GetLastError();
      }
    }
  }
  else {
    if (error) {
      *error = GetLastError();
    }
  }
#endif

  return result;
}

const char* getShlibExt() {
  return ".dll";
}

std::vector<std::string> splitPath(const char* pathEnv, size_t len) {
  if (len == 0) {
    len = strlen(pathEnv);
  }

  std::vector<std::string> paths;
  std::string currentPath;
  currentPath.reserve(len); // Reserve enough space to avoid reallocations

  for (size_t i = 0; i < len; ++i) {
    char ch = pathEnv[i];

    // On Windows, paths may be enclosed in quotes to escape semicolons
    if (ch == '\"') {
      // Skip the initial quote
      ++i;
      // Append characters until the closing quote or end of string
      while (i < len && pathEnv[i] != '\"') {
        currentPath += pathEnv[i++];
      }
      if (i < len && pathEnv[i] == '\"') {
        // Skip the closing quote
        ++i;
      }
    }
    else if (ch == ';') {
      if (!currentPath.empty()) {
        paths.push_back(currentPath);
        currentPath.clear();
      }
    }
    else {
      currentPath += ch;
    }
  }

  // Add the last path if there is one
  if (!currentPath.empty()) {
    paths.push_back(currentPath);
  }

  return paths;
}
