# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/varunshitole/Desktop/mockturtle-master/build/my_npn_synthesis/build_cmake/_deps/kitty-src")
  file(MAKE_DIRECTORY "/Users/varunshitole/Desktop/mockturtle-master/build/my_npn_synthesis/build_cmake/_deps/kitty-src")
endif()
file(MAKE_DIRECTORY
  "/Users/varunshitole/Desktop/mockturtle-master/build/my_npn_synthesis/build_cmake/_deps/kitty-build"
  "/Users/varunshitole/Desktop/mockturtle-master/build/my_npn_synthesis/build_cmake/_deps/kitty-subbuild/kitty-populate-prefix"
  "/Users/varunshitole/Desktop/mockturtle-master/build/my_npn_synthesis/build_cmake/_deps/kitty-subbuild/kitty-populate-prefix/tmp"
  "/Users/varunshitole/Desktop/mockturtle-master/build/my_npn_synthesis/build_cmake/_deps/kitty-subbuild/kitty-populate-prefix/src/kitty-populate-stamp"
  "/Users/varunshitole/Desktop/mockturtle-master/build/my_npn_synthesis/build_cmake/_deps/kitty-subbuild/kitty-populate-prefix/src"
  "/Users/varunshitole/Desktop/mockturtle-master/build/my_npn_synthesis/build_cmake/_deps/kitty-subbuild/kitty-populate-prefix/src/kitty-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/varunshitole/Desktop/mockturtle-master/build/my_npn_synthesis/build_cmake/_deps/kitty-subbuild/kitty-populate-prefix/src/kitty-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/varunshitole/Desktop/mockturtle-master/build/my_npn_synthesis/build_cmake/_deps/kitty-subbuild/kitty-populate-prefix/src/kitty-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
