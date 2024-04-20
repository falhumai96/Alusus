/**
 * @file Core/Basic/TiU32Str.h
 * Contains the header of class Core::Basic::TiU32Str.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#ifndef CORE_BASIC_TIU32STR_H
#define CORE_BASIC_TIU32STR_H

namespace Core::Basic
{

/**
 * @brief An identifiable object that holds a string value.
 * @ingroup basic_datatypes
 */
template <class P> class TiU32StrBase : public P
{
  //============================================================================
  // Type Info

  TEMPLATE_TYPE_INFO(TiU32StrBase, P, "Core.Data", "Core", "alusus.org", (P));
  OBJECT_FACTORY(TiU32StrBase<P>);


  //============================================================================
  // Member Variables

  private: U32Str value;


  //============================================================================
  // Constructors

  public: TiU32StrBase()
  {
  }

  public: TiU32StrBase(U32Char const *v) : value(v)
  {
  }

  public: TiU32StrBase(U32Char const *v, Word c) : value(v, c)
  {
  }

  public: TiU32StrBase(Char const *v, Word c = 0) : value(v, c)
  {
  }

  public: static SharedPtr<TiU32StrBase<P>> create(U32Char const *v)
  {
    return newSrdObj<TiU32StrBase<P>>(v);
  }

  public: static SharedPtr<TiU32StrBase<P>> create(U32Char const *v, Word c)
  {
    return newSrdObj<TiU32StrBase<P>>(v, c);
  }

  public: static SharedPtr<TiU32StrBase<P>> create(Char const *v, Word c)
  {
    return newSrdObj<TiU32StrBase<P>>(v, c);
  }


  //============================================================================
  // Operators

  public: TiU32StrBase<P>& operator=(TiU32StrBase<P> const &v)
  {
    this->value = v.value;
    return *this;
  }

  public: TiU32StrBase<P>& operator=(U32Char const *v)
  {
    this->value = v;
    return *this;
  }

  public: TiU32StrBase<P>& operator=(Char const *v)
  {
    this->value = v;
    return *this;
  }

  public: operator U32Char const*() const
  {
    return this->value;
  }

  public: Bool operator==(TiU32StrBase<P> const &s) const
  {
    return this->value.compare(s.value) == 0;
  }

  public: Bool operator==(U32Char const *s) const
  {
    return this->value.compare(s) == 0;
  }

  public: Bool operator!=(TiU32StrBase<P> const &s) const
  {
    return this->value.compare(s.value) != 0;
  }

  public: Bool operator!=(U32Char const *s) const
  {
    return this->value.compare(s) != 0;
  }

  public: Bool operator>(TiU32StrBase<P> const &s) const
  {
    return this->value.compare(s.value) > 0;
  }

  public: Bool operator>(U32Char const *s) const
  {
    return this->value.compare(s) > 0;
  }

  public: Bool operator<(TiU32StrBase<P> const &s) const
  {
    return this->value.compare(s.value) < 0;
  }

  public: Bool operator<(U32Char const *s) const
  {
    return this->value.compare(s) < 0;
  }


  //============================================================================
  // Member Functions

  public: void set(U32Char const *v)
  {
    this->value = v;
  }

  public: void set(U32Char const *v, Word c)
  {
    this->value.assign(v, c);
  }

  public: U32Char const* get() const
  {
    return this->value;
  }

  public: U32Str const& getU32Str() const
  {
    return this->value;
  }

}; // class


//==============================================================================
// Typedefs

typedef TiU32StrBase<TiObject> TiU32Str;

} // namespace

#endif
