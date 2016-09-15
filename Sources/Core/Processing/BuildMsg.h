/**
 * @file Core/Processing/BuildMsg.h
 * Contains the header of class Core::Processing::BuildMsg.
 *
 * @copyright Copyright (C) 2015 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <http://alusus.net/alusus_license_1_0>.
 */
//==============================================================================

#ifndef CORE_PROCESSING_BUILDMSG_H
#define CORE_PROCESSING_BUILDMSG_H

namespace Core { namespace Processing
{

/**
 * @brief The base of all build messages.
 * @ingroup processing
 *
 * Build messages are messages raised during the build process containing an
 * error, a warning, or simply a notification of something. All build msg
 * classes must derive from this class. This class provides a set of abstract
 * functions for obtaining the details of the msg including code, description,
 * severity, and source code location.
 */
class BuildMsg : public TiObject
{
  //============================================================================
  // Type Info

  TYPE_INFO(BuildMsg, TiObject, "Core.Processing", "Core", "alusus.net");


  //============================================================================
  // Member Variables

  /// The source location at which the message was generated.
  private: Data::SourceLocation sourceLocation;

  /**
   * @brief A temporary buffer to hold the generated message description.
   * @sa getDescription()
   */
  private: static Str tempBuf;


  //============================================================================
  // Constructor / Destructor

  public: BuildMsg()
  {
  }

  public: BuildMsg(Data::SourceLocation const &l) : sourceLocation(l)
  {
  }

  public: virtual ~BuildMsg()
  {
  }


  //============================================================================
  // Member Functions

  /**
   * @brief Get the message code.
   *
   * The message code is any code that can uniquely identify the message.
   * Ideally this consists of a prefix character (or group of characters)
   * identifying the unit that generated the message (for example L for Lexer
   * and P for Parser) followed by a number identifying the message
   * within that unit. For example, a parsing error code can have a value of
   * P23.
   */
  public: virtual Str const& getCode() const = 0;

  /**
   * @brief Get the severity of the message.
   *
   * The severity specifies how safe it is to ignore the message. The lower
   * the value, the less safe it is, with the value of 0 meaning it's a
   * blocker error. The following are the possible values:
   *
   * 0: This is a blocker error. Compilation can't continue, or results are
   *    unpredictable if compilation continues.<br>
   * 1: This is an error, but compilation can continue.<br>
   * 2: This is a critical warning, user should pay attention to it.<br>
   * 3: This is a warning that the user can ignore.<br>
   * 4: This is just a notification, no user action is required.
   * The default implementation returns 0.
   */
  public: virtual Int getSeverity() const
  {
    return 0;
  }

  /**
   * @brief Build a human readable description of the message.
   *
   * The description does not include the source code location associated with
   * the message (filename, line number, etc).
   *
   * @param str A reference to a string that will receive the text.
   */
  public: virtual void buildDescription(Str &str) const = 0;

  /// Get a human readable description of the message.
  public: Str const& getDescription() const;

  /// Set the source location at which the message was generated.
  public: void setSourceLocation(Data::SourceLocation const &l)
  {
    this->sourceLocation = l;
  }

  /// Get the source location at which the message was generated.
  public: Data::SourceLocation const& getSourceLocation() const
  {
    return this->sourceLocation;
  }

}; // class

//==============================================================================
// Macros

#define DEFINE_BUILD_MSG(name, typeNamespace, moduleName, url, code, severity, msg) \
  class name : public Core::Processing::BuildMsg \
  { \
    TYPE_INFO(name, Core::Processing::BuildMsg, typeNamespace, moduleName, url); \
    public: name() {} \
    public: name(Data::SourceLocation const &sl) : Processing::BuildMsg(sl) \
    { \
    } \
    public: virtual Str const& getCode() const \
    { \
      static Str _code(code); \
      return _code; \
    } \
    public: virtual Int getSeverity() const \
    { \
      return severity; \
    } \
    public: virtual void buildDescription(Str &str) const \
    { \
      str = msg; \
    } \
  }

} } // namespace

#endif
