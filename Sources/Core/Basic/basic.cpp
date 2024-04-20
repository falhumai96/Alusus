/**
 * @file Core/Basic/basic.cpp
 * Contains the global implementations of Basic namespace's declarations.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
 //==============================================================================

#include "OSAL.hpp"
#include "core.h"
#include "strs.h"

#include <locale>
#include <codecvt>
#include <algorithm>

namespace Core { namespace Basic
{

//============================================================================
// Variables and Types

// typedef std::codecvt<U32Char,Char,std::mbstate_t> FacetType;

// static std::locale utf8Locale("en_US.UTF-8");

// static const FacetType& utf8Facet = std::use_facet<FacetType>(utf8Locale);


//============================================================================
// Global Functions

Int compareStr(Char const *str1, Char const *str2)
{
  if (str1 == 0 && str2 == 0) return 0;
  else if (str1 == 0) return -1;
  else if (str2 == 0) return 1;
  else return strcmp(str1, str2);
}


Int compareStr(U32Char const *str1, U32Char const *str2)
{
  if (str1 == 0 && str2 == 0) return 0;
  else if (str1 == 0) return -1;
  else if (str2 == 0) return 1;
  else return u32_strcmp(str1, str2);
}


Int compareStr(Char const *str1, Char const *str2, Int size)
{
  if (str1 == 0 && str2 == 0) return 0;
  else if (str1 == 0) return -1;
  else if (str2 == 0) return 1;
  else return strncmp(str1, str2, size);
}


Int compareStr(U32Char const *str1, U32Char const *str2, Int size)
{
  if (str1 == 0 && str2 == 0) return 0;
  else if (str1 == 0) return -1;
  else if (str2 == 0) return 1;
  else return u32_strncmp(str1, str2, size);
}


Bool compareStrSuffix(Char const *str, Char const *suffix)
{
  Word strLen = getStrLen(str);
  Word suffixLen = getStrLen(suffix);
  if (suffixLen >= strLen) return false;
  return compareStr(str + strLen - suffixLen, suffix) == 0;
}


void convertStr(
  Char const *input, int inputLength, U32Char *output, int outputSize,
  int &processedInputLength, int &resultedOutputLength
) {
  // std::mbstate_t mystate = std::mbstate_t();
  // Char const* fromNext;
  // U32Char* toNext;

  // // translate characters:
  // utf8Facet.in(mystate, input, input + inputLength, fromNext, output, output + outputSize, toNext);

  // processedInputLength = fromNext - input;
  // resultedOutputLength = toNext - output;

  // Initialize the resulting lengths to zero
  processedInputLength = 0;
  resultedOutputLength = 0;

  std::string inputString(input, inputLength);
  std::u32string convertedString;
  try {
    convertedString = utf8_to_utf32(inputString);
  } catch (...) {
    return;
  }

  size_t copySize = std::min(convertedString.size() * sizeof(U32Char), (size_t)outputSize * sizeof(U32Char));
  std::memcpy(output, convertedString.c_str(), copySize);

  processedInputLength = inputLength;
  resultedOutputLength = (int)(copySize / sizeof(U32Char));
}


void convertStr(
  U32Char const *input, int inputLength, Char *output, int outputSize,
  int &processedInputLength, int &resultedOutputLength
) {
  // std::mbstate_t mystate = std::mbstate_t();
  // U32Char const *fromNext;
  // Char *toNext;

  // // translate characters:
  // utf8Facet.out(mystate, input, input+inputLength, fromNext, output, output+outputSize, toNext);

  // processedInputLength = fromNext-input;
  // resultedOutputLength = toNext-output;

  // Initialize the resulting lengths to zero
  processedInputLength = 0;
  resultedOutputLength = 0;

  std::u32string inputString(input, inputLength);
  std::string convertedString;
  try {
    convertedString = utf32_to_utf8(inputString);
  } catch (...) {
    return;
  }

  size_t copySize = std::min(convertedString.size(), (size_t)outputSize);
  std::memcpy(output, convertedString.c_str(), copySize);

  processedInputLength = inputLength;
  resultedOutputLength = (int)copySize;
}


U32Char getWideCharFromUtf8(Char const *s)
{
  U32Char buf[2];
  SBU32STR(buf).assign(s, 2);
  return *buf;
}


U32Char getWideCharFromUtf8(Char c)
{
  Char s[2];
  U32Char buf[2];
  s[0] = c;
  s[1] = U'\0';
  SBU32STR(buf).assign(s, 2);
  return *buf;
}


Int parseHexDigit(U32Char wc)
{
  static U32Char zero = getWideCharFromUtf8('0');
  static U32Char nine = getWideCharFromUtf8('9');
  static U32Char a = getWideCharFromUtf8('a');
  static U32Char f = getWideCharFromUtf8('f');
  static U32Char bigA = getWideCharFromUtf8('A');
  static U32Char bigF = getWideCharFromUtf8('F');
  if (wc >= a && wc <= f) {
    return (wc - a) + 10;
  } else if (wc >= bigA && wc <= bigF) {
    return (wc - bigA) + 10;
  } else if (wc >= zero && wc <= nine) {
    return wc - zero;
  } else {
    throw EXCEPTION(InvalidArgumentException, S("wc"), S("Invalid hex digit"), wc);
  }
}


Int parseHexDigits(U32Char const *wc, Word count)
{
  Int val = 0;
  while (count > 0) {
    val *= 16;
    val += parseHexDigit(*wc);
    ++wc;
    --count;
  }
  return val;
}


void printIndents(OutStream &stream, int indents)
{
  for (Int i=0; i < indents; ++i) {
    stream << S(" ");
  }
}

} } // namespace
