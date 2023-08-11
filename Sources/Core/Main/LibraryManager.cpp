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




#if defined(_WIN32) || defined(WIN32)
// Returns the last Win32 error, in string format. Returns an empty string if there is no error.
// Adapted from https://stackoverflow.com/a/17387176 [accessed August 8th, 2023].
std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    // Ask Win32 to give us the string version of that message ID.
    // The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}
#endif


PtrWord LibraryManager::load(Char const *path, Str &error)
{

#if defined(_WIN32) || defined(WIN32)

  HMODULE handle = LoadLibraryA(path);
  if (!handle) {
    if (error.getLength() != 0) error += S("\n");
    auto errorString = GetLastErrorAsString();
    error += errorString.c_str();
    return 0;
  }

  // Get the library gateway if this library supports it, otherwise we'll just load the library.
  LibraryGateway *gateway = nullptr;
  LibraryGatewayGetter fn = reinterpret_cast<LibraryGatewayGetter>(GetProcAddress(handle, LIBRARY_GATEWAY_GETTER_NAME));
  if (fn != nullptr) {
    gateway = fn();
    if (gateway == nullptr) {
      FreeLibrary(handle);
      return 0;
    }
  }

  PtrWord id = reinterpret_cast<PtrWord>(handle);
  this->addLibrary(id, gateway);
  return id;

#else

  auto flags = RTLD_NOW | RTLD_GLOBAL;
#if defined(__linux__)
  flags |= RTLD_DEEPBIND; // To start lookup symbols from the library itself initially.
#endif
  void *handle = dlopen(path, flags);
  if (!handle) {
    if (error.getLength() != 0) error += S("\n");
    error += dlerror();
    return 0;
  }

  // Get the library gateway if this library supports it, otherwise we'll just load the library.
  LibraryGateway *gateway = nullptr;
  LibraryGatewayGetter fn = reinterpret_cast<LibraryGatewayGetter>(dlsym(handle, LIBRARY_GATEWAY_GETTER_NAME));
  if (fn) {
    gateway = fn();
    if (!gateway) {
      dlclose(handle);
      return 0;
    }
  }

  PtrWord id = reinterpret_cast<PtrWord>(handle);
  this->addLibrary(id, gateway);
  return id;

#endif

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
