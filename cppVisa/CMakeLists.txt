#/////////////////////////////////////////////////////////////////////////////
#// Author:      Alexandre CARPENTIER
#// Modified by:
#// Created:     26/03/2026
#// Copyright:   (c) Alexandre CARPENTIER
#// Licence:     LGPL-2.1-or-later
#/////////////////////////////////////////////////////////////////////////////

# go modern
cmake_minimum_required (VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)

# Activez Rechargement à chaud pour les compilateurs MSVC si cela est pris en charge.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("cppVisa")

file(GLOB_RECURSE SOURCE_FILES "library_src/*.cpp" "library_src/*.h")
file(GLOB_RECURSE EXEC_FILES "executable_src/*.cpp" "executable_src/*.h")

# VISA library path
set(VISA_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/NIVISA/Lib_x64/msc/visa64.lib")

add_library(cppVisa SHARED ${SOURCE_FILES})
target_link_libraries(cppVisa PRIVATE ${VISA_LIB_PATH})

add_executable(tests "executable_src/tests.cpp")
target_link_libraries(tests PRIVATE cppVisa)

add_executable(tests_advanced "executable_src/tests_advanced.cpp")
target_link_libraries(tests_advanced PRIVATE cppVisa)
