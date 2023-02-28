import os
def build_and_run_target(target: str):
    os.system("cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug")
    os.system("cmake --build build -t {}".format(target))

import sys
if __name__ == "__main__":
    build_and_run_target(sys.argv[1])