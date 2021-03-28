cmake_minimum_required(VERSION 2.8...3.19)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

PROJECT(msged C)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(BUILD_SHARED_LIBS "Build shared libs" ON)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  ADD_DEFINITIONS(-g -DDEBUG -D_DEBUG)
  set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "My multi config types" FORCE)
else(CMAKE_BUILD_TYPE MATCHES Debug)
  ADD_DEFINITIONS(-DNDEBUG)
  set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "My multi config types" FORCE)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

if (MSVC)
  if (BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-D_DLL -D_MAKE_DLL)
  else()
    set(CompilerFlags
      CMAKE_CXX_FLAGS
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_C_FLAGS
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_RELEASE
    )
    foreach(CompilerFlag ${CompilerFlags})
      string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
  endif()
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../out/lib")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../out/bin")
  ADD_DEFINITIONS(-DUSE_MSGAPI -DWIN32 -D_WINDOWS -D_CONSOLE -D_CRT_SECURE_NO_WARNINGS)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/h;${CMAKE_CURRENT_SOURCE_DIR}/../huskylib;${CMAKE_CURRENT_SOURCE_DIR}/../fidoconf;${CMAKE_CURRENT_SOURCE_DIR}/../smapi")
  if(CMAKE_BUILD_TYPE MATCHES Debug)
    list( APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../out/lib/Debug" )
  else(CMAKE_BUILD_TYPE MATCHES Debug)
    list( APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../out/lib/Release" )
  endif(CMAKE_BUILD_TYPE MATCHES Debug)
else ()
  if (NOT BUILD_SHARED_LIBS)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  endif()
  include_directories("h")
  ADD_DEFINITIONS(-Wall -DUNIX -DUSE_MSGAPI)
endif (MSVC)

find_library(husky_LIB NAMES husky PATHS "${CMAKE_FIND_ROOT_PATH}")
find_library(fidoconfig_LIB NAMES fidoconfig PATHS "${CMAKE_FIND_ROOT_PATH}")
find_library(smapi_LIB NAMES smapi PATHS "${CMAKE_FIND_ROOT_PATH}")

if (UNIX)
if (CMAKE_SYSTEM_NAME MATCHES FreeBSD)
find_library(curses_LIB termcap HINTS "${TERMCAP_ROOT_DIR}" PATHS /sw /usr /usr/local /opt/local PATH_SUFFIXES lib)
ADD_DEFINITIONS(-DUNAME=\"BSD\")
elseif (CMAKE_SYSTEM_NAME MATCHES Darwin)
find_library(curses_LIB ncurses HINTS "${NCURSES_ROOT_DIR}" PATHS /sw /usr /usr/local /opt/local PATH_SUFFIXES lib)
ADD_DEFINITIONS(-DUNAME=\"MacOS\")
elseif (CMAKE_SYSTEM_NAME MATCHES Linux)
find_library(curses_LIB ncurses HINTS "${NCURSES_ROOT_DIR}" PATHS /sw /usr /usr/local /opt/local PATH_SUFFIXES lib)
ADD_DEFINITIONS(-DUNAME=\"LNX\")
endif ()
endif (UNIX)

message("-------------------------")
message("System            : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
message("Compiler          : ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message("Prefix            : ${CMAKE_INSTALL_PREFIX} (run \"cmake . -DCMAKE_INSTALL_PREFIX:PATH=/other\" for other prefix)")
message("libhusky          : ${husky_LIB}")
message("libsmapi          : ${smapi_LIB}")
message("curses            : ${curses_LIB}")
message("Build shared      : ${BUILD_SHARED_LIBS}")
message("Build type        : ${CMAKE_CONFIGURATION_TYPES}")
message("-------------------------")

ADD_EXECUTABLE(msged addr.c ansi.c areas.c bmg.c charset.c config.c control.c
  curses.c date.c dialogs.c dirute.c dlgbox.c dlist.c echotoss.c environ.c
  fconf.c fecfg145.c fido.c filedlg.c flags.c freq.c gestr120.c getopts.c
  group.c help.c helpcmp.c helpinfo.c init.c keycode.c list.c maintmsg.c
  makemsgn.c memextra.c menu.c misc.c mnu.c msg.c msged.c mxbt.c normalc.c
  nshow.c quick.c quote.c readmail.c readtc.c screen.c strextra.c system.c
  template.c textfile.c timezone.c userlist.c vsev.c vsevops.c win.c wrap.c
)
target_link_libraries(msged ${smapi_LIB} ${husky_LIB} ${curses_LIB})

INSTALL(TARGETS msged
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
