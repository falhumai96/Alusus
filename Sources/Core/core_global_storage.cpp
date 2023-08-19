/**
 * @file Core/core_global_storage.cpp
 * Contains functions for setting and getting global storage.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) &&               \
    !defined(__CYGWIN__)
#define DL_EXPORTED extern "C" __declspec(dllexport)
#else
#define DL_EXPORTED extern "C" __attribute__((__visibility__("default")))
#endif

void *globalStorage=0;

DL_EXPORTED void* getCoreGlobalStorage()
{
    return ::globalStorage;
}

DL_EXPORTED void setCoreGlobalStorage(void *strg)
{
    ::globalStorage = strg;
}
