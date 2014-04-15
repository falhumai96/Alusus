/**
 * @file Core/Standard/ParsedItem.h
 * Contains the header of class Core::Standard::ParsedItem.
 *
 * @copyright Copyright (C) 2014 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <http://alusus.net/alusus_license_1_0>.
 */
//==============================================================================

#ifndef STANDARD_PARSED_ITEM_H
#define STANDARD_PARSED_ITEM_H

namespace Core { namespace Standard
{

/**
 * @brief The base of all parsed data generated by GenericParsingHandler.
 * @ingroup standard
 *
 * Contains generic information that are shared among the different item types.
 */
class ParsedItem : public IdentifiableObject
{
  //============================================================================
  // Type Info

  TYPE_INFO(ParsedItem, IdentifiableObject, "Core.Standard", "Core", "alusus.net");


  //============================================================================
  // Member Variables

  /**
   * @brief The production id this item refers to.
   *
   * This value refers to the id of the production definition this item
   * represents. If this value is UNKNOWN_ID, then the item is an
   * inner term, not a production root.
   */
  private: Word prodId;

  /**
   * @brief The number of the line at which the token appeared in the source.
   *
   * Although this refers to the token, it actually applies to all item types
   * since an error reporter needs to report a source code location no matter
   * at which item the error was detected. In the case of non-token items,
   * this refers to the location of the first token detected inside that term.
   */
  private: Int line;

  /**
   * @brief The number of the column at which the token appeared in the source.
   *
   * Although this refers to the token, it actually applies to all item types
   * since an error reporter needs to report a source code location no matter
   * at which item the error was detected. In the case of non-token items,
   * this refers to the location of the first token detected inside that term.
   */
  private: Int column;


  //============================================================================
  // Constructor / Destructor

  public: ParsedItem(Word pid=UNKNOWN_ID, Int l=-1, Int c=-1) : prodId(pid), line(l), column(c)
  {
  }

  public: virtual ~ParsedItem()
  {
  }


  //============================================================================
  // Member Functions

  /**
   * @brief Set the production id this list refers to.
   *
   * This value refers to the id of the production definition this element
   * represents. If this value is 0, then the element is an inner term, not
   * a production root.
   */
  public: void setProdId(Word id)
  {
    this->prodId = id;
  }

  /**
   * @brief Get the production id this list refers to.
   *
   * This value refers to the id of the production definition this element
   * represents. If this value is 0, then the element is an inner term, not
   * a production root.
   */
  public: Word getProdId() const
  {
    return this->prodId;
  }

  /**
   * @brief Set the token's line number.
   *
   * Set the line number at which the token appeared in the source code. This
   * value refers to the line number of the first character in the token.
   * Although this refers to the token, it actually applies to all item types
   * since an error reporter needs to report a source code location no matter
   * at which item the error was detected. In the case of non-token items,
   * this refers to the location of the first token detected inside that term.
   *
   * @param l The value to set as the token's line number.
   */
  public: void setLine(Int l)
  {
    this->line = l;
  }

  /**
   * @brief Get the token's line number.
   *
   * Get the line number at which the token appeared in the source code. This
   * value refers to the line number of the first character in the token.
   * Although this refers to the token, it actually applies to all item types
   * since an error reporter needs to report a source code location no matter
   * at which item the error was detected. In the case of non-token items,
   * this refers to the location of the first token detected inside that term.
   *
   * @return The line number of the first character in the token.
   */
  public: virtual Int getLine() const
  {
    return this->line;
  }

  /**
   * @brief Set the token's column number.
   *
   * Set the column number at which the token appeared in the source code.
   * This value refers to the column number of the first character in the token.
   * Although this refers to the token, it actually applies to all item types
   * since an error reporter needs to report a source code location no matter
   * at which item the error was detected. In the case of non-token items,
   * this refers to the location of the first token detected inside that term.
   *
   * @param c The value to set as the token's column number.
   */
  public: void setColumn(Int c)
  {
    this->column = c;
  }

  /**
   * @brief Get the token's column number.
   *
   * Get the column number at which the token appeared in the source code.
   * This value refers to the column number of the first character in the token.
   * Although this refers to the token, it actually applies to all item types
   * since an error reporter needs to report a source code location no matter
   * at which item the error was detected. In the case of non-token items,
   * this refers to the location of the first token detected inside that term.
   *
   * @return The column number of the first character in the token.
   */
  public: virtual Int getColumn() const
  {
    return this->column;
  }

  /**
   * @brief Copy this object's data to a new object.
   *
   * This function is needed by the duplicate override function of the derived
   * class.
   */
  protected: void copyTo(ParsedItem *item) const
  {
    item->setProdId(this->getProdId());
    item->setLine(this->getLine());
    item->setColumn(this->getColumn());
  }

}; // class

} } // namespace

#endif
