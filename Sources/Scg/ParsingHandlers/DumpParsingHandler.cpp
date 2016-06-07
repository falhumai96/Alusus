/**
 * @file Scg/ParsingHandlers/DumpParsingHandler.cpp
 *
 * @copyright Copyright (C) 2014 Rafid Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <http://alusus.net/alusus_license_1_0>.
 */
//==============================================================================

#include <core.h>   // Alusus core header files.
#include <llvm/IR/IRBuilder.h>
#include <scg.h>
#include <ParsingHandlers/DumpParsingHandler.h>

namespace Scg
{

using namespace Core;
using namespace Core::Data;


//==============================================================================
// Overloaded Abstract Functions

void DumpParsingHandler::onProdEnd(Processing::Parser *parser, Processing::ParserState *state)
{
  SharedPtr<TiObject> item = state->getData();

  static ReferenceSeeker seeker;
  static SharedPtr<Reference> nameReference = REF_PARSER->parseQualifier(
        STR("self~where(prodId=Subject.Subject1).{find prodId=Subject.Parameter, 0}"),
        ReferenceUsageCriteria::MULTI_DATA);

  // Find the name of the module to execute.
  auto name = tio_cast<Ast::Token>(seeker.tryGet(nameReference.get(), item.get()));
  TiObject *def = 0;

  if (name != 0) {
    def = this->rootManager->getDefinitionsRepository()->get(name->getText().c_str());
  }

  if (def != 0) {
    outStream << STR("------------------ Parsed Data Dump ------------------\n");
    dumpData(outStream, def, 0);
    outStream << STR("------------------------------------------------------\n");
  } else {
    // Create a build msg.
    Str message = "Couldn't find module: ";
    message += name->getText();
    Ast::MetadataHolder *metadata = item->getInterface<Ast::MetadataHolder>();

    if (metadata != 0) {
      state->addBuildMsg(std::make_shared<Processing::CustomBuildMsg>(message.c_str(), metadata->getSourceLocation()));
    } else {
      state->addBuildMsg(std::make_shared<Processing::CustomBuildMsg>(message.c_str(), name->getSourceLocation()));
    }
  }

  // Reset parsed data because we are done with the command.
  state->setData(SharedPtr<TiObject>(0));
}

} // namespace
