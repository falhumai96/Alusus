/**
 * @file Spp/LlvmCodeGen/llvm_code_gen.cpp
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#include "OSAL.hpp"
#include "spp.h"

namespace Spp::LlvmCodeGen
{

void llvmDiagnosticCallback(const llvm::DiagnosticInfo &di, void *context)
{
  std::string msg;
  llvm::raw_string_ostream stream(msg);
  llvm::DiagnosticPrinterRawOStream dpstream(stream);

  di.print(dpstream);
  stream.flush();

  // Get the output stream.
  AutoAPRPool pool;
  apr_file_t* cStdoutFile;
  apr_status_t rv = apr_file_open_stdout(&cStdoutFile, pool.getPool());
  if (rv != APR_SUCCESS) {
    throw EXCEPTION(GenericException, S("Error opening APR stdout."));
  }
  AutoAPRFile stdoutFile(cStdoutFile);
  APRFilebuf stdoutBuf(stdoutFile.getFile());
  std::ostream outStream(&stdoutBuf);

  outStream << msg << "\n";
}

}
