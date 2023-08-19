/**
 * @file Core/Main/main.cpp
 * Contains the global implementations of Main namespace's declarations.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#include "core.h"
#include <stdio.h>  /* defines FILENAME_MAX */
#if __APPLE__
  #include <mach-o/dyld.h>
#endif
#if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && !defined(__CYGWIN__)
  #include <direct.h>
  #include <windows.h>
#else
  #include <unistd.h>
  #include <limits.h>
#endif

namespace Core::Main
{

using namespace Data;

#if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && !defined(__CYGWIN__)
  #define _getWorkingDirectory _getcwd
#else
  #define _getWorkingDirectory getcwd
#endif


//==============================================================================
// Global Functions

Srl::String getWorkingDirectory()
{
  thread_local static std::array<Char,FILENAME_MAX> currentPath;

  if (!_getWorkingDirectory(currentPath.data(), currentPath.size()))
  {
    throw EXCEPTION(GenericException, S("Couldn't obtain the current working directory."));
  }

  Srl::String path(currentPath.data());
  if (path(path.getLength() - 1) != C('/')) path += C('/');
  return path;
}

} // namespace
