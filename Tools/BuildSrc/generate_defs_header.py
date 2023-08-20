import argparse
import json
import os


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("out_dir", metavar="out-dir")
    parser.add_argument("--bin-dir-name", default="bin")
    parser.add_argument("--lib-dir-name", default="lib")
    parser.add_argument("--include-dir-name", default="include")
    args = parser.parse_args()

    defs_path = os.path.join(args.out_dir, "AlususDefs.h")

    output = "#define ALUSUS_BIN_DIR_NAME {alusus_bin_dir_name}".format(
        alusus_bin_dir_name=json.dumps(args.bin_dir_name))
    output += "\n"
    output += "#define ALUSUS_LIB_DIR_NAME {alusus_lib_dir_name}".format(
        alusus_lib_dir_name=json.dumps(args.lib_dir_name))
    output += "\n"
    output += "#define ALUSUS_INCLUDE_DIR_NAME {alusus_include_dir_name}".format(
        alusus_include_dir_name=json.dumps(args.include_dir_name))
    output += "\n"

    # Don't write a new file if the output didn't change.
    if os.path.isfile(defs_path):
        with open(defs_path, "r") as fd:
            old_output = fd.read()
            if old_output == output:
                exit(0)

    # Update the file.
    os.makedirs(args.out_dir, exist_ok=True)
    with open(defs_path, "w") as fd:
        fd.write(output)
