/**
 * @file Core/Data/Ast/Bracket.h
 * Contains the header of class Core::Data::Ast::Bracket.
 *
 * @copyright Copyright (C) 2016 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <http://alusus.net/alusus_license_1_0>.
 */
//==============================================================================

#ifndef CORE_DATA_AST_BRACKET_H
#define CORE_DATA_AST_BRACKET_H

namespace Core { namespace Data { namespace Ast
{

// TODO: DOC

class Bracket : public Node,
                public virtual MapContainer, public virtual MetadataHolder,
                public virtual Clonable, public virtual Printable
{
  //============================================================================
  // Type Info

  TYPE_INFO(Bracket, Node, "Core.Data.Ast", "Core", "alusus.net");
  IMPLEMENT_INTERFACES(Node, MapContainer, MetadataHolder, Clonable, Printable);


  //============================================================================
  // Member Variables

  private: BracketType type;
  private: TioSharedPtr operand;

  IMPLEMENT_MAP_CONTAINER((TiObject, operand));

  IMPLEMENT_AST_MAP_PRINTABLE(Bracket, << (this->type == BracketType::ROUND ? STR("()") : STR("[]")));


  //============================================================================
  // Constructors & Destructor

  public: Bracket()
  {
  }

  public: Bracket(Word pid, SourceLocation const &sl) :
    MetadataHolder(pid, sl)
  {
  }

  public: Bracket(Word pid, BracketType t, TioSharedPtr const &o) :
    MetadataHolder(pid), type(t), operand(o)
  {
    OWN_SHAREDPTR(this->operand);
  }

  public: Bracket(Word pid, SourceLocation const &sl, BracketType t, TioSharedPtr const &o) :
    MetadataHolder(pid, sl), type(t), operand(o)
  {
    OWN_SHAREDPTR(this->operand);
  }

  public: virtual ~Bracket()
  {
    DISOWN_SHAREDPTR(this->operand);
  }

  public: static SharedPtr<Bracket> create()
  {
    return std::make_shared<Bracket>();
  }

  public: static SharedPtr<Bracket> create(Word pid, SourceLocation const &sl)
  {
    return std::make_shared<Bracket>(pid, sl);
  }

  public: static SharedPtr<Bracket> create(Word pid, BracketType t, TioSharedPtr const &o)
  {
    return std::make_shared<Bracket>(pid, t, o);
  }

  public: static SharedPtr<Bracket> create(Word pid, SourceLocation const &sl, BracketType t, TioSharedPtr const &o)
  {
    return std::make_shared<Bracket>(pid, sl, t, o);
  }


  //============================================================================
  // Member Functions

  public: void setType(BracketType t)
  {
    this->type = t;
  }

  public: BracketType getType() const
  {
    return this->type;
  }

  public: void setOperand(TioSharedPtr const &o)
  {
    UPDATE_OWNED_SHAREDPTR(this->operand, o);
  }

  public: TioSharedPtr const& getOperand() const
  {
    return this->operand;
  }


  //============================================================================
  // MetadataHolder Overrides

  public: virtual TiObject* getAttribute(Char const *name)
  {
    if (SBSTR(name) == STR("type")) {
      return &this->type;
    }
    return MetadataHolder::getAttribute(name);
  }


  //============================================================================
  // Clonable Implementation

  public: virtual SharedPtr<TiObject> clone() const;

}; // class

} } } // namespace

#endif
