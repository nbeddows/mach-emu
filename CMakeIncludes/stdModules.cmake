# Copyright (c) 2021-2024 Nicolas Beddows <nicolas.beddows@gmail.com>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set(STD_MODULES_BUILD_TIMESTAMP_FILE std_modules_build_timestamp)
# One time build only
add_custom_target(StdModules ALL
    DEPENDS ${STD_MODULES_BUILD_TIMESTAMP_FILE}
)

set(STD_MODULES_OPTIONS -fmodules-ts -std=c++20 -c -x c++-system-header)

# compile the required std modules
add_custom_command(
    OUTPUT ${STD_MODULES_BUILD_TIMESTAMP_FILE}
    COMMAND ${CMAKE_COMMAND} -E echo "Building standard libraries modules"
    COMMAND ${CMAKE_CXX_COMPILER} ${STD_MODULES_OPTIONS} ${STD_MODULES}
    COMMAND ${CMAKE_COMMAND} -E touch ${STD_MODULES_BUILD_TIMESTAMP_FILE}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    BYPRODUCTS
)
