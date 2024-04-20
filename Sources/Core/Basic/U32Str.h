/**
 * @file Core/Basic/U32Str.h
 * Contains the header of class Core::Basic::U32Str.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#ifndef CORE_BASIC_WSTR_H
#define CORE_BASIC_WSTR_H

namespace Core::Basic
{

/**
 * @brief Basic UTF-32 string functionality with comparison operators.
 * @ingroup basic_datatypes
 *
 * This class overrides std's u32string class to provide comparison operators.
 */
class U32Str : public Srl::U32String
{
  //============================================================================
  // Constructors

  public: using Srl::U32String::U32String;

  public: U32Str(Srl::U32String const &str) : Srl::U32String(str)
  {
  }

  public: U32Str(U32Char const *str, LongInt pos, LongInt n)
  {
    this->assign(str, pos, n);
  }

  public: U32Str(Char const *str, LongInt pos=0, LongInt n=0)
  {
    this->assign(str, pos, n);
  }


  //============================================================================
  // Functions

  using Srl::U32String::assign;

  public: void assign(U32Char const *buf, LongInt pos, LongInt n);

  public: void assign(Char const *buf, LongInt pos, LongInt n);

  public: void assign(Char const *s, Word n=0)
  {
    if (n == 0) n = getStrLen(s);
    std::vector<U32Char> buffer(n + 1, 0);
    Int inLength, outLength;
    convertStr(s, n, (U32Char*) buffer.data(), n, inLength, outLength);
    this->assign((U32Char*) buffer.data(), outLength);
  }

  public: SbU32Str const sbu32str() const
  {
    return sbu32str_cast(this->getBuf());
  }

}; // class

} // namespace

#endif
