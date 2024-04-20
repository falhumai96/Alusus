/**
 * @file Core/Basic/SbU32Str.h
 * Contains the header of class Core::Basic::SbU32Str.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#ifndef CORE_BASIC_SBU32STR_H
#define CORE_BASIC_SBU32STR_H

namespace Core::Basic
{

/**
 * @brief Static buffer string with comparison operations.
 * @ingroup basic_datatypes
 *
 * This class uses static string buffers instead of the dynamic Srl::U32String
 * class. This class is a wrappar around the U32Char* buffers. It treats 'this' as
 * the pointer to the buffer. This allows the user to easily cast any buffer
 * into this class.
 */
class SbU32Str
{
  //============================================================================
  // Member Variables

  private: U32Char *buf;


  //============================================================================
  // Constructors

  public: SbU32Str(U32Char *b) : buf(b)
  {
  }

  public: SbU32Str(Word *b) : buf(reinterpret_cast<U32Char*>(b))
  {
  }


  //============================================================================
  // Operators

  public: Bool operator==(U32Char const *s) const
  {
    return compareStr(this->getBuf(), s) == 0;
  }

  public: Bool operator!=(U32Char const *s) const
  {
    return compareStr(this->getBuf(), s) != 0;
  }

  public: Bool operator>(U32Char const *s) const
  {
    return compareStr(this->getBuf(), s) > 0;
  }

  public: Bool operator<(U32Char const *s) const
  {
    return compareStr(this->getBuf(), s) < 0;
  }

  public: operator U32Char const*() const {
    return this->buf;
  }


  //============================================================================
  // Functions

  /// @name UTF-32 Character Assigning Functions
  /// @{

  public: void assign(U32Char const *str, Word n, Word bufferSize);

  public: void assign(U32Char const *str, Word bufferSize)
  {
    this->assign(str, getStrLen(str), bufferSize);
  }

  public: void append(U32Char const *str, Word src_size, Word bufferSize);

  public: void append(U32Char const *str, Word bufferSize)
  {
    this->append(str, getStrLen(str), bufferSize);
  }

  /// @}

  /// @name Byte Character Assigning Functions
  /// @{

  public: void assign(Char const *str, Word n, Word bufferSize);

  public: void assign(Char const *str, Word bufferSize)
  {
    this->assign(str, 0, bufferSize);
  }

  public: void append(Char const *str, Word src_size, Word bufferSize);

  public: void append(Char const *str, Word bufferSize)
  {
    this->append(str, 0, bufferSize);
  }

  /// @}

  /// @name Other Functions
  /// @{

  public: Word getLength() const
  {
    if (this->getBuf() == 0) return 0;
    return getStrLen(this->getBuf());
  }

  public: U32Char const* getBuf() const
  {
    return this->buf;
  }

  public: U32Char* getBuf()
  {
    return this->buf;
  }

  /// @}

}; // class


/**
 * @brief Cast any const buffer into a const SbU32Str object.
 * @ingroup basic_datatypes
 */
template <class T> SbU32Str const sbu32str_cast(T const *b)
{
  return SbU32Str(const_cast<T*>(b));
}


/**
 * @brief Cast any buffer into a SbU32Str object.
 * @ingroup basic_datatypes
 */
template <class T> SbU32Str sbu32str_cast(T *b)
{
  return SbU32Str(b);
}


/**
 * @brief Wrapper for static buffer string literals (SbU32Str).
 * @ingroup basic_macros
 *
 * This wrapper is needed for future purposes. The main purpose of this is to
 * allow easy conversion of the program from ascii to unicode. This macro can
 * be thought as a combination of the S macro and sbu32str_cast.
 * Usage: SBU32STR("hello") == S("hello")
 */
#define SBU32STR(x)    Core::Basic::sbu32str_cast(x)

} // namespace

#endif
