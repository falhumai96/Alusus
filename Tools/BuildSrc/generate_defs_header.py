import json
import os
import sys


if __name__ == "__main__":
    out_dir = sys.argv[1]
    version = sys.argv[2]
    revision = sys.argv[3]
    shlib_version = sys.argv[4]
    release_date = sys.argv[5]
    hijri_release_date = sys.argv[6]
    compiler_used = sys.argv[7]
    cmake_build_type = sys.argv[8]
    cmake_system_name = sys.argv[9]
    cmake_system_processor = sys.argv[10]
    bin_dir_name = sys.argv[11]
    lib_dir_name = sys.argv[12]
    include_dir_name = sys.argv[13]

    defs_path = os.path.join(out_dir, "AlususDefs.h")

    output = ""

    output += "#define ALUSUS_VERSION {alusus_version}".format(
        alusus_version=json.dumps(version))
    output += "\n"
    output += "#define ALUSUS_REVISION {alusus_revision}".format(
        alusus_revision=json.dumps(revision))
    output += "\n"
    output += "#define ALUSUS_SHLIB_VERSION {alusus_shlib_version}".format(
        alusus_shlib_version=json.dumps(shlib_version))
    output += "\n"
    output += "#define ALUSUS_RELEASE_DATE {alusus_release_date}".format(
        alusus_release_date=json.dumps(release_date))
    output += "\n"
    output += "#define ALUSUS_HIJRI_RELEASE_DATE {alusus_hijri_release_date}".format(
        alusus_hijri_release_date=json.dumps(hijri_release_date))
    output += "\n"
    output += "#define ALUSUS_COMPILER_USED {alusus_compiler_used}".format(
        alusus_compiler_used=json.dumps(compiler_used))
    output += "\n"
    output += "#define ALUSUS_CMAKE_BUILD_TYPE {alusus_cmake_build_type}".format(
        alusus_cmake_build_type=json.dumps(cmake_build_type))
    output += "\n"
    output += "#define ALUSUS_CMAKE_BUILD_TYPE_LOWER {alusus_cmake_build_type}".format(
        alusus_cmake_build_type=json.dumps(cmake_build_type).lower())
    output += "\n"
    if cmake_build_type.lower() == "debug":
        output += "#define ALUSUS_USE_LOGS"
        output += "\n"
    output += "#define ALUSUS_CMAKE_SYSTEM_NAME {alusus_cmake_system_name}".format(
        alusus_cmake_system_name=json.dumps(cmake_system_name))
    output += "\n"
    output += "#define ALUSUS_CMAKE_SYSTEM_NAME_LOWER {alusus_cmake_system_name}".format(
        alusus_cmake_system_name=json.dumps(cmake_system_name.lower()))
    output += "\n"
    output += "#define ALUSUS_CMAKE_SYSTEM_PROCESSOR {alusus_cmake_system_processor}".format(
        alusus_cmake_system_processor=json.dumps(cmake_system_processor))
    output += "\n"
    output += "#define ALUSUS_CMAKE_SYSTEM_PROCESSOR_LOWER {alusus_cmake_system_processor}".format(
        alusus_cmake_system_processor=json.dumps(cmake_system_processor).lower())
    output += "\n"
    output += "#define ALUSUS_BIN_DIR_NAME {alusus_bin_dir_name}".format(
        alusus_bin_dir_name=json.dumps(bin_dir_name))
    output += "\n"
    output += "#define ALUSUS_LIB_DIR_NAME {alusus_lib_dir_name}".format(
        alusus_lib_dir_name=json.dumps(lib_dir_name))
    output += "\n"
    output += "#define ALUSUS_INCLUDE_DIR_NAME {alusus_include_dir_name}".format(
        alusus_include_dir_name=json.dumps(include_dir_name))
    output += "\n"

    # Don't write a new file if the output didn't change.
    if os.path.isfile(defs_path):
        with open(defs_path, "r") as fd:
            old_output = fd.read()
            if old_output == output:
                exit(0)

    # Update the file.
    os.makedirs(out_dir, exist_ok=True)
    with open(defs_path, "w") as fd:
        fd.write(output)
