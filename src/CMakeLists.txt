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


project (libunboxer VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES CXX)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)

set(LIB_TARGET_NAME unboxer)

include(GNUInstallDirs)
include(GenerateExportHeader)

find_package( Qt5Core 5.12.0 REQUIRED )
find_package(Qt5 COMPONENTS Core Network REQUIRED)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

set(SOURCES
    unboxer_impl.cpp
    boxreader.cpp
    inputmemory_impl.cpp
    inputhttp_impl.cpp
    inputfile_impl.cpp
    blobextractor.cpp
    )
set(HEADERS
    status.h
    box.h
    unboxer.h
    unboxer_impl.h
    boxreader.h
    inputstreamer.h
    cacher.h
    input.h
    inputmemory_impl.h
    inputhttp_impl.h
    inputfile_impl.h
    blobextractor.h
    )


macro(add_library_type type suffix)
add_library (${LIB_TARGET_NAME}${suffix} ${type} ${SOURCES} ${HEADERS})
generate_export_header(${LIB_TARGET_NAME}${suffix}
  BASE_NAME unboxer
)
target_include_directories(${LIB_TARGET_NAME}${suffix} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include/${LIB_TARGET_NAME}>  # <prefix>/include/unboxer
)
target_include_directories(${LIB_TARGET_NAME}${suffix} PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries (${LIB_TARGET_NAME}${suffix} PUBLIC Qt5::Core Qt5::Network)
target_compile_definitions(${LIB_TARGET_NAME}${suffix} PRIVATE UNBOXER_LIBRARY)

install(TARGETS ${LIB_TARGET_NAME}${suffix} DESTINATION ${LIBRARY_INSTALL_DIR})
install(FILES
 ${HEADERS}
 ${PROJECT_BINARY_DIR}/unboxer_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIB_TARGET_NAME}
)

install(
  TARGETS ${LIB_TARGET_NAME}${suffix}
  EXPORT ${LIB_TARGET_NAME}${suffix}-targets
  RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION  ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT ${LIB_TARGET_NAME}${suffix}-targets
  NAMESPACE ${LIB_TARGET_NAME}::
  FILE ${LIB_TARGET_NAME}${suffix}-targets.cmake
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_TARGET_NAME}"
)
endmacro()

if(BUILD_STATIC)
  add_library_type(STATIC _static)
  string(TOUPPER ${LIB_TARGET_NAME}_static UPPER_STATIC_PROJECT_NAME)
  set_target_properties(${LIB_TARGET_NAME}_static PROPERTIES
    COMPILE_FLAGS -D${UPPER_STATIC_PROJECT_NAME}_STATIC_DEFINE)
endif()

if(BUILD_SHARED)
  add_library_type(SHARED "")
  set_target_properties(${LIB_TARGET_NAME} PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR})
endif()

include(CMakePackageConfigHelpers)


configure_package_config_file(
  "${CMAKE_SOURCE_DIR}/cmake/${LIB_TARGET_NAME}-config.cmake.in"
  ${LIB_TARGET_NAME}-config.cmake
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_TARGET_NAME}"
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/${LIB_TARGET_NAME}-config.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_TARGET_NAME}"
)
