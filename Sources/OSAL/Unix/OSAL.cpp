/**
 * @file OSAL/Win32/OSAL.cpp
 * Contains the implementations of Unix specific code.
 *
 * @copyright Copyright (C) 2024 Faisal Al-Humaimidi
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
 //==============================================================================

#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "OSAL.hpp"

// Ignored on non-Windows.
void setUTF8CP() {}

// Ignored on non-Windows.
void restoreOriginalCP() {}

std::string followSymlink(const char* path, int* error) {
  std::string result;

  // Check if the error pointer is not NULL and initialize it to 0
  if (error != nullptr) {
    *error = 0;
  }

  // Unix-like systems implementation
  char resolved_path[PATH_MAX];
  if (realpath(path, resolved_path) != NULL) {
    result = std::string(resolved_path);
  }
  else {
    // If realpath fails, set the error to the errno value
    if (error != nullptr) {
      *error = errno;
    }
  }

  return result;
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
    // On Unix-like systems, colons are used as separators and cannot be escaped
    if (ch == ':') {
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
