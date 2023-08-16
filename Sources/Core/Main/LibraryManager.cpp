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

#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#else
#include <dlfcn.h>
#endif
#include <AlususOSAL.hpp>
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

PtrWord LibraryManager::load(Char const *path, Str &error)
{
#if defined(_WIN32) || defined(WIN32)
  // We can specify Windows flags if we start using LoadLibraryEx. For now, flags are ignored.
  auto flags = 0;
#else
  auto flags = RTLD_NOW | RTLD_GLOBAL;
#if defined(__linux__)
  flags |= RTLD_DEEPBIND; // To start lookup symbols from the library itself initially.
#endif
#endif
  void *handle = AlususOSAL::dlopen(path, flags);
  if (!handle) {
    if (error.getLength() != 0) error += S("\n");
    error += AlususOSAL::dlerror();
    return 0;
  }

  // Get the library gateway if this library supports it, otherwise we'll just load the library.
  LibraryGateway *gateway = nullptr;
  LibraryGatewayGetter fn = reinterpret_cast<LibraryGatewayGetter>(AlususOSAL::dlsym(handle, LIBRARY_GATEWAY_GETTER_NAME));
  if (fn) {
    gateway = fn();
    if (!gateway) {
      AlususOSAL::dlclose(handle);
      handle = nullptr;
      return 0;
    }
  }

  PtrWord id = reinterpret_cast<PtrWord>(handle);
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
