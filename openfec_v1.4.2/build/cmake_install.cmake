# Install script for directory: /home/xiaohuigao/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/xiaohuigao/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/build/src/cmake_install.cmake")
  include("/home/xiaohuigao/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/build/applis/eperftool/cmake_install.cmake")
  include("/home/xiaohuigao/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/build/applis/howto_examples/simple_client_server/cmake_install.cmake")
  include("/home/xiaohuigao/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/build/tools/descr_stats_v1.2/cmake_install.cmake")
  include("/home/xiaohuigao/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/build/tests/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/xiaohuigao/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
