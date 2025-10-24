cmake_minimum_required(VERSION 3.10)
project(mockturtle LANGUAGES CXX)

# ======================================================
# 🔧 Global settings
# ======================================================
set(MOCKTURTLE_CXX_STANDARD "17" CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD ${MOCKTURTLE_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ======================================================
# ⚙️ Build Options
# ======================================================
option(MOCKTURTLE_BUILD_EXAMPLES "Build mockturtle examples" ON)
option(MOCKTURTLE_BUILD_TESTS "Build mockturtle tests" OFF)
option(MOCKTURTLE_BUILD_EXPERIMENTS "Build mockturtle experiments" OFF)

option(BILL_Z3 "Enable Z3 interface for bill library" OFF)
option(MOCKTURTLE_ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)
option(MOCKTURTLE_ENABLE_MATPLOTLIB "Enable matplotlib library in experiments" OFF)
option(MOCKTURTLE_ENABLE_NAUTY "Enable the Nauty library for percy" OFF)
option(MOCKTURTLE_ENABLE_ABC "Enable linking ABC as a static library" OFF)
option(MOCKTURTLE_ENABLE_ASAN "Enable AddressSanitizer for mockturtle" OFF)

# ======================================================
# 🧠 Compiler Settings
# ======================================================
if(UNIX)
  include(CheckCXXCompilerFlag)
  add_compile_options(-W -Wall -Wextra)

  foreach (WARNING unknown-pragmas gnu-anonymous-struct nested-anon-types)
    check_cxx_compiler_flag("-Wno-${WARNING}" HAS_WNO_${WARNING})
    if (HAS_WNO_${WARNING})
      add_compile_options(-Wno-${WARNING})
    endif()
  endforeach()

  if (MOCKTURTLE_ENABLE_COVERAGE)
    add_compile_options(-O0 -g --coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
  endif()

  if (MOCKTURTLE_ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -DADDRESS_SANITIZER)
    add_link_options(-fsanitize=address)
  endif()
endif()

if(MSVC)
  add_compile_options(/EHsc /bigobj)
endif()

if (WIN32)
  set(MOCKTURTLE_ENABLE_NAUTY OFF)
endif()

# ======================================================
# 📦 Core Subdirectories
# ======================================================
add_subdirectory(include)
add_subdirectory(lib)

# ======================================================
# 🧩 Optional Features
# ======================================================
if(MOCKTURTLE_ENABLE_NAUTY)
  add_definitions(-DMOCKTURTLE_ENABLE_NAUTY)
endif()

if (MOCKTURTLE_ENABLE_ABC)
  if (NOT EXISTS ${PROJECT_SOURCE_DIR}/lib/abc_static/libabc.a)
    message(FATAL_ERROR "Cannot find libabc.a in lib/abc_static/. For more info, see https://github.com/lsils/abc-staticlib")
  endif()
endif()

# ======================================================
# ▶️ Examples (optional, guarded for CI)
# ======================================================
if(MOCKTURTLE_BUILD_EXAMPLES)
  if (EXISTS "${CMAKE_SOURCE_DIR}/examples/CMakeLists.txt")
    message(STATUS "Including examples directory.")
    add_subdirectory(examples)
  else()
    message(WARNING "Skipping examples/ directory — not found.")
  endif()
endif()

# ======================================================
# 🧪 Tests (optional, guarded)
# ======================================================
if(MOCKTURTLE_BUILD_TESTS)
  if (EXISTS "${CMAKE_SOURCE_DIR}/test/CMakeLists.txt")
    message(STATUS "Including test directory.")
    add_subdirectory(test)
  else()
    message(WARNING "Skipping test/ directory — not found.")
  endif()
endif()

# ======================================================
# 🧫 Experiments (optional, guarded)
# ======================================================
if(MOCKTURTLE_BUILD_EXPERIMENTS)
  if (EXISTS "${CMAKE_SOURCE_DIR}/experiments/CMakeLists.txt")
    message(STATUS "Including experiments directory.")
    add_subdirectory(experiments)
  else()
    message(WARNING "Skipping experiments/ directory — not found.")
  endif()
endif()

# ======================================================
# ✅ Summary Message
# ======================================================
message(STATUS "")
message(STATUS "-----------------------------------------------")
message(STATUS "  Mockturtle Build Configuration Summary")
message(STATUS "-----------------------------------------------")
message(STATUS "  C++ Standard:            ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Examples:          ${MOCKTURTLE_BUILD_EXAMPLES}")
message(STATUS "  Build Tests:             ${MOCKTURTLE_BUILD_TESTS}")
message(STATUS "  Build Experiments:       ${MOCKTURTLE_BUILD_EXPERIMENTS}")
message(STATUS "  Enable Coverage:         ${MOCKTURTLE_ENABLE_COVERAGE}")
message(STATUS "  Enable AddressSanitizer: ${MOCKTURTLE_ENABLE_ASAN}")
message(STATUS "  Enable Nauty:            ${MOCKTURTLE_ENABLE_NAUTY}")
message(STATUS "  Enable ABC Static Lib:   ${MOCKTURTLE_ENABLE_ABC}")
message(STATUS "-----------------------------------------------")
message(STATUS "")
