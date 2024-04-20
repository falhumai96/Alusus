/**
 * @file Srl/strs.h
 * Contains string functions implementations.
 *
 * @copyright Copyright (C) 2023 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
 //==============================================================================

#include "strs.h"

#include <cstdarg>
#include <cinttypes>
#include <locale>
#include <codecvt>
#include <string>
#include <stdexcept>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstddef>
#include <errno.h>

namespace Srl
{

static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> u32Converter;

// Convert a UTF-32 string to a UTF-8 string
std::string utf32_to_utf8(const std::u32string& utf32) {
  // Possible code for migrating from C++17.
  // std::string utf8;
  // for (auto cp : utf32) {
  //   if (cp < 0x80) {
  //     utf8.push_back(static_cast<char>(cp));
  //   }
  //   else if (cp < 0x800) {
  //     utf8.push_back(static_cast<char>((cp >> 6) | 0xC0));
  //     utf8.push_back(static_cast<char>((cp & 0x3F) | 0x80));
  //   }
  //   else if (cp < 0x10000) {
  //     utf8.push_back(static_cast<char>((cp >> 12) | 0xE0));
  //     utf8.push_back(static_cast<char>(((cp >> 6) & 0x3F) | 0x80));
  //     utf8.push_back(static_cast<char>((cp & 0x3F) | 0x80));
  //   }
  //   else {
  //     utf8.push_back(static_cast<char>((cp >> 18) | 0xF0));
  //     utf8.push_back(static_cast<char>(((cp >> 12) & 0x3F) | 0x80));
  //     utf8.push_back(static_cast<char>(((cp >> 6) & 0x3F) | 0x80));
  //     utf8.push_back(static_cast<char>((cp & 0x3F) | 0x80));
  //   }
  // }
  // return utf8;

  return u32Converter.to_bytes(utf32.c_str());
}

// Convert a UTF-8 string to a UTF-32 string
std::u32string utf8_to_utf32(const std::string& utf8) {
  // Possible code for migrating from C++17.
  // std::u32string utf32;
  // for (size_t i = 0; i < utf8.size(); ) {
  //   char32_t cp;
  //   if ((utf8[i] & 0x80) == 0) {
  //     cp = utf8[i];
  //     ++i;
  //   }
  //   else if ((utf8[i] & 0xE0) == 0xC0) {
  //     if (i + 1 >= utf8.size()) throw std::runtime_error("Invalid UTF-8 sequence");
  //     cp = ((utf8[i] & 0x1F) << 6) | (utf8[i + 1] & 0x3F);
  //     i += 2;
  //   }
  //   else if ((utf8[i] & 0xF0) == 0xE0) {
  //     if (i + 2 >= utf8.size()) throw std::runtime_error("Invalid UTF-8 sequence");
  //     cp = ((utf8[i] & 0x0F) << 12) | ((utf8[i + 1] & 0x3F) << 6) | (utf8[i + 2] & 0x3F);
  //     i += 3;
  //   }
  //   else {
  //     if (i + 3 >= utf8.size()) throw std::runtime_error("Invalid UTF-8 sequence");
  //     cp = ((utf8[i] & 0x07) << 18) | ((utf8[i + 1] & 0x3F) << 12) | ((utf8[i + 2] & 0x3F) << 6) | (utf8[i + 3] & 0x3F);
  //     i += 4;
  //   }
  //   utf32.push_back(cp);
  // }
  // return utf32;

  return u32Converter.from_bytes(utf8.c_str());
}

// A swprintf-like function for char32_t strings
int u32_snprintf(char32_t* buffer, size_t size, const char32_t* format, ...) {
  if (!buffer || !size || !*buffer || !format || !*format) {
    return 0;
  }

  // Convert char32_t to std::string using utf32_to_utf8
  std::string str_format;
  try {
    str_format = utf32_to_utf8(format);
  } catch (...) {
    errno = EILSEQ;
    return -1;
  }

  // Prepare a buffer for vsnprintf
  std::vector<char> str_buffer(size);

  // Use vsnprintf to format the string
  va_list args;
  va_start(args, format);
  int result = vsnprintf(str_buffer.data(), size, str_format.c_str(), args);
  va_end(args);

  // Convert std::string back to char32_t using utf8_to_utf32
  std::u32string u32_buffer;
  try {
    u32_buffer = utf8_to_utf32(str_buffer.data());
  } catch (...) {
    errno = EILSEQ;
    return -1;
  }

  // Copy to the original buffer
  std::copy(u32_buffer.begin(), u32_buffer.end(), buffer);

  return result;
}

const char32_t* u32_strchr(const char32_t* str, char32_t ch) {
  if (!str) {
    return nullptr;
  }

  while (*str != U'\0') {
    if (*str == ch)
      return str;
    ++str;
  }

  if (ch == U'\0')
    return str;

  return nullptr;
}

const char32_t* u32_memchr(const char32_t* ptr, char32_t ch, size_t count) {
  if (!ptr || !*ptr || !count) {
    return nullptr;
  }

  while (count-- > 0) {
    if (*ptr == ch)
      return ptr;
    ++ptr;
  }

  // If the character was not found, return NULL
  return nullptr;
}

const char32_t* u32_strstr(const char32_t* str, const char32_t* substr) {
  if (!str || !substr || !*str || !*substr) {
    return nullptr;
  }

  if (!*substr)
    return str;
  for (; *str; ++str) {
    const char32_t* h = str;
    const char32_t* n = substr;
    while (*h && *n && *h == *n) {
      ++h;
      ++n;
    }
    if (!*n)
      return str;
  }
  return nullptr;
}

const char32_t* u32_strrchr(const char32_t* str, char32_t ch) {
  if (!str || !*str) {
    return nullptr;
  }

  if (ch == U'\0')
    return str;

  const char32_t* last_occurrence = nullptr;
  while (*str != U'\0') {
    if (*str == ch)
      last_occurrence = str;
    ++str;
  }

  // If the character was not found, return NULL
  return last_occurrence;
}

// here
int u32_strcmp(const char32_t* str1, const char32_t* str2) {
  while (*str1 && *str2 && (*str1 == *str2)) {
    ++str1;
    ++str2;
  }
  return *(char32_t*)str1 - *(char32_t*)str2;
}

int u32_strncmp(const char32_t* str1, const char32_t* str2, size_t count) {
  while (count && *str1 && *str2 && (*str1 == *str2)) {
    ++str1;
    ++str2;
    count--;
  }

  if (count == 0) {
    return 0;
  }

  return *(char32_t*)str1 - *(char32_t*)str2;
}

char32_t* u32_strcpy(char32_t* dest, const char32_t* src) {
  char32_t* original_dest = dest;
  while ((*dest++ = *src++));
  return original_dest;
}

char32_t* u32_strncpy(char32_t* dest, const char32_t* src, size_t count) {
  char32_t* original_dest = dest;
  while (count > 0 && *src != U'\0') {
    *dest = *src;
    dest++;
    src++;
    count--;
  }

  // Fill remaining characters in dest with null characters
  while (count > 0) {
    *dest = U'\0';
    dest++;
    count--;
  }

  return original_dest;
}

char32_t* u32_strcat(char32_t* dest, const char32_t* src) {
  char32_t* original_dest = dest;

  while (*dest)
    ++dest;
  while (*src)
    *dest++ = *src++;
  *dest = U'\0';
  return original_dest;
}

char32_t* u32_strncat(char32_t* dest, const char32_t* src, size_t count) {
  char32_t* original_dest = dest;
  
  if (!count) {
    return original_dest;
  }
  
  while (*dest)
    ++dest;

  while (*src && count--) {
    *dest++ = *src++;
  }

  *dest = U'\0';

  return original_dest;
}

size_t u32_strlen(const char32_t* str) {
  if (!str || !*str) {
    return 0;
  }

  size_t length = 0;
  while (*str++) {
    length++;
  }

  return length;
}

char32_t u32_towupper(char32_t ch) {
  // This is a simple implementation that only works for ASCII characters.
  // A full implementation would require a Unicode character database.
  if (ch >= U'a' && ch <= U'z')
    return ch - (U'a' - U'A');
  return ch;
}

char32_t u32_towlower(char32_t ch) {
  // This is a simple implementation that only works for ASCII characters.
  // A full implementation would require a Unicode character database.
  if (ch >= U'A' && ch <= U'Z')
    return ch + (U'a' - U'A');
  return ch;
}

}
