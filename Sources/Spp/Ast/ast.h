/**
 * @file Spp/Ast/ast.h
 * Contains the definitions and include statements of all types in the Ast
 * namespace.
 *
 * @copyright Copyright (C) 2024 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#ifndef SPP_AST_AST_H
#define SPP_AST_AST_H

namespace Spp::Ast
{

/**
 * @defgroup spp_ast Ast
 * @ingroup spp
 * @brief Classes related to the SPP's AST.
 */

//==============================================================================
// Forward Class Declarations

class NodePathResolver;
class Helper;
class Type;


//==============================================================================
// Types

/// @ingroup spp_ast
s_enum(TypeMatchOptions,
  NONE = 0,
  SKIP_DEREF = 1
);

/// @ingroup spp_ast
class TypeMatchStatus : public TiObject
{
  TYPE_INFO(TypeMatchStatus, TiObject, "Spp.Ast", "Spp", "alusus.org");
  public:
  enum Status {
    NONE = 0,
    AGGREGATION = 1,
    EXPLICIT_CAST = 2,
    CUSTOM_CASTER = 3,
    IMPLICIT_CAST = 4,
    PROMOTION = 5,
    REF_AGGREGATION = 6,
    EXACT = 7
  };
  Int value;
  Int derefs;
  Int rank;
  TypeMatchStatus() : value(0), derefs(0) {
    this->rank = 0;
  }
  TypeMatchStatus(Status v, Int d = 0) : value(v), derefs(d) {
    this->rank = 100000000 * this->value;
  }
  TypeMatchStatus(TypeMatchStatus const &prevValue, Status v, Int d = 0) : value(v), derefs(d) {
    this->rank = 100000000 * this->value + prevValue.rank / 10;
  }
  TypeMatchStatus const& operator=(Status v) {
    this->value = v;
    this->rank = 100000000 * value;
    return *this;
  }
  TypeMatchStatus const& operator=(TypeMatchStatus const &v) {
    this->value = v.value;
    this->derefs = v.derefs;
    this->rank = v.rank;
    return *this;
  }
  bool operator ==(TypeMatchStatus const &v) const { return this->rank == v.rank && this->derefs == v.derefs; }
  bool operator !=(TypeMatchStatus const &v) const { return this->rank != v.rank || this->derefs != v.derefs; }
  bool operator ==(Status v) const { return this->value == v; }
  bool operator !=(Status v) const { return this->value != v; }
  bool operator >(TypeMatchStatus const &v) const { return this->rank > v.rank; }
  bool operator >=(TypeMatchStatus const &v) const { return this->rank >= v.rank; }
  bool operator >(Status v) const { return this->value > v; }
  bool operator >=(Status v) const { return this->value >= v; }
  bool operator <(TypeMatchStatus const &v) const { return this->rank < v.rank; }
  bool operator <=(TypeMatchStatus const &v) const { return this->rank <= v.rank; }
  bool operator <(Status v) const { return this->value < v; }
  bool operator <=(Status v) const { return this->value <= v; }
};

/// @ingroup spp_ast
ti_s_enum(DefinitionDomain,
  TiInt, "Spp.Ast", "Spp", "alusus.org",
  FUNCTION = 0, OBJECT = 1, GLOBAL = 2
);

/// @ingroup spp_ast
s_enum(TypeInitMethod,
  NONE = 0,
  AUTO = 1,
  USER = 2,
  BOTH = 3
);


//==============================================================================
// Forward Class Declarations

class Function;


//==============================================================================
// Functions

Char const* findOperationModifier(Core::Data::Ast::Definition const *def);

Bool isInjection(Core::Data::Ast::Definition *def);

Function* getDummyBuiltInOpFunction();

} // namespace


//==============================================================================
// Includes

#include "callee_lookup_defs.h"

//// AST Classes
#include "Block.h"
// Types
#include "Type.h"
#include "DataType.h"
#include "VoidType.h"
#include "IntegerType.h"
#include "FloatType.h"
#include "PointerType.h"
#include "ReferenceType.h"
#include "ArrayType.h"
#include "UserType.h"
#include "FunctionType.h"
#include "Macro.h"
// Containers & Definitions
#include "Module.h"
#include "TemplateVarDef.h"
#include "Template.h"
#include "Function.h"
#include "Variable.h"
// Control Statements
#include "ForStatement.h"
#include "IfStatement.h"
#include "WhileStatement.h"
#include "ContinueStatement.h"
#include "BreakStatement.h"
#include "ReturnStatement.h"
#include "PreprocessStatement.h"
#include "PreGenTransformStatement.h"
// Operators
#include "PointerOp.h"
#include "ContentOp.h"
#include "DerefOp.h"
#include "NoDerefOp.h"
#include "CastOp.h"
#include "SizeOp.h"
#include "TypeOp.h"
#include "AstRefOp.h"
#include "InitOp.h"
#include "TerminateOp.h"
#include "NextArgOp.h"
#include "UseInOp.h"
// Misc
#include "ArgPack.h"
#include "ThisTypeRef.h"
#include "AstLiteralCommand.h"

// Helpers
#include "Helper.h"
#include "CalleeTracer.h"
#include "NodePathResolver.h"
#include "metadata_helpers.h"

#endif
