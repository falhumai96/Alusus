/**
 * @file Core/main.cpp
 * Contains the program's entry point.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core.h"
#include "OSAL.hpp"


Str computeL18nDictionaryPath(const Char *argvBinPath)
{
  // TODO: Properly report OS/APR errors.

  // Follow argvBinPath symlink.
  int error;
  std::string resolvedPath = followSymlink(argvBinPath, &error);
  if (error) {
    throw EXCEPTION(GenericException, S("Error following symlink."));
  }

  // Get CWD.
  AutoAPRPool pool;
  Char* cwd;
  apr_status_t rv = apr_filepath_get(&cwd, APR_FILEPATH_NATIVE, pool.getPool());
  if (rv != APR_SUCCESS) {
    throw EXCEPTION(GenericException, S("Could not get CWD."));
  }

  // Get abs path of followed symlink of argvBinPath.
  size_t neededLength = cwk_path_get_absolute(cwd, resolvedPath.c_str(), nullptr, 0) + 1;
  std::vector<Char> absolutePathBuffer(neededLength, 0);
  cwk_path_get_absolute(cwd, resolvedPath.c_str(), (Char*)absolutePathBuffer.data(), absolutePathBuffer.size());

  // Get "Notices_L18n" path.
  Str l18nPath;
  {
    // Get the dirname of the abs path.
    cwk_path_get_dirname(absolutePathBuffer.data(), &neededLength);
    Str dirname;
    if (neededLength > 0) {
      dirname = Str(absolutePathBuffer.data(), neededLength);
    } else {
      throw EXCEPTION(GenericException, S("Could not get binary directory."));
    }

    // Get the install path.
    cwk_path_get_dirname(dirname.getBuf(), &neededLength);
    Str installPath;
    if (neededLength > 0) {
      installPath = Str(dirname.getBuf(), neededLength);
    }

    // Join with "Notices_L18n".
    neededLength = cwk_path_join(installPath.getBuf(), "Notices_L18n", nullptr, 0) + 1;
    std::vector<Char> joinedPathBuffer(neededLength, 0);
    cwk_path_join(installPath.getBuf(), "Notices_L18n", (Char*)joinedPathBuffer.data(), joinedPathBuffer.size());

    // Normalize the path.
    neededLength = cwk_path_normalize(joinedPathBuffer.data(), nullptr, 0) + 1;
    std::vector<Char> normalizedPathBuffer(neededLength, 0);
    cwk_path_normalize(joinedPathBuffer.data(), (Char*)normalizedPathBuffer.data(), normalizedPathBuffer.size());

    l18nPath = normalizedPathBuffer.data();
  }

  return l18nPath;
}

/**
 * @defgroup main Main
 * @brief Contains elements related to the main program.
 */

using namespace Core;

/**
 * @brief The entry point of the program.
 * @ingroup main
 *
 * Parse the file with the name passed from the command line.
 */
int main(int argCount, char **args)
{
  // Get system language.
  Str lang = getSystemLanguage();

  // Set the UTF-8 codepage.
  setUTF8CP();
  std::atexit(restoreOriginalCP);

  apr_status_t rv;

  // Initialize APR.
  rv = apr_app_initialize(&argCount, (const char* const**)&args, NULL);
  if (rv != APR_SUCCESS) {
    if (lang == "ar") {
      fprintf(stderr, "خطأ (%d) في تهيئة APR" APR_EOL_STR, APR_TO_OS_ERROR(rv));
    }
    else {
      fprintf(stderr, "Error (%d) initializing APR" APR_EOL_STR, APR_TO_OS_ERROR(rv));
    }
    return 1;
  }
  std::atexit(apr_terminate);

  // Main APR pool.
  AutoAPRPool mainPool;

  // Get the output stream.
  apr_file_t* cStdoutFile;
  rv = apr_file_open_stdout(&cStdoutFile, mainPool.getPool());
  if (rv != APR_SUCCESS) {
    if (lang == "ar") {
      fprintf(stderr, "خطأ (%d) في فتح مخرج الإخراج الأساسي لـ APR" APR_EOL_STR, APR_TO_OS_ERROR(rv));
    }
    else {
      fprintf(stderr, "Error (%d) opening APR stdout" APR_EOL_STR, APR_TO_OS_ERROR(rv));
    }
    return 1;
  }
  AutoAPRFile stdoutFile(cStdoutFile);
  APRFilebuf stdoutBuf(stdoutFile.getFile());
  std::ostream outStream(&stdoutBuf);

  Bool help = false;
  Bool interactive = false;
  Char const *sourceFile = 0;
  Bool dump = false;
  if (argCount < 2) help = true;
  for (Int i = 1; i < argCount; ++i) {
    if (strcmp(args[i], S("--help")) == 0) help = true;
    else if (strcmp(args[i], S("--مساعدة")) == 0) help = true;
    else if (strcmp(args[i], S("--interactive")) == 0) interactive = true;
    else if (strcmp(args[i], S("--تفاعلي")) == 0) interactive = true;
    else if (strcmp(args[i], S("-i")) == 0) interactive = true;
    else if (strcmp(args[i], S("-ت")) == 0) interactive = true;
    else if (strcmp(args[i], S("--dump")) == 0) dump = true;
    else if (strcmp(args[i], S("--إلقاء")) == 0) dump = true;
#ifdef ALUSUS_USE_LOGS
    // Parse the log option.
    else if (strcmp(args[i], S("--log")) == 0 || strcmp(args[i], S("--تدوين")) == 0) {
      if (i < argCount-1) {
        ++i;
        Logger::setFilter(atoi(args[i]));
      } else {
        Logger::setFilter(0);
      }
    }
#endif
    else {
      sourceFile = args[i];
      break;
    }
  }

  Str l18nPath = computeL18nDictionaryPath(args[0]);
  if (lang == "ar" || lang == "en") {
    // TODO: Support other locales.
    Core::Notices::L18nDictionary::getSingleton()->initialize(lang, l18nPath.getBuf());
  }

  // We'll show help if now source file is given.
  if (sourceFile == 0 && !interactive) help = true;

  if (help) {
    Char alususReleaseYear[5];
    Char alususHijriReleaseYear[5];
    copyStr(ALUSUS_RELEASE_DATE, alususReleaseYear, 4);
    copyStr(ALUSUS_HIJRI_RELEASE_DATE, alususHijriReleaseYear, 4);
    alususReleaseYear[4] = alususHijriReleaseYear[4] = 0;
    // Check if the command line was in English by detecting if the first character is ASCII.
    if (lang == "ar") {
      // Write Arabic help.
      outStream << "لغة الأسُس" << APR_EOL_STR <<
        "الإصدار (" << ALUSUS_VERSION ALUSUS_REVISION << ")" << APR_EOL_STR << "(" << ALUSUS_RELEASE_DATE << " م)" << APR_EOL_STR << "(" << ALUSUS_HIJRI_RELEASE_DATE << " هـ)" << APR_EOL_STR <<
        "جميع الحقوق محفوظة لـ سرمد خالد عبدالله (" << alususReleaseYear << " م) \\ (" << alususHijriReleaseYear << " هـ)" << APR_EOL_STR << APR_EOL_STR;
      outStream << "نُشر هذا البرنامج برخصة الأسُس العامة، الإصدار 1.0، والمتوفرة على الرابط أدناه." << APR_EOL_STR <<
        "يرجى قراءة الرخصة قبل استخدام البرنامج. استخدامك لهذا البرنامج أو أي من الملفات" << APR_EOL_STR <<
        "المرفقة معه إقرار منك أنك قرأت هذه الرخصة ووافقت على جميع فقراتها." << APR_EOL_STR;
      outStream << APR_EOL_STR << "Alusus Public License: <https://alusus.org/ar/license.html>" << APR_EOL_STR;
      outStream << APR_EOL_STR << "طريقة الاستخدام:" << APR_EOL_STR;
      outStream << "الأسُس [<خيارات القلب>] <الشفرة المصدرية> [<خيارات البرنامج>]" << APR_EOL_STR;
      outStream << "الشفرة المصدرية = اسم الملف الحاوي على الشفرة المصدرية" << APR_EOL_STR;
      outStream << "alusus [<Core options>] <source> [<program options>]" << APR_EOL_STR;
      outStream << "source = filename." << APR_EOL_STR;
      outStream << APR_EOL_STR << "الخيارات:" << APR_EOL_STR;
      outStream << "\tتنفيذ بشكل تفاعلي:" << APR_EOL_STR;
      outStream << "\t\t--تفاعلي" << APR_EOL_STR;
      outStream << "\t\t-ت" << APR_EOL_STR;
      outStream << "\t\t--interactive" << APR_EOL_STR;
      outStream << "\t\t-i" << APR_EOL_STR;
      outStream << "\tالقاء شجرة AST عند الانتهاء:" << APR_EOL_STR;
      outStream << "\t\t--شجرة" << APR_EOL_STR;
      outStream << "\t\t--dump" << APR_EOL_STR;
      #if defined(ALUSUS_USE_LOGS)
      outStream << "\tالتحكم بمستوى التدوين (قيمة من 6 بتات):" << APR_EOL_STR;
      outStream << "\t\t--تدوين" << APR_EOL_STR;
      outStream << "\t\t--log" << APR_EOL_STR;
      #endif
    }
    else {
      // Write English help.
      outStream << "Alusus Language" APR_EOL_STR
        "Version " ALUSUS_VERSION ALUSUS_REVISION " (" ALUSUS_RELEASE_DATE ")" APR_EOL_STR
        "Copyright (C) " << alususReleaseYear << " Sarmad Khalid Abdullah" APR_EOL_STR APR_EOL_STR;
      outStream << "This software is released under Alusus Public License, Version 1.0." APR_EOL_STR
        "For details on usage and copying conditions read the full license at" APR_EOL_STR
        "<https://alusus.org/license.html>. By using this software you acknowledge" APR_EOL_STR
        "that you have read the terms in the license and agree with and accept all such" APR_EOL_STR
        "terms." APR_EOL_STR APR_EOL_STR;
      outStream << "Usage: alusus [<Core options>] <source> [<program options>]" APR_EOL_STR;
      outStream << "source = filename." APR_EOL_STR;
      outStream << APR_EOL_STR "Options:" APR_EOL_STR;
      outStream << "\t--interactive, -i  Run in interactive mode." APR_EOL_STR;
      outStream << "\t--dump  Tells the Core to dump the resulting AST tree." APR_EOL_STR;
      #if defined(ALUSUS_USE_LOGS)
      outStream << "\t--log  A 6 bit value to control the level of details of the log." APR_EOL_STR;
      #endif
    }
    return EXIT_SUCCESS;
  }
  else if (interactive) {
    // Run in interactive mode.
    if (lang == "ar") {
      outStream << "تنفيذ بشكل تفاعلي." APR_EOL_STR;
      outStream << "إضغط على CTRL+C للخروج." APR_EOL_STR APR_EOL_STR;
    }
    else {
      outStream << "Running in interactive mode." APR_EOL_STR;
      outStream << "Press CTRL+C to exit." APR_EOL_STR APR_EOL_STR;
    }

    try {
      // Prepare the root object;
      Main::RootManager root(argCount, args);
      root.setInteractive(true);
      root.setLanguage(lang);
      Slot<void, SharedPtr<Notices::Notice> const&> noticeSlot(
        [](SharedPtr<Notices::Notice> const &notice)->void
        {
          Notices::printNotice(notice.get());
        }
      );
      root.noticeSignal.connect(noticeSlot);

      // Get the input stream.
      apr_file_t* cStdinFile;
      rv = apr_file_open_stdin(&cStdinFile, mainPool.getPool());
      if (rv != APR_SUCCESS) {
        if (lang == "ar") {
          fprintf(stderr, "خطأ (%d) في فتح مخرج الإدخال الأساسي لـ APR" APR_EOL_STR, APR_TO_OS_ERROR(rv));
        }
        else {
          fprintf(stderr, "Error (%d) opening APR stdin" APR_EOL_STR, APR_TO_OS_ERROR(rv));
        }
        return 1;
      }
      AutoAPRFile stdinFile(cStdinFile);
      APRFilebuf stdinBuf(stdinFile.getFile());
      std::istream inStream(&stdinBuf);

      // Parse the standard input stream.
      Processing::InteractiveCharInStream charStream(inStream, outStream);
      root.processStream(&charStream, S("user input"));
    } catch (Exception &e) {
      outStream << e.getVerboseErrorMessage() << APR_EOL_STR;
      return EXIT_FAILURE;
    } catch (Int v) {
      return v;
    }
    return EXIT_SUCCESS;
  } else {
    // Parse the provided source file.
    try {
      // Prepare the root object;
      Main::RootManager root(argCount, args);
      root.setLanguage(lang);
      Slot<void, SharedPtr<Notices::Notice> const&> noticeSlot(
        [](SharedPtr<Notices::Notice> const &notice)->void
        {
          Notices::printNotice(notice.get());
        }
      );
      root.noticeSignal.connect(noticeSlot);

      // Parse the provided filename.
      TioSharedPtr ptr = root.processFile(sourceFile);
      if (ptr == 0) return EXIT_SUCCESS;

      // Print the parsed data.
      if (dump) {
        outStream << APR_EOL_STR << S("-- BUILD COMPLETE --") << APR_EOL_STR << APR_EOL_STR <<
          S("Build Results:") << APR_EOL_STR << APR_EOL_STR;
        Data::dumpData(outStream, ptr.get(), 0);
        outStream << APR_EOL_STR;
      }
    } catch (FileException &e) {
      if (e.getComment() == S("invalid")) {
        if (lang == "ar") {
          outStream << S("صنف الملف غير صالح: ") << e.getFileName() << APR_EOL_STR;
        } else {
          outStream << S("Invalid file type: ") << e.getFileName() << APR_EOL_STR;
        }
      } else {
        if (lang == "ar") {
          outStream << S("الملف مفقود: ") << e.getFileName() << APR_EOL_STR;
        } else {
          outStream << S("File not found: ") << e.getFileName() << APR_EOL_STR;
        }
      }
      return EXIT_FAILURE;
    } catch (Exception &e) {
      outStream << e.getVerboseErrorMessage() << APR_EOL_STR;
      return EXIT_FAILURE;
    } catch (Int v) {
      return v;
    }
    return EXIT_SUCCESS;
  }
}
