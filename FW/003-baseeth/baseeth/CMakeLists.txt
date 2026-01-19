
file(GLOB MAIN_C_CXX_LD_FILES *.h *.cpp *.c  *.hpp)
idf_component_register(SRCS ${MAIN_C_CXX_LD_FILES}
                       PRIV_REQUIRES esp_eth esp_netif main lwip esp_driver_gpio esp_driver_spi
                       INCLUDE_DIRS "."
                       WHOLE_ARCHIVE TRUE)


add_compile_options( -Wno-error=clobbered -Wno-clobbered -Wno-error=cpp  )
include_directories(${CMAKE_CURRENT_LIST_DIR})
add_compile_definitions(-DHBOX_CONFIG_HEADER=hbox_config.h )

