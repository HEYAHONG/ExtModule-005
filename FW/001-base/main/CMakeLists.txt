
file(GLOB MAIN_C_CXX_LD_FILES *.h *.cpp *.c  *.hpp)
idf_component_register(SRCS ${MAIN_C_CXX_LD_FILES}
                       PRIV_REQUIRES fatfs
                       INCLUDE_DIRS "."
                       WHOLE_ARCHIVE TRUE)


add_compile_options( -Wno-error=clobbered -Wno-clobbered -Wno-error=cpp  )
include_directories(${CMAKE_CURRENT_LIST_DIR})
add_compile_definitions(-DHBOX_CONFIG_HEADER=hbox_config.h )
add_subdirectory(../../3rdparty/HCppBox/master/hbox HCppBox_hbox)
#启用hbox
hbox_enable(${COMPONENT_LIB})
##hbox的配置部分
target_link_libraries(HBox PRIVATE ${COMPONENT_LIB})

#链接脚本
target_linker_script(${COMPONENT_LIB} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/hbox_config.ld)
idf_component_get_property(esptool_elf2image_args esptool_py ESPTOOL_PY_ELF2IMAGE_ARGS)
list(APPEND esptool_elf2image_args   --use-segments)
idf_component_set_property(esptool_py ESPTOOL_PY_ELF2IMAGE_ARGS "${esptool_elf2image_args}")

