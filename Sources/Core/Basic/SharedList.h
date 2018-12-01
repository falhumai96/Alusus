/**
 * @file Core/Basic/SharedList.h
 * Contains the header of class Core::Basic::SharedList.
 *
 * @copyright Copyright (C) 2018 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <http://alusus.net/alusus_license_1_0>.
 */
//==============================================================================

#ifndef CORE_BASIC_SHAREDLIST_H
#define CORE_BASIC_SHAREDLIST_H

namespace Core::Basic
{

template<class CTYPE> class SharedList : public SharedListBase<CTYPE, TiObject>
{
  //============================================================================
  // Type Info

  typedef SharedListBase<CTYPE, TiObject> _MyBase;
  TEMPLATE_TYPE_INFO(SharedList, _MyBase, "Core.Basic", "Core", "alusus.net", (CTYPE));


  //============================================================================
  // Constructors

  public: using SharedListBase<CTYPE, TiObject>::SharedListBase;

  public: static SharedPtr<SharedList> create(const std::initializer_list<SharedPtr<CTYPE>> &args)
  {
    return std::make_shared<SharedList>(args);
  }


  //============================================================================
  // Member Functions

  /// @name Abstract Implementations
  /// @{

  private: virtual SharedPtr<CTYPE> prepareForSet(
    Int index, SharedPtr<CTYPE> const &obj, Bool inherited, Bool newEntry
  ) {
    return obj;
  }

  private: virtual void prepareForUnset(
    Int index, SharedPtr<CTYPE> const &obj, Bool inherited
  ) {
  }

  /// @}

  /// @name Inheritted Functions
  /// @{

  public: void setBase(SharedList<CTYPE> *b)
  {
    SharedListBase<CTYPE, TiObject>::setBase(b);
  }

  public: SharedList<CTYPE>* getBase() const
  {
    return static_cast<SharedList<CTYPE>*>(this->base);
  }

  /// @}

}; // class

} // namespace

#endif
