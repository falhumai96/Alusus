/**
 * @file OSAL/Win32/OSAL.cpp
 * Contains the implementations of Linux specific code.
 *
 * @copyright Copyright (C) 2024 Faisal Al-Humaimidi
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
 //==============================================================================

#include <stdlib.h>
#include <string.h>

#include "OSAL.hpp"

const char* getSystemLanguage() {
  // Get the LANGUAGE environment variable
  const char* langEnv = getenv("LANGUAGE");

  // Check if LANGUAGE starts with "ar"
  if (langEnv && strncmp(langEnv, "ar", 2) == 0) {
    return "ar";
  }

  // Get the LANG environment variable
  langEnv = getenv("LANG");

  // Check if LANG starts with "ar"
  if (langEnv && strncmp(langEnv, "ar", 2) == 0) {
    return "ar";
  }

  return "en";
}

const char* getShlibExt() {
  return ".so";
}
