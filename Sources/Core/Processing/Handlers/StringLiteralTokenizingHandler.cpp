/**
 * @file Core/Processing/Handlers/StringLiteralTokenizingHandler.cpp
 * Contains the implementation of class
 * Core::Processing::Handlers::StringLiteralTokenizingHandler.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#include "core.h"

namespace Core::Processing::Handlers
{

//==============================================================================
// Overloaded Abstract Functions

void StringLiteralTokenizingHandler::prepareToken(
  Data::Token *token, Word id, U32Char const *tokenText, Word tokenTextLength,
  Data::SourceLocationRecord const &sourceLocation
) {
  // Precomputed UTF-32 characters values.
  static U32Char backSlashChar = getWideCharFromUtf8(S("\\"));
  static U32Char newLineChar = getWideCharFromUtf8(S("\n"));
  static U32Char carriageReturnChar = getWideCharFromUtf8(S("\r"));
  static U32Char formFeedChar = getWideCharFromUtf8(S("\f"));
  static U32Char tabChar = getWideCharFromUtf8(S("\t"));
  static U32Char nLetterChar = getWideCharFromUtf8(S("n"));
  static U32Char rLetterChar = getWideCharFromUtf8(S("r"));
  static U32Char tLetterChar = getWideCharFromUtf8(S("t"));
  static U32Char fLetterChar = getWideCharFromUtf8(S("f"));
  static U32Char jeemLetterChar = getWideCharFromUtf8(S("ج"));
  static U32Char raaLetterChar = getWideCharFromUtf8(S("ر"));
  static U32Char taaLetterChar = getWideCharFromUtf8(S("ت"));
  static U32Char doubleQuoteChar = getWideCharFromUtf8(S("\""));
  static U32Char singleQuoteChar = getWideCharFromUtf8(S("'"));
  static U32Char xLetterChar = getWideCharFromUtf8(S("x"));
  static U32Char hLetterChar = getWideCharFromUtf8(S("h"));
  static U32Char uLetterChar = getWideCharFromUtf8(S("u"));
  static U32Char bigULetterChar = getWideCharFromUtf8(S("U"));

  U32Char outerQuoteChar, innerQuoteChar;
  if (outerQuoteType == OuterQuoteType::DOUBLE) {
    outerQuoteChar = doubleQuoteChar;
    innerQuoteChar = singleQuoteChar;
  } else {
    outerQuoteChar = singleQuoteChar;
    innerQuoteChar = doubleQuoteChar;
  }

  // Set the token text after parsing control sequences and removing quotes.
  Word i = 0;
  Bool inStr = false;
  U32Char *buffer = reinterpret_cast<U32Char*>(SALLOC(tokenTextLength*sizeof(U32Char)));
  Word bufferLength = 0;
  while (i < tokenTextLength) {
    if (inStr) {
      if (tokenText[i] == outerQuoteChar) inStr = false;
      else if (tokenText[i] == backSlashChar) {
        ++i;
        if (tokenText[i] == outerQuoteChar) {
          buffer[bufferLength] = outerQuoteChar;
          ++bufferLength;
        } else if (tokenText[i] == innerQuoteChar) {
            buffer[bufferLength] = innerQuoteChar;
            ++bufferLength;
        } else if (tokenText[i] == backSlashChar) {
            buffer[bufferLength] = backSlashChar;
            ++bufferLength;
        } else if (tokenText[i] == nLetterChar || tokenText[i] == jeemLetterChar) {
          buffer[bufferLength] = newLineChar;
          ++bufferLength;
        } else if (tokenText[i] == rLetterChar || tokenText[i] == raaLetterChar) {
          buffer[bufferLength] = carriageReturnChar;
          ++bufferLength;
        } else if (tokenText[i] == tLetterChar || tokenText[i] == taaLetterChar) {
          buffer[bufferLength] = tabChar;
          ++bufferLength;
        } else if (tokenText[i] == fLetterChar) {
            buffer[bufferLength] = formFeedChar;
            ++bufferLength;
        } else if (tokenText[i] == xLetterChar || tokenText[i] == hLetterChar) {
          ++i;
          U32Char val = (U32Char)parseHexDigits(tokenText + i, 2);
          ++i;
          buffer[bufferLength] = val;
          ++bufferLength;
        } else if (tokenText[i] == uLetterChar) {
          ++i;
          U32Char val = (U32Char)parseHexDigits(tokenText + i, 4);
          i += 3;
          buffer[bufferLength] = val;
          ++bufferLength;
        } else if (tokenText[i] == bigULetterChar) {
          ++i;
          U32Char val = (U32Char)parseHexDigits(tokenText + i, 8);
          i += 7;
          buffer[bufferLength] = val;
          ++bufferLength;
        }
        // TODO: Parse other escape sequences.
      } else {
        buffer[bufferLength] = tokenText[i];
        ++bufferLength;
      }
    } else {
      if (tokenText[i] == outerQuoteChar) inStr = true;
    }
    ++i;
  }
  buffer[bufferLength] = WC('\0');
  token->setText(buffer, bufferLength);
  SFREE(buffer);
  // Set other token info.
  token->setId(id);
  token->setAsKeyword(false);
  token->setSourceLocation(sourceLocation);
}

} // namespace
