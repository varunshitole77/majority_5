cmake_minimum_required(VERSION 3.14)
cmake_policy(VERSION 3.5...3.27)

project(mig_synthesis_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Options
# ============================================================
option(USE_MOCKTURTLE "Enable mockturtle integration" ON)

include(FetchContent)

if(USE_MOCKTURTLE)
  message(STATUS "Mockturtle ON: using FetchContent (header-only) for kitty/bill/percy/phmap + local Mockturtle")

  # ---------------- Kitty ----------------
  FetchContent_Declare(
    kitty
    GIT_REPOSITORY https://github.com/msoeken/kitty.git
    GIT_TAG master
  )
  FetchContent_Populate(kitty)
  add_library(kitty INTERFACE)
  target_include_directories(kitty INTERFACE ${kitty_SOURCE_DIR}/include)

  # ---------------- Bill ----------------
  FetchContent_Declare(
    bill
    GIT_REPOSITORY https://github.com/lsils/bill.git
    GIT_TAG master
  )
  FetchContent_Populate(bill)
  add_library(bill INTERFACE)
  target_include_directories(bill INTERFACE ${bill_SOURCE_DIR}/include)

  # ---------------- Percy ----------------
  FetchContent_Declare(
    percy
    GIT_REPOSITORY https://github.com/lsils/percy.git
    GIT_TAG master
  )
  FetchContent_Populate(percy)
  add_library(percy INTERFACE)
  target_include_directories(percy INTERFACE ${percy_SOURCE_DIR}/include)

  # ---------------- parallel-hashmap (phmap) ----------------
  FetchContent_Declare(
    phmap
    GIT_REPOSITORY https://github.com/greg7mdp/parallel-hashmap.git
    GIT_TAG master
  )
  FetchContent_Populate(phmap)
  add_library(phmap INTERFACE)
  target_include_directories(phmap INTERFACE
    ${phmap_SOURCE_DIR}
    ${phmap_SOURCE_DIR}/parallel_hashmap
  )

  # ---------------- fmt (try system first, otherwise fetch) ----------------
  find_package(fmt QUIET)
  if(NOT fmt_FOUND)
    message(STATUS "fmt not found system-wide, fetching with FetchContent")
    FetchContent_Declare(
      fmt
      GIT_REPOSITORY https://github.com/fmtlib/fmt.git
      GIT_TAG 10.2.1
    )
    FetchContent_MakeAvailable(fmt)
  endif()

  # ---------------- Mockturtle (local) ----------------
  set(MOCK_BASE /Users/varunshitole/Desktop/mockturtle-master/include)

  add_definitions(-DUSE_MOCKTURTLE)
endif()

# ============================================================
# Source files (shared)
# ============================================================
set(SOURCES
    src/truth_table.cpp
    src/npn_classifier.cpp
    src/mig_structure.cpp
    src/mig_synthesizer.cpp
    src/utils.cpp
)

# ============================================================
# Helper macro to set includes + deps for a target
# ============================================================
function(configure_target target_name)
  target_include_directories(${target_name} PRIVATE
    src
    ${MOCK_BASE}
  )
  if(USE_MOCKTURTLE)
    target_link_libraries(${target_name} PRIVATE kitty bill percy phmap fmt::fmt)
  endif()
endfunction()

# ============================================================
# Executables
# ============================================================
add_executable(main main.cpp ${SOURCES})
configure_target(main)

add_executable(compute_n1 compute/compute_n1.cpp ${SOURCES})
configure_target(compute_n1)

add_executable(compute_n2 compute/compute_n2.cpp ${SOURCES})
configure_target(compute_n2)

add_executable(compute_n3 compute/compute_n3.cpp ${SOURCES})
configure_target(compute_n3)

add_executable(compute_n4 compute/compute_n4.cpp ${SOURCES})
configure_target(compute_n4)

add_executable(compute_n5 compute/compute_n5.cpp ${SOURCES})
configure_target(compute_n5)

# ============================================================
# Output directory for all executables
# ============================================================
set_target_properties(
  main compute_n1 compute_n2 compute_n3 compute_n4 compute_n5
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
