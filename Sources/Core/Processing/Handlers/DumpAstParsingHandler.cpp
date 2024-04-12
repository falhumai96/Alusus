/**
 * @file Core/Processing/Handlers/DumpAstParsingHandler.cpp
 * Contains the implementation of Core::Processing::Handlers::DumpAstParsingHandler.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#include "OSAL.hpp"
#include "core.h"

namespace Core::Processing::Handlers
{

using namespace Core::Data;

//==============================================================================
// Overloaded Abstract Functions

void DumpAstParsingHandler::onProdEnd(Parser *parser, ParserState *state)
{
  // Get the output stream.
  AutoAPRPool pool;
  apr_file_t* cStdoutFile;
  apr_status_t rv = apr_file_open_stdout(&cStdoutFile, pool.getPool());
  if (rv != APR_SUCCESS) {
    throw EXCEPTION(GenericException, S("Error opening APR stdout."));
  }
  AutoAPRFile stdoutFile(cStdoutFile);
  APRFilebuf stdoutBuf(stdoutFile.getFile());
  std::ostream stdoutStream(&stdoutBuf);

  using SeekVerb = Data::Seeker::Verb;

  auto data = state->getData().ti_cast_get<Containing<TiObject>>()->getElement(1);
  ASSERT(data != 0);
  auto metadata = ti_cast<Core::Data::Ast::MetaHaving>(data);
  ASSERT(metadata != 0);

  try {
    Bool found = false;
    auto node = ti_cast<Core::Data::Node>(data);
    if (node == 0) {
      state->addNotice(newSrdObj<Notices::InvalidDumpArgNotice>(metadata->findSourceLocation()));
    } else {
      node->setOwner(parser->getRootScope().get());
      this->rootManager->getSeeker()->foreach(data, state->getDataStack(),
        [=, &stdoutStream, &found](TiInt action, TiObject *obj)->SeekVerb
        {
          if (action == Core::Data::Seeker::Action::TARGET_MATCH && obj != 0) {
            stdoutStream << "------------------ Parsed Data Dump ------------------" << APR_EOL_STR;
            dumpData(stdoutStream, obj, 0);
            stdoutStream << APR_EOL_STR << "------------------------------------------------------" << APR_EOL_STR;
            found = true;
          }
          return SeekVerb::MOVE;
        }, 0
      );
      if (!found) {
        state->addNotice(newSrdObj<Notices::InvalidDumpArgNotice>(metadata->findSourceLocation()));
      }
    }
  } catch (InvalidArgumentException) {
    state->addNotice(newSrdObj<Notices::InvalidDumpArgNotice>(metadata->findSourceLocation()));
  }

  state->setData(SharedPtr<TiObject>(0));
}

} // namespace
