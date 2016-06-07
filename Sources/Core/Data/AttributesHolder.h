/**
 * @file Core/Data/AttributesHolder.h
 * Contains the header of interface Data::AttributesHolder.
 *
 * @copyright Copyright (C) 2014 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <http://alusus.net/alusus_license_1_0>.
 */
//==============================================================================

#ifndef CORE_DATA_ATTRIBUTESHOLDER_H
#define CORE_DATA_ATTRIBUTESHOLDER_H

namespace Core { namespace Data
{

// TODO: DOC

class AttributesHolder : public TiInterface
{
  //============================================================================
  // Type Info

  INTERFACE_INFO(AttributesHolder, TiInterface, "Core.Data", "Core", "alusus.net");


  //============================================================================
  // Abstract Functions

  public: virtual TiObject* getAttribute(Char const *name) = 0;

}; // class

} } // namespace

#endif
