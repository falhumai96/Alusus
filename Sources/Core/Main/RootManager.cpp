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

#include "AlususDefs.h"
#include "AlususOSAL.hpp"
#include "core.h"
#include <cstddef>
#include <stdlib.h>
#include <sys/stat.h>
#include <vector>

namespace Core::Main
{

static std::vector<char const *> sourceExtensions = {
    S(".alusus"), S(".source"), S(".الأسس"), S(".أسس"), S(".مصدر")};

//==============================================================================
// Constructor

RootManager::RootManager() : libraryManager(this), processedFiles(true) {
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
  this->processArgCount = 0;
  this->processArgs = 0;

  this->coreBinPath = AlususOSAL::getModuleDirectory().c_str();
  auto coreBinPathOsPath = AlususOSAL::getModuleDirectory();

  // Initialize current paths.

  // Module directory.
  this->pushSearchPath(coreBinPathOsPath);

  // Package lib dirnames.
  for (auto &defaultLibPath : AlususOSAL::getAlususPackageLibDirNames()) {
    auto defaultLibFullPath = coreBinPathOsPath.parent_path() / defaultLibPath;
    this->pushSearchPath(defaultLibFullPath);
  }

  // Current working directory.
  this->pushSearchPath(AlususOSAL::getWorkingDirectory());

  // Paths from "ALUSUS_LIBS" environment variable.
  auto alususPaths =
      AlususOSAL::parsePathVariable(AlususOSAL::getenv(S("ALUSUS_LIBS")));
  for (auto &path : alususPaths) {
    this->pushSearchPath(path);
  }

  // TODO: Do we need to add the paths from LD_LIBRARY_PATH env variable?
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
  // Find the absolute path of the requested file.
  thread_local static AlususOSAL::Path resultFilename;
  AlususOSAL::Path inputFilename = (char *)filename;
  if (this->findFile(inputFilename, resultFilename)) {
    return this->_processFile(resultFilename, allowReprocess);
  } else {
    throw EXCEPTION(FileException, filename, C('r'));
  }
}

SharedPtr<TiObject> RootManager::_processFile(AlususOSAL::Path const &fullPath,
                                              Bool allowReprocess) {
  // Do not reprocess if already processed.
  if (!allowReprocess) {
    if (this->processedFiles.findIndex(fullPath.c_str()) != -1)
      return TioSharedPtr::null;
  }
  this->processedFiles.add(fullPath.c_str(), TioSharedPtr::null);

  // Extract the directory part and add it to the current paths.
  auto parentDir = fullPath.parent_path();
  if (!parentDir.empty()) {
    this->pushSearchPath(parentDir);
  }

  // Process the file.
  Processing::Engine engine(this->rootScope);
  this->noticeSignal.relay(engine.noticeSignal);
  auto result = engine.processFile(fullPath.c_str());

  // Remove the added path, if any.
  if (!parentDir.empty()) {
    this->popSearchPath(parentDir);
  }

  return result;
}

SharedPtr<TiObject> RootManager::processStream(Processing::CharInStreaming *is, Char const *streamName)
{
  Processing::Engine engine(this->rootScope);
  this->noticeSignal.relay(engine.noticeSignal);
  return engine.processStream(is, streamName);
}

Bool RootManager::tryImportFile(Char const *filename, Str &errorDetails) {
  // Lookup the file in the search paths.
  Bool loadSource = false;

  AlususOSAL::Path inputFilename = (char *)filename;
  thread_local static AlususOSAL::Path resultFilename;

  if (this->findFile(inputFilename, resultFilename)) {
    auto resultExtension = resultFilename.extension().string();
    for (auto &extension : sourceExtensions) {
      if (resultExtension == extension) {
        loadSource = true;
        break;
      }
    }
  }

  if (loadSource) {
    // Load a source file.
    try {
      LOG(LogLevel::PARSER_MAJOR, S("Importing source file: ") << filename);

      this->_processFile(resultFilename);
      return true;
    } catch (...) {
      return false;
    }
  } else {
    // Load a library.
    LOG(LogLevel::PARSER_MAJOR, S("Importing library: ") << filename);

    PtrWord id =
        this->getLibraryManager()->load(resultFilename.c_str(), errorDetails);

    return id != 0;
  }
}

void RootManager::pushSearchPath(AlususOSAL::Path const &path) {
  if (path.empty()) {
    throw EXCEPTION(InvalidArgumentException, S("path"),
                    S("Argument is null or empty string."));
  }

  // Only accept absolute paths.
  if (!path.is_absolute()) {
    throw EXCEPTION(InvalidArgumentException, S("path"),
                    S("Path must be an absolute path."));
  }

  // Only add the path if it doesn't already exist.
  // We will only check the top of the stack. If this path exists deeper in the
  // stack then we'll add it again to make it available at the top of the stack.
  // We won't remove the other copy because we don't expect any penalties from
  // keeping it there.
  if (!this->searchPaths.empty() && this->searchPaths.back() == path.c_str()) {
    ++this->searchPathCounts.back();
  } else {
    this->searchPaths.push_back(path.c_str());
    this->searchPathCounts.push_back(1);
  }
}

void RootManager::popSearchPath(AlususOSAL::Path const &path) {
  if (path.empty()) {
    throw EXCEPTION(InvalidArgumentException, S("path"),
                    S("Argument is null or empty string."));
  }

  // Only accept absolute paths.
  if (!path.is_absolute()) {
    throw EXCEPTION(InvalidArgumentException, S("path"),
                    S("Path must be an absolute path."));
  }

  for (size_t i = this->searchPaths.size() - 1; i >= 0; --i) {
    if (this->searchPaths[i] == path.c_str()) {
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
                  S("Path was not found in the stack."), path.c_str());
}

Bool RootManager::findFile(AlususOSAL::Path const &filename,
                           AlususOSAL::Path &resultFilename) {
  if (filename.empty()) {
    throw EXCEPTION(InvalidArgumentException, S("filename"),
                    S("Argument is null or empty string."));
  }

  // Is the filename an absolute path already?
  if (filename.is_absolute()) {
    if (this->tryFileName(filename, resultFilename)) {
      resultFilename = resultFilename.canonical();
      return true;
    }
  } else {
    // Try all current paths.
    for (size_t idx = this->searchPaths.size() - 1; idx >= 0; --idx) {
      AlususOSAL::Path searchFilename = (char *)this->searchPaths[idx].getBuf();
      searchFilename /= filename;
      if (this->tryFileName(searchFilename, resultFilename)) {
        resultFilename = resultFilename.canonical();
        return true;
      }
    }
  }

  return false;
}

Bool RootManager::tryFileName(AlususOSAL::Path const &filename,
                              AlususOSAL::Path &resultFilename) {
  if (filename.is_regular_file()) {
    resultFilename = filename;
    return true;
  }

  // Try source extensions.
  for (auto &extension : sourceExtensions) {
    auto newPath = AlususOSAL::Path(filename.string() + extension);
    if (newPath.is_regular_file()) {
      resultFilename = newPath;
      return true;
    }
  }

  // Try OS libraries.
  auto filenameDir = filename.parent_path();
  auto shlibNames = AlususOSAL::constructShlibNames(filename.filename());
  for (auto &shlibName : shlibNames) {
    auto newPath = filenameDir / shlibName;
    if (newPath.is_regular_file()) {
      resultFilename = newPath;
      return true;
    }
  }

  return false;
}

} // namespace
