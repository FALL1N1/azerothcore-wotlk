#
# Copyright (C) 2016+     AzerothCore <www.azerothcore.org>, released under GNU AGPL3 v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

if( WIN32 )
  list(APPEND PRIVATE_SOURCES ${winDebugging})
  if ( MSVC )
    list(APPEND PRIVATE_SOURCES logonserver.rc)
  endif()
endif()
 
if (USE_COREPCH)
  set(PRIVATE_PCH_HEADER PrecompiledHeaders/logonPCH.h)
endif()

# Group sources
GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(logonserver
  ${PRIVATE_SOURCES})

if(NOT WIN32)
  set_target_properties(logonserver PROPERTIES
    COMPILE_DEFINITIONS _ACORE_CORE_CONFIG="${CONF_DIR}/logonserver.conf")
endif()

add_dependencies(logonserver revision.h)

if(UNIX AND NOT NOJEM)
  set(logonserver_LINK_FLAGS "-pthread -lncurses ${logonserver_LINK_FLAGS}")
endif()

set_target_properties(logonserver PROPERTIES LINK_FLAGS "${logonserver_LINK_FLAGS}")

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

target_include_directories(logonserver
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(logonserver
  PRIVATE
    game-interface
  PUBLIC
    game
    shared
    logon
    scripts
    gsoap
    readline
    ${GPERFTOOLS_LIBRARIES})

set_target_properties(logonserver
  PROPERTIES
    FOLDER
      "server")

if( WIN32 )
  if ( MSVC )
    add_custom_command(TARGET logonserver
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/logonserver.conf.dist ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
    )
  elseif ( MINGW )
    add_custom_command(TARGET logonserver
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/logonserver.conf.dist ${CMAKE_BINARY_DIR}/bin/
    )
  endif()
endif()

if( UNIX )
  install(TARGETS logonserver DESTINATION bin)
elseif( WIN32 )
  install(TARGETS logonserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()

install(FILES logonserver.conf.dist DESTINATION ${CONF_DIR})

# Generate precompiled header
if( USE_COREPCH )
  add_cxx_pch(logonserver ${PRIVATE_PCH_HEADER})
endif()

# handle config file
CU_GET_GLOBAL("MODULE_CONFIG_FILE_LIST")
FOREACH(configFile ${MODULE_CONFIG_FILE_LIST})
if( WIN32 )
  if ( MSVC )
    add_custom_command(TARGET logonserver
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy "${configFile}" ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
    )
  elseif ( MINGW )
    add_custom_command(TARGET logonserver
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy "${configFile}" ${CMAKE_BINARY_DIR}/bin/
    )
  endif()
endif()
install(FILES "${configFile}" DESTINATION ${CONF_DIR})
ENDFOREACH()

CU_GET_GLOBAL("MODULE_CONFIG_FILE_LIST")
FOREACH(configFile ${MODULE_CONFIG_FILE_LIST})
    get_filename_component(file_name ${configFile} NAME_WE)
    set(CONFIG_LIST ${CONFIG_LIST}${file_name},)
ENDFOREACH()
add_definitions(-DCONFIG_FILE_LIST=$<1:"${CONFIG_LIST}">)
# end handle config file

CU_RUN_HOOK("AFTER_LOGONSERVER_CMAKE")
