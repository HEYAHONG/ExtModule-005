
file(GLOB MAIN_C_CXX_FILES *.h *.cpp *.c  *.hpp)
idf_component_register(SRCS ${MAIN_C_CXX_FILES}
                       PRIV_REQUIRES fatfs
                       INCLUDE_DIRS ".")


add_compile_options( -Wno-error=clobbered -Wno-clobbered -Wno-error=cpp  )
include_directories(${CMAKE_CURRENT_LIST_DIR})
add_compile_definitions(-DHBOX_CONFIG_HEADER=hbox_config.h )
add_subdirectory(../../3rdparty/HCppBox/master/hbox HCppBox_hbox)
#启用hbox
hbox_enable(${COMPONENT_LIB})
##hbox的配置部分
target_link_libraries(HBox PRIVATE ${COMPONENT_LIB})
