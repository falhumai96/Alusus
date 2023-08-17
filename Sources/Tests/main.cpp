/**
 * @file Tests/main.cpp
 * Contains the end-to-end test's main program.
 *
 * @copyright Copyright (C) 2023 Rafid Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

// System headers
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex>
#include <filesystem>

// Alusus header files
#include "core.h"
#include "AlususOSAL.hpp"

using Core::Notices::Notice;
using Core::Data::Ast::List;
using Core::Main::RootManager;

namespace Tests
{

Str resultFilename;

Bool isDirectory(Char const *path)
{
  struct stat path_stat;
  stat(path, &path_stat);
  return S_ISDIR(path_stat.st_mode);
}


Bool compareStringEnd(Str const &str, Char const *end)
{
  Int len = getStrLen(end);
  if (len >= str.getLength()) return false;
  return compareStr(str.getBuf()+str.getLength()-len, end) == 0;
}


/**
 * Executes the given Alusus source code and compare the result against
 * an output file (having the same name with .output at the end).
 *
 * @param[in] fileName  The name of Alusus source file name.
 *
 * @return Returns @c true if the function succeeds, otherwise @c false.
 */
Bool runSourceFile(Str const &fileName)
{
  // Redirect stdout to a file.
  fpos_t pos;
  fgetpos(stdout, &pos);
  int fd = dup(fileno(stdout));
  freopen(resultFilename,"w", stdout);

  try
  {
    // Prepare the root object;
    RootManager root;
    Slot<void, SharedPtr<Core::Notices::Notice> const&> noticeSlot(
      [](SharedPtr<Core::Notices::Notice> const &notice)->void
      {
        Core::Notices::printNotice(notice.get());
      }
    );
    root.noticeSignal.connect(noticeSlot);

    // Parse the provided filename.
    auto ptr = root.processFile(fileName);

    // Restore stdout.
    fflush(stdout);
    dup2(fd, fileno(stdout));
    close(fd);
    clearerr(stdout);
    fsetpos(stdout, &pos);

    if (ptr == 0)
      return false;

    return true;
  }
  catch (Srl::Exception &e)
  {
    // Restore stdout.
    fflush(stdout);
    dup2(fd, fileno(stdout));
    close(fd);
    clearerr(stdout);
    fsetpos(stdout, &pos);

    std::cout << "Failed to run source file " << fileName << "." << std::endl;
    std::cout << "The following error were thrown: " << std::endl;
    std::cout << e.getVerboseErrorMessage() << std::endl;
    return false;
  }
  catch (...)
  {
    // Restore stdout.
    fflush(stdout);
    dup2(fd, fileno(stdout));
    close(fd);
    clearerr(stdout);
    fsetpos(stdout, &pos);

    std::cout << "Failed to run source file " << fileName << "." << std::endl;
    throw;
  }
}


/**
 * Checks whether the run result of a source file matches the expected result.
 * The expected result is stored in a file having the same name followed by
 * ".output" post-fix.
 * @param[in] fileName  The name of Alusus source file name.
 *
 * @return Returns @c true if the run result matches the expected result,
 * otherwise @c false.
 */
Bool checkRunResult(Str const &fileName)
{
  std::ifstream runResult(resultFilename);
  std::string runResultContent((std::istreambuf_iterator<char>(runResult)),
      std::istreambuf_iterator<char>());

  std::ifstream expectedResult(fileName + ".output");
  std::string expectedResultContent((std::istreambuf_iterator<char>(expectedResult)),
      std::istreambuf_iterator<char>());

  auto massagedRunResultContent = std::regex_replace(
    runResultContent, std::regex("target datalayout = \"[a-zA-Z0-9:-]+\""), S("target datalayout = \"<sanitized>\"")
  );
  massagedRunResultContent = std::regex_replace(
    massagedRunResultContent, std::regex(", align [0-9]+"), S("")
  );
  auto massagedExpectedResultContent = std::regex_replace(
    expectedResultContent, std::regex("target datalayout = \"[a-zA-Z0-9:-]+\""), S("target datalayout = \"<sanitized>\"")
  );

  // Remove one character from the expectedResultContent because, for some
  // reason, editors seem to append 0A at the end of the file!
  auto ret =  massagedRunResultContent.compare(massagedExpectedResultContent) == 0;
  if (ret == true)
    std::cout << "Successful." << std::endl;
  else
  {
    std::cout << "Failed." << std::endl;
    std::cout << "Expected Result (Length = " << massagedExpectedResultContent.size() << "): " << std::endl;
    std::cout << massagedExpectedResultContent << std::endl;
    std::cout << "Received Result (Length = " << massagedRunResultContent.size() << "): " << std::endl;
    std::cout << massagedRunResultContent << std::endl;
  }
  return ret;
}


/**
 * Update the test snapshot with the result from the prev execution.
 *
 * @param[in] fileName  The name of Alusus source file name.
 */
void updateTestSnapshot(Str const &fileName)
{
  std::ifstream runResult(resultFilename);
  std::string runResultContent((std::istreambuf_iterator<char>(runResult)),
      std::istreambuf_iterator<char>());

  std::ofstream expectedResult(fileName + ".output");
  expectedResult << runResultContent;

  std::cout << "Done. " << std::endl;
}


/**
 * Runs the given Alusus source file and checks the result against an expected
 * content file. The expected result is stored in a file having the same name
 * of the source file followed by ".output" post-fix.
 *
 * @param[in] fileName  The name of Alusus source file name.
 *
 * @return Returns @c true if the run result matches the expected result,
 * otherwise @c false.
 */
Bool runAndCheckSourceFile(Str const &fileName)
{
  if (getenv(S("ALUSUS_TEST_UPDATE")) != 0) {
    std::cout << ">>> Updating " << fileName << ": ";
  } else {
    std::cout << ">>> Testing " << fileName << ": ";
  }
  if (!runSourceFile(fileName))
    return false;
  if (getenv(S("ALUSUS_TEST_UPDATE")) != 0) {
    updateTestSnapshot(fileName);
    return true;
  } else {
    return checkRunResult(fileName);
  }
}


/**
 * Runs all the end-to-end tests under given folder.
 *
 * @return Returns @c true if all test succeeds, otherwise @c false.
 */
Bool runEndToEndTests(Str const &dirPath, Char const *ext = ".alusus")
{
  DIR *dir;
  dirent *ent;
  if ((dir = opendir(dirPath)) != nullptr)
  {
    auto ret = true;
    while ((ent = readdir(dir)) != nullptr)
    {
      Str fileName(ent->d_name);
      Str filePath = dirPath + "/" + fileName;
      if (isDirectory(filePath) && fileName != "." && fileName != "..") {
        if (!runEndToEndTests(filePath, ext)) ret = false;
      } else if (
        compareStringEnd(fileName, ext) && fileName.find("-ignore.") == std::string::npos
      ) {
        if (!runAndCheckSourceFile(filePath)) ret = false;
      }
    }
    closedir(dir);
    return ret;
  }
  else
  {
    std::cout << "Could not open end-to-end tests directory: " << dirPath << " !" << std::endl;
    return false;
  }
}

} // namespace


using namespace Tests;

int main(int argc, char **oldArgv)
{
  // Set the codepage.
  AlususOSAL::UTF8CodePage utf8CodePage;

  // Get the UTF-8 args.
  char * const * argv;
  AlususOSAL::getUTF8Argv(&argv, oldArgv);

  Char alususReleaseYear[5];
  copyStr(ALUSUS_RELEASE_DATE, alususReleaseYear, 4);
  alususReleaseYear[4] = 0;
  std::cout << "Alusus End-to-End Tests\n"
               "Version " ALUSUS_VERSION ALUSUS_REVISION " (" ALUSUS_RELEASE_DATE ")\n"
               "Copyright (C) " << alususReleaseYear << " Rafid Khalid Abdullah\n\n";

  if (argc < 3 || argc > 4) {
    std::cout << "Invalid arguments";
    return EXIT_FAILURE;
  }
  AlususOSAL::Path repoPath = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
  AlususOSAL::Path l18nPath = repoPath / "Notices_L18n";
  AlususOSAL::Path subpath = repoPath / "Sources" / "Tests" / argv[1];
  Char const *ext = argv[2];
  if (argc == 4 && compareStr(argv[3], S("ar")) == 0) {
    Core::Notices::L18nDictionary::getSingleton()->initialize(S("ar"), l18nPath.u8c_str());
  } else {
    Core::Notices::L18nDictionary::getSingleton()->initialize(S("en"), l18nPath.u8c_str());
  }

  Core::Notices::setSourceLocationPathSkipping(true);

  // Prepare a temporary filename.
  Char const * tempPath = getenv("TMPDIR");
  if (tempPath == 0) tempPath = getenv("TMP");
  if (tempPath == 0) tempPath = getenv("TEMP");
  if (tempPath == 0) tempPath = getenv("TEMPDIR");
  if (tempPath == 0) tempPath = "/tmp/";
  resultFilename = tempPath;
  if (resultFilename(resultFilename.getLength() - 1) != '/') resultFilename += "/";
  resultFilename += "AlususEndToEndTest.txt";

  auto ret = EXIT_SUCCESS;
  if (!runEndToEndTests(subpath.u8c_str(), ext)) ret = EXIT_FAILURE;

  std::remove(resultFilename);

  return ret;
}
