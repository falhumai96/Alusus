/**
 * @file Core/Main/LibraryManager.cpp
 * Contains the implementation of class Core::Main::LibraryManager.
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

namespace Core::Main
{

void LibraryManager::addLibrary(PtrWord id, LibraryGateway *gateway)
{
  for (Word i = 0; i < this->entries.size(); ++i) {
    if (this->entries[i].id == id) {
      ASSERT(gateway == this->entries[i].gateway);
      ++this->entries[i].refCount;
      if (gateway != 0) gateway->initializeDuplicate(this->root);
      return;
    }
  }
  this->entries.push_back(Entry(id, gateway));
  if (gateway != 0) gateway->initialize(this->root);
}


void LibraryManager::removeLibrary(PtrWord id)
{
  for (Word i = 0; i < this->entries.size(); ++i) {
    if (this->entries[i].id == id) {
      if (this->entries[i].refCount == 1) {
        if (this->entries[i].gateway != 0) {
          this->entries[i].gateway->uninitialize(this->root);
        }
        this->entries.erase(this->entries.begin() + i);
      } else {
        --this->entries[i].refCount;
        if (this->entries[i].gateway != 0) {
          this->entries[i].gateway->uninitializeDuplicate(this->root);
        }
      }
      return;
    }
  }
  throw EXCEPTION(InvalidArgumentException, S("id"),
                  S("ID not found among loaded libraries."));
}


PtrWord LibraryManager::findLibrary(Char const *libId)
{
  for (Word i = 0; i < this->entries.size(); ++i) {
    if (this->entries[i].gateway != 0) {
      if (this->entries[i].gateway->getLibraryId() == libId) return this->entries[i].id;
    }
  }
  return 0;
}


LibraryGateway* LibraryManager::getGateway(PtrWord id)
{
  for (Word i = 0; i < this->entries.size(); ++i) {
    if (this->entries[i].id == id) return this->entries[i].gateway;
  }
  throw EXCEPTION(InvalidArgumentException, S("id"),
                  S("ID not found among loaded libraries."));
}


LibraryGateway* LibraryManager::getGateway(Char const *libId)
{
  for (Word i = 0; i < this->entries.size(); ++i) {
    if (this->entries[i].gateway != 0) {
      if (this->entries[i].gateway->getLibraryId() == libId) return this->entries[i].gateway;
    }
  }
  throw EXCEPTION(InvalidArgumentException, S("libId"),
                  S("ID not found among loaded libraries."), libId);
}

// TODO: Make the error string buffer dynamic.
static std::string getDlError(apr_dso_handle_t *dsoHandle) {
  constexpr size_t ERROR_BUFFER_SIZE = 256;
  char errorBuffer[ERROR_BUFFER_SIZE];
  apr_dso_error(dsoHandle, errorBuffer, ERROR_BUFFER_SIZE);
  return errorBuffer;
}

PtrWord LibraryManager::load(Char const *path, Str &error)
{
  apr_dso_handle_t *dsoHandle = nullptr;
  apr_status_t rv = apr_dso_load(&dsoHandle, path, dsoHandlesPool.getPool());
  if (rv != APR_SUCCESS) {
    if (error.getLength() != 0) error += S("\n");
    std::string errorString = getDlError(dsoHandle);
    error += errorString.c_str();
    return 0;
  }

  // Get the library gateway if this library supports it, otherwise we'll just load the library.
  LibraryGateway *gateway = nullptr;
  apr_dso_handle_sym_t gatewayGetterSym;
  rv = apr_dso_sym(&gatewayGetterSym, dsoHandle, LIBRARY_GATEWAY_GETTER_NAME);
  if (rv == APR_SUCCESS) {
    gateway = reinterpret_cast<LibraryGatewayGetter>(gatewayGetterSym)();
    if (!gateway) {
      // TODO: Check unloading errors.
      apr_dso_unload(dsoHandle);
      dsoHandle = nullptr;
      return 0;
    }
  }

  PtrWord id = reinterpret_cast<PtrWord>(dsoHandle);
  this->addLibrary(id, gateway);
  return id;
}


void LibraryManager::unload(PtrWord id)
{
  this->removeLibrary(id);
  // We won't be unloading the library at this point because some AST elements might be dependent on functions in this
  // library.
  // void *handle = reinterpret_cast<void*>(id);
  // dlclose(handle);
}


void LibraryManager::unloadAll()
{
  while (!this->entries.empty()) {
    this->unload(this->entries.back().id);
  }
}

} // namespace
