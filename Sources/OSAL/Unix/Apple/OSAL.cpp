/**
 * @file OSAL/Win32/OSAL.cpp
 * Contains the implementations of Apple (e.g. macOS) specific code.
 *
 * @copyright Copyright (C) 2024 Faisal Al-Humaimidi
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
 //==============================================================================

#include <CoreFoundation/CoreFoundation.h>

#include "OSAL.hpp"

const char* getSystemLanguage() {
  CFArrayRef languages = CFPreferencesCopyAppValue(CFSTR("AppleLanguages"), kCFPreferencesCurrentApplication);
  CFStringRef languageCode = (CFStringRef)CFArrayGetValueAtIndex(languages, 0);

  // Check if language is Arabic.
  if (CFStringCompare(languageCode, CFSTR("ar"), 0) == kCFCompareEqualTo) {
    return "ar";
  }

  return "en";
}

const char* getShlibExt() {
  return ".dylib";
}
