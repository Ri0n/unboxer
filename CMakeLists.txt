# Copyright (c) 2021, Sergei Ilinykh <rion4ik@gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.12)
project(unboxer VERSION 1.0.0 DESCRIPTION "MP4 files parser and extractor")

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_TOOLS "Build demo tools" ON)
option(BUILD_STATIC "Build static unboxer library" ON)
option(BUILD_SHARED "Build shared unboxer library" OFF)

include(GNUInstallDirs)

if(BUILD_STATIC)
    set(UNBOXER_LIB_SUFFIX "_static")
elseif(BUILD_SHARED)
    set(UNBOXER_LIB_SUFFIX "")
else()
    message(FATAL_ERROR "All libraries types were disabled. Nothing to build.")
endif()

add_subdirectory(src)
if(BUILD_TESTS)
add_subdirectory(test)
endif()
if(BUILD_TOOLS)
add_subdirectory(tools)
endif()

install(FILES LICENSE TYPE DOC)

# packaging

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(CPACK_PACKAGE_CONTACT "rion4ik@@gmail.com")
  set(CPACK_PACKAGE_DESCRIPTION "${CMAKE_PROJECT_DESCRIPTION}")
  set(CPACK_PACKAGE_RELOCATABLE OFF)
  set(CPACK_PACKAGE_VENDOR "Sergei Ilinykh")

  if(CMAKE_VERSION VERSION_LESS "3.12")
    set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
    set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
  endif()

  set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)

  set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

  set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)
  set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

  include(CPack)
endif()
