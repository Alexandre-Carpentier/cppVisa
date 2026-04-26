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

project ("cppVisaProject")
add_subdirectory("cppVisa")