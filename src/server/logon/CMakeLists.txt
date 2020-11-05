# Copyright (C)
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

CU_RUN_HOOK(BEFORE_GAME_LIBRARY)

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

if (USE_COREPCH)
  set(PRIVATE_PCH_HEADER PrecompiledHeaders/logonPCH.h)
endif ()

# Group sources
GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_library(logon-interface INTERFACE)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

target_include_directories(logon-interface
  INTERFACE
    ${PUBLIC_INCLUDES})

target_link_libraries(logon-interface
  INTERFACE
    common
    shared)

add_library(logon STATIC
  ${PRIVATE_SOURCES}
  ${ElunaLuaEngineFiles})

add_dependencies(logon revision.h)

target_include_directories(logon
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(logon
  PRIVATE
    logon-interface
    scripts-interface)

set_target_properties(logon
  PROPERTIES
    FOLDER
      "server")

# Generate precompiled header
if (USE_COREPCH)
  add_cxx_pch(logon ${PRIVATE_PCH_HEADER})
endif ()

CU_RUN_HOOK(AFTER_GAME_LIBRARY)
