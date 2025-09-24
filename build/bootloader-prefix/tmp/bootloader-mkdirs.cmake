# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/hui/esp/esp-idf/components/bootloader/subproject"
  "/home/hui/esp_test/Item/build/bootloader"
  "/home/hui/esp_test/Item/build/bootloader-prefix"
  "/home/hui/esp_test/Item/build/bootloader-prefix/tmp"
  "/home/hui/esp_test/Item/build/bootloader-prefix/src/bootloader-stamp"
  "/home/hui/esp_test/Item/build/bootloader-prefix/src"
  "/home/hui/esp_test/Item/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/hui/esp_test/Item/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/hui/esp_test/Item/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
