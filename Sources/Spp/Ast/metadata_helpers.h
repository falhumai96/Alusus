/**
 * @file Spp/Ast/metadata_helpers.h
 *
 * @copyright Copyright (C) 2024 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#ifndef SPP_AST_METADATAHELPERS_H
#define SPP_AST_METADATAHELPERS_H

namespace Spp::Ast
{

//==============================================================================
// Global Constants

constexpr Char const* META_EXTRA_AST_TYPE = S("astType");


//==============================================================================
// Global Functions

// tryGetAstType

template <class OT,
          typename std::enable_if<std::is_base_of<Core::Data::Ast::MetaHaving, OT>::value, int>::type = 0>
inline Type* tryGetAstType(OT *object)
{
  auto box = object->getExtra(META_EXTRA_AST_TYPE).template ti_cast_get<TiBox<Type*>>();
  if (box == 0) return 0;
  else return box->get();
}

template <class OT,
          typename std::enable_if<!std::is_base_of<Core::Data::Ast::MetaHaving, OT>::value, int>::type = 0>
inline Type* tryGetAstType(OT *object)
{
  auto metadata = ti_cast<Core::Data::Ast::MetaHaving>(object);
  if (metadata == 0) return 0;
  auto box = metadata->getExtra(META_EXTRA_AST_TYPE).template ti_cast_get<TiBox<Type*>>();
  if (box == 0) return 0;
  else return box->get();
}

// getAstType

template <class OT>
inline Type* getAstType(OT *object)
{
  auto result = tryGetAstType(object);
  if (result == 0) {
    throw EXCEPTION(GenericException, S("Object is missing the AST type."));
  }
  return result;
}

// setAstType

template <class OT,
          typename std::enable_if<std::is_base_of<Core::Data::Ast::MetaHaving, OT>::value, int>::type = 0>
inline void setAstType(OT *object, SharedPtr<Type> const &type)
{
  object->setExtra(META_EXTRA_AST_TYPE, TiBox<Type*>::create(type.get()));
}

template <class OT,
          typename std::enable_if<!std::is_base_of<Core::Data::Ast::MetaHaving, OT>::value, int>::type = 0>
inline void setAstType(OT *object, SharedPtr<Type> const &type)
{
  auto metadata = ti_cast<Core::Data::Ast::MetaHaving>(object);
  if (metadata == 0) {
    throw EXCEPTION(InvalidArgumentException, S("object"), S("Object does not implement the MetaHaving interface."));
  }
  metadata->setExtra(META_EXTRA_AST_TYPE, TiBox<Type*>::create(type.get()));
}

template <class OT,
          typename std::enable_if<std::is_base_of<Core::Data::Ast::MetaHaving, OT>::value, int>::type = 0>
inline void setAstType(OT *object, Type *type)
{
  object->setExtra(META_EXTRA_AST_TYPE, TiBox<Type*>::create(type));
}

template <class OT,
          typename std::enable_if<!std::is_base_of<Core::Data::Ast::MetaHaving, OT>::value, int>::type = 0>
inline void setAstType(OT *object, Type *type)
{
  auto metadata = ti_cast<Core::Data::Ast::MetaHaving>(object);
  if (metadata == 0) {
    throw EXCEPTION(InvalidArgumentException, S("object"), S("Object does not implement the MetaHaving interface."));
  }
  metadata->setExtra(META_EXTRA_AST_TYPE, TiBox<Type*>::create(type));
}

} // namespace

#endif
