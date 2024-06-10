# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/ESP-IDF/Espressif/frameworks/esp-idf-v4.4.6/components/bootloader/subproject"
  "D:/ESP-IDF/Espressif/frameworks/esp-idf-v4.4.6/examples/camera_test/build/bootloader"
  "D:/ESP-IDF/Espressif/frameworks/esp-idf-v4.4.6/examples/camera_test/build/bootloader-prefix"
  "D:/ESP-IDF/Espressif/frameworks/esp-idf-v4.4.6/examples/camera_test/build/bootloader-prefix/tmp"
  "D:/ESP-IDF/Espressif/frameworks/esp-idf-v4.4.6/examples/camera_test/build/bootloader-prefix/src/bootloader-stamp"
  "D:/ESP-IDF/Espressif/frameworks/esp-idf-v4.4.6/examples/camera_test/build/bootloader-prefix/src"
  "D:/ESP-IDF/Espressif/frameworks/esp-idf-v4.4.6/examples/camera_test/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/ESP-IDF/Espressif/frameworks/esp-idf-v4.4.6/examples/camera_test/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
