/**
 * @file Core/Main/RootManager.cpp
 * Contains the implementation of class Core::Main::RootManager.
 *
 * @copyright Copyright (C) 2022 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
 //==============================================================================

#include "OSAL.hpp"
#include "core.h"
#include <cstddef>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>

namespace Core::Main
{

static std::vector<Char const *> sourceExtensions = {
    S(".alusus"), S(".source"), S(".الأسس"), S(".أسس"), S(".مصدر")};

//==============================================================================
// Constructor

Str RootManager::getCurrentWorkingDirectory(void *rv)
{
  AutoAPRPool pool;
  Char* cwd;

  // Get the current working directory
  apr_status_t tmpRV = apr_filepath_get(&cwd, APR_FILEPATH_NATIVE, pool.getPool());
  if (rv) {
    *((apr_status_t*)rv) = tmpRV;
  }

  return cwd;
}

Str RootManager::getParentDirectory(Char const* path)
{
  if (!path) {
    return Str();
  }

  size_t dirnameLength;
  cwk_path_get_dirname(path, &dirnameLength);
  if (dirnameLength == 0) {
    return Str();
  }
  return Str(path, dirnameLength);
}

void RootManager::computeCoreBinPath(const Char* argvBinPath, const Char* cwd, Str& resultPath)
{
  // TODO: Properly report OS/APR errors.

  if (!argvBinPath) {
    throw EXCEPTION(GenericException, S("Expected a program path in argvBinPath."));
  }

  // Follow the symlink of argvBinPath
  int error;
  std::string resolvedPath = followSymlink(argvBinPath, &error);
  if (error) {
    throw EXCEPTION(GenericException, S("Error following symlink."));
  }

  // Use the current working directory as the base path to get the abs path
  size_t neededLength = cwk_path_get_absolute(cwd, resolvedPath.c_str(), nullptr, 0) + 1;
  std::vector<Char> absolutePathBuffer(neededLength, 0);
  cwk_path_get_absolute(cwd, resolvedPath.c_str(), (Char*)absolutePathBuffer.data(), absolutePathBuffer.size());

  // Get the directory name
  Str dirname = getParentDirectory(absolutePathBuffer.data());
  if (dirname.getLength() == 0) {
    throw EXCEPTION(GenericException, S("Expected a parent directory."));
  }

  // Set resultPath to the normalized directory path
  resultPath = normalizePath(dirname.getBuf());
  if (resultPath.getLength() == 0) {
    throw EXCEPTION(GenericException, S("Expected a normalized path."));
  }
}

RootManager::RootManager(Int count, Char const* const* args) : processArgCount(count), processArgs(args), libraryManager(this), processedFiles(true) {
  this->rootScope = Data::Ast::Scope::create();
  this->rootScope->setProdId(ID_GENERATOR->getId("Root"));
  this->exprRootScope = Data::Ast::Scope::create();
  this->exprRootScope->setProdId(ID_GENERATOR->getId("Root"));

  this->rootScopeHandler.setSeeker(&this->seeker);
  this->rootScopeHandler.setRootScope(this->rootScope);

  this->noticeSignal.relay(this->inerNoticeSignal);
  this->noticeSignal.connect(this->noticeSlot);

  Data::Grammar::StandardFactory factory;
  factory.createGrammar(this->rootScope.get(), this, false);
  factory.createGrammar(this->exprRootScope.get(), this, true);

  this->interactive = false;

  // Compute the coreBinPath.
  apr_status_t rv;
  Str cwd = getCurrentWorkingDirectory((void*)&rv);
  if (rv != APR_SUCCESS) {
    throw EXCEPTION(GenericException, S("Error getting the current working directory."));
  }
  this->computeCoreBinPath(args[0], cwd, this->coreBinPath);

  // Initialize current paths (order of lookup is in reverse of the order of push).

  // Binary directory.
  this->pushSearchPath(this->coreBinPath);

  // Library directory.
  // TODO: [Design] better have Alusus libraries under `<Lib dir name>/<Alusus version>` as `<Lib dir name>`
  //       is supposed to be for system libraries only.
  Str libraryDirectory = getParentDirectory(this->coreBinPath.getBuf());
  libraryDirectory = joinPaths(libraryDirectory.getBuf(), ALUSUS_LIB_DIR_NAME);
  this->pushSearchPath(normalizePath(libraryDirectory));

  // Paths from "ALUSUS_LIBS" environment variable.
  std::vector<std::string> alususPaths;
  AutoAPRPool pool;
  Char* alususLibsCStr = nullptr;
  rv = apr_env_get(&alususLibsCStr, "ALUSUS_LIBS", pool.getPool());
  if (rv == APR_SUCCESS) {
    auto alususLibs = splitPath(alususLibsCStr);
    for (const auto& alususLib : alususLibs) {
      this->pushSearchPath(alususLib.c_str());
    }
  }

  // Current working directory.
  this->pushSearchPath(cwd);
}

//==============================================================================
// Member Functions

void RootManager::flushNotices()
{
  Int count = this->getNoticeStore()->getCount();
  if (count == 0) return;

  // Now emit the messages.
  for (Int i = 0; i < count; ++i) {
    this->inerNoticeSignal.emit(this->getNoticeStore()->get(i));
  }
  this->getNoticeStore()->flush(count);
}


SharedPtr<TiObject> RootManager::parseExpression(Char const *str)
{
  Processing::Engine engine(this->exprRootScope);
  auto result = engine.processString(str, str);

  if (result == 0) {
    throw EXCEPTION(
      InvalidArgumentException,
      S("str"),
      S("Parsing did not result in a valid expression"),
      str
    );
  }

  return result;
}

SharedPtr<TiObject> RootManager::processString(Char const *str,
                                               Char const *name) {
  Processing::Engine engine(this->rootScope);
  this->noticeSignal.relay(engine.noticeSignal);
  return engine.processString(str, name);
}

SharedPtr<TiObject> RootManager::processFile(Char const *filename,
                                             Bool allowReprocess) {
  // Find the absolute path of the requested source file (only source files are accepted initially).
  [[maybe_unused]] Str resultFileName;
  if (this->findSourceFile(filename, resultFileName)) {
    return this->_processSourceFile(resultFileName, allowReprocess);
  } else {
    throw EXCEPTION(FileException, filename, C('r'));
  }
}

SharedPtr<TiObject> RootManager::_processSourceFile(Char const *fullPath,
                                                    Bool allowReprocess) {
  // Do not reprocess if already processed.
  if (!allowReprocess) {
    if (this->processedFiles.findIndex(fullPath) != -1)
      return TioSharedPtr::null;
  }
  this->processedFiles.add(fullPath, TioSharedPtr::null);

  // Extract the directory part and add it to the current paths.
  Str parentDir = getParentDirectory(fullPath);
  if (parentDir.getLength() > 0) {
    this->pushSearchPath(parentDir);
  }

  // Process the file.
  Processing::Engine engine(this->rootScope);
  this->noticeSignal.relay(engine.noticeSignal);
  auto result = engine.processFile(fullPath);

  // Remove the added path, if any.
  if (parentDir.getLength() > 0) {
    this->popSearchPath(parentDir);
  }

  return result;
}

SharedPtr<TiObject> RootManager::processStream(Processing::CharInStreaming* is, Char const* streamName) {
  Processing::Engine engine(this->rootScope);
  this->noticeSignal.relay(engine.noticeSignal);
  return engine.processStream(is, streamName);
}

Bool RootManager::tryImportFile(Char const *filename, Str &errorDetails) {
  // Lookup the presumed Alusus file in the search paths.
  Str resultFilename;
  if (this->findSourceFile(filename, resultFilename)) {
    // Load a source file.
    try {
      LOG(LogLevel::PARSER_MAJOR, S("Importing source file: ") << filename);

      this->_processSourceFile(resultFilename.getBuf());
      return true;
    }
    catch (...) {
      return false;
    }
  }

  // Assume it is a shared library.
  else {
    // Load a library.
    LOG(LogLevel::PARSER_MAJOR, S("Importing library: ") << filename);

    // Try the name directly.
    PtrWord id =
      this->getLibraryManager()->load(filename, errorDetails);
    if (id != 0) {
      LOG(LogLevel::PARSER_MAJOR, S("Found library: ") << filename);
      return true;
    }

    std::string newPathName = filename;

#ifdef ALUSUS_USE_LOGS
    // Try adding the Alusus debug name.
    newPathName += ".dbg";
    id = this->getLibraryManager()->load(newPathName.c_str(), errorDetails);
    if (id != 0) {
      LOG(LogLevel::PARSER_MAJOR, S("Found library: ") << newPathName.c_str());
      return true;
    }

    // Try with system library extension.
    newPathName += getShlibExt();
    id = this->getLibraryManager()->load(newPathName.c_str(), errorDetails);
    if (id != 0) {
      LOG(LogLevel::PARSER_MAJOR, S("Found library: ") << newPathName.c_str());
      return true;
    }

    // Try prefixing the name with "lib".
    newPathName = std::string("lib") + newPathName;
    id = this->getLibraryManager()->load(newPathName.c_str(), errorDetails);
    if (id != 0) {
      LOG(LogLevel::PARSER_MAJOR, S("Found library: ") << newPathName.c_str());
      return true;
    }

    // Reset to try without debug names.
    newPathName = filename;
#endif

    // Try with system library extension.
    newPathName += getShlibExt();
    id = this->getLibraryManager()->load(newPathName.c_str(), errorDetails);
    if (id != 0) {
      LOG(LogLevel::PARSER_MAJOR, S("Found library: ") << newPathName.c_str());
      return true;
    }

    // Try prefixing the name with "lib".
    newPathName = std::string("lib") + newPathName;
    id = this->getLibraryManager()->load(newPathName.c_str(), errorDetails);
    if (id != 0) {
      LOG(LogLevel::PARSER_MAJOR, S("Found library: ") << newPathName.c_str());
      return true;
    }

    // Return false if we failed all patterns.
    return false;
  }
}

Bool RootManager::isShlib(Char const* filename)
{
  if (!filename) {
    return false;
  }
  const Char* extension;
  size_t length;

  // Attempt to get the extension from the filename
  if (cwk_path_get_extension(filename, &extension, &length)) {
    // Convert the extension to lowercase for comparison
    std::string fileExtension(extension, length);
    std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);

    // Get the system shared library extension and convert it to lowercase
    std::string shlibExt(getShlibExt());
    std::transform(shlibExt.begin(), shlibExt.end(), shlibExt.begin(), ::tolower);

    // Compare the extensions
    return fileExtension == shlibExt;
  }

  return false;
}

Bool RootManager::isAlususFile(Char const* filename)
{
  if (!filename) {
    return false;
  }

  // Get the extension from the filename
  const Char* extension;
  size_t length;
  if (cwk_path_get_extension(filename, &extension, &length)) {
    // Convert the extension to lowercase for comparison
    std::string fileExtension(extension, length);
    std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);

    // Loop through all source extensions and compare
    for (const auto& ext : sourceExtensions) {
      if (fileExtension == ext) {
        return true;
      }
    }
  }

  return false;
}

Str RootManager::joinPaths(Char const* path1, Char const* path2)
{
  size_t neededLength = cwk_path_join(path1, path2, nullptr, 0) + 1;
  std::vector<Char> joinedPathBuffer(neededLength, 0);
  cwk_path_join(path1, path2, (Char*)joinedPathBuffer.data(), joinedPathBuffer.size());
  Str toReturn = joinedPathBuffer.data();
  return toReturn;
}

Str RootManager::normalizePath(Char const* path)
{
  if (!path) {
    return Str();
  }

  size_t neededLength = cwk_path_normalize(path, nullptr, 0) + 1;
  std::vector<Char> normalizedPathBuffer(neededLength, 0);
  cwk_path_normalize(path, (Char*)normalizedPathBuffer.data(), normalizedPathBuffer.size());
  Str toReturn = normalizedPathBuffer.data();
  return toReturn;
}

void RootManager::pushSearchPath(Str const& path) {
  if (path.getLength() == 0) {
    throw EXCEPTION(InvalidArgumentException, S("path"),
      S("Argument is null or empty string."));
  }

  // Only accept absolute paths.
  if (!cwk_path_is_absolute(path.getBuf())) {
    throw EXCEPTION(InvalidArgumentException, S("path"),
      S("Path must be an absolute path."));
  }

  // Only add the path if it doesn't already exist.
  // We will only check the top of the stack. If this path exists deeper in the
  // stack then we'll add it again to make it available at the top of the stack.
  // We won't remove the other copy because we don't expect any penalties from
  // keeping it there.
  if (!this->searchPaths.empty() &&
    strcmp(this->searchPaths.back().getBuf(), path.getBuf()) == 0) {
    ++this->searchPathCounts.back();
  } else {
    this->searchPaths.push_back(path);
    this->searchPathCounts.push_back(1);
  }
}

void RootManager::popSearchPath(Str const& path) {
  if (path.getLength() == 0) {
    throw EXCEPTION(InvalidArgumentException, S("path"),
                    S("Argument is null or empty string."));
  }

  // Only accept absolute paths.
  if (!cwk_path_is_absolute(path.getBuf())) {
    throw EXCEPTION(InvalidArgumentException, S("path"),
                    S("Path must be an absolute path."));
  }

  for (size_t i = this->searchPaths.size() - 1; i >= 0; --i) {
    if (strcmp(this->searchPaths[i].getBuf(), path.getBuf()) == 0) {
      // Decrement the count and only remove it when it reaches 0.
      --this->searchPathCounts[i];
      if (this->searchPathCounts[i] == 0) {
        this->searchPaths.erase(this->searchPaths.begin() + i);
        this->searchPathCounts.erase(this->searchPathCounts.begin() + i);
      }
      return;
    }
  }
  throw EXCEPTION(InvalidArgumentException, S("path"),
                  S("Path was not found in the stack."), path.getBuf());
}

Bool RootManager::findSourceFile(Char const* filename, Str& resultFilename) {
  // TODO: Do we need explicit exception for this case?
  // if (!filename) {
  //   throw EXCEPTION(InvalidArgumentException, S("filename"),
  //     S("Argument is null or empty string."));
  // }

  if (!filename) {
    return false;
  }

  // Is the filename an absolute path already?
  if (cwk_path_is_absolute(filename)) {
    return trySourceFileName(filename, resultFilename);
  }

  // Try all current search paths.
  else {
    for (auto it = this->searchPaths.rbegin(); it != this->searchPaths.rend(); ++it) {
      Str joinedPaths = joinPaths((*it).getBuf(), filename);
      if (this->trySourceFileName(joinedPaths.getBuf(), resultFilename)) {
        return true;
      }
    }
  }

  return false;
}

Bool RootManager::trySourceFileName(Char const* filename, Str& resultFilename) {
  if (!filename) {
    return false;
  }

  AutoAPRPool pool;

  // If the file exists without extension addition, check if it is an Alusus file, and if so,
  // set it as resultFilename and return true.
  apr_finfo_t finfo;
  apr_status_t rv = apr_stat(&finfo, filename, APR_FINFO_TYPE, pool.getPool());
  if (isAlususFile(filename) && rv == APR_SUCCESS && finfo.filetype == APR_REG) {
    resultFilename = normalizePath(filename);
    return true;
  }

  // Try source extensions.
  for (auto& extension : sourceExtensions) {
    std::string path(filename);
    path += extension;
    rv = apr_stat(&finfo, path.c_str(), APR_FINFO_TYPE, pool.getPool());
    if (rv == APR_SUCCESS && finfo.filetype == APR_REG) {
      resultFilename = normalizePath(path.c_str());
      return true;
    }
  }

  return false;
}

} // namespace
