# Install script for directory: D:/Projects/CharacterForge/native/src/PhotoshopAPI/thirdparty/simdutf

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/PhotoshopAPIBuild")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/src/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/singleheader/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "simdutf_Development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/thirdparty/simdutf/include/simdutf.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "simdutf_Development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/thirdparty/simdutf/include/simdutf_c.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "simdutf_Development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/thirdparty/simdutf/include/simdutf")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "simdutf_Development" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/src/Debug/simdutf.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/src/Release/simdutf.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/src/MinSizeRel/simdutf.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/src/RelWithDebInfo/simdutf.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "simdutf_Development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/simdutf" TYPE FILE FILES
    "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/simdutf-config.cmake"
    "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/simdutf-config-version.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "example_Development" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/simdutf/simdutfTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/simdutf/simdutfTargets.cmake"
         "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/CMakeFiles/Export/4f66283cfb13f6b5cba5288387de34b2/simdutfTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/simdutf/simdutfTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/simdutf/simdutfTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/simdutf" TYPE FILE FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/CMakeFiles/Export/4f66283cfb13f6b5cba5288387de34b2/simdutfTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/simdutf" TYPE FILE FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/CMakeFiles/Export/4f66283cfb13f6b5cba5288387de34b2/simdutfTargets-debug.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/simdutf" TYPE FILE FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/CMakeFiles/Export/4f66283cfb13f6b5cba5288387de34b2/simdutfTargets-minsizerel.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/simdutf" TYPE FILE FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/CMakeFiles/Export/4f66283cfb13f6b5cba5288387de34b2/simdutfTargets-relwithdebinfo.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/simdutf" TYPE FILE FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/CMakeFiles/Export/4f66283cfb13f6b5cba5288387de34b2/simdutfTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/simdutf.pc")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "D:/Projects/CharacterForge/native/src/PhotoshopAPI/vs2022/thirdparty/simdutf/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
