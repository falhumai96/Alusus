/**
 * @file Scg/Operators/CastToChar.h
 *
 * @copyright Copyright (C) 2015 Hicham OUALI ALAMI
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <http://alusus.net/alusus_license_1_0>.
 */
//==============================================================================

#ifndef __CastToChar_h__
#define __CastToChar_h__

// Scg header files
#include <Expression.h>
#include <Operators/CastingOperator.h>
#include <exceptions.h>
#include <typedefs.h>

// LLVM forward declarations
#include <llvm_fwd.h>
#include <Types/ValueTypeSpec.h>

namespace Scg
{
/**
 * Represents a binary operator.
 */
class CastToChar : public CastingOperator
{
private:
  //! Storing the LLVM cast inst so that it can be freed after code generation.
  llvm::CastInst *llvmCastInst = nullptr;

public:
  /**
   * Construct an integer-to-float casting operator.
   * @param[in] operand The operand of the operator.
   */
  CastToChar(Expression *operand)
  {
    children.push_back(operand);
  }

  /**
   * Get the expression representing the left-hand side of the binary operator.
   *
   * @return A pointer to the left-hand side expression.
   */
  const ExpressionArray::value_type GetOperand() const { return children[0]; }
  ExpressionArray::value_type GetOperand() { return children[0]; }

  //! @copydoc Expression::GetValueTypeSpec()
  virtual const ValueTypeSpec *GetValueTypeSpec() const override;

  //! @copydoc Expression::GenerateCode()
  virtual CodeGenerationStage GenerateCode();

  //! @copydoc Expression::PostGenerateCode()
  virtual CodeGenerationStage PostGenerateCode();

  //! @copydoc Expression::ToString()
  virtual std::string ToString();
};
}

#endif // __CastToChar_h__
