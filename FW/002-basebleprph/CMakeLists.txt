cmake_minimum_required(VERSION 3.22)


include(${CMAKE_CURRENT_LIST_DIR}/../3rdparty/esp-idf/tools/cmake/project.cmake)


#添加main组件
idf_build_component(${CMAKE_CURRENT_LIST_DIR}/../001-base/main "project_components")

#添加basebleprph组件
idf_build_component(${CMAKE_CURRENT_LIST_DIR}/basebleprph "project_components")

project(002-basebleprph)

#spiffs
spiffs_create_partition_image(storage spiffs_image FLASH_IN_PROJECT)

