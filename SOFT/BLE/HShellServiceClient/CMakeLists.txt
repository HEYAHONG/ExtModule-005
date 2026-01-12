

cmake_minimum_required(VERSION 3.13)

#程序名称
Set(PROJECT_NAME HShellServiceClient)
#主版本
set(PROJECT_MAJOR_VERSION_STR 1)
#次版本号
set(PROJECT_MINOR_VERSION_STR 0)
#修订号
set(PROJECT_REVISION_VERSION_STR 0)
#构建版本（以时间戳天数作为构建版本）
string(TIMESTAMP PROJECT_BUILD_TIME_SECOND "%s" UTC)
math(EXPR  PROJECT_BUILD_VERSION_STR "${PROJECT_BUILD_TIME_SECOND}/60/60/24"  OUTPUT_FORMAT DECIMAL)
#程序版本
set(PROJECT_VERSION_STR "${PROJECT_MAJOR_VERSION_STR}.${PROJECT_MINOR_VERSION_STR}.${PROJECT_REVISION_VERSION_STR}.${PROJECT_BUILD_VERSION_STR}")


Project(${PROJECT_NAME} C CXX ASM)
Project(${PROJECT_NAME}	VERSION "${PROJECT_VERSION_STR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)


file(GLOB MAIN_CXX_C_FILES *.cpp *.hpp *.c *.h )
list(APPEND  MAIN_SRCS ${MAIN_CXX_C_FILES})



add_executable(${PROJECT_NAME}  ${MAIN_SRCS})
target_link_libraries(${PROJECT_NAME} ${MAIN_LIBS})

#添加线程库
FIND_PACKAGE(Threads REQUIRED)
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  ${CMAKE_THREAD_LIBS_INIT})

#设置MSVC编译选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set_property(TARGET ${PROJECT_NAME}
		     PROPERTY WIN32_EXECUTABLE FALSE )
	target_compile_options(${PROJECT_NAME}
				PUBLIC /utf-8)
	
endif()

if(MSVC)
    # Use the static C library for all build types
    foreach(var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
      )
      if(${var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${var} "${${var}}")
      endif()
    endforeach()
    add_definitions(-D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING )
    add_compile_options(/Gd /EHsc)
endif(MSVC)

##添加HBox库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../FW/3rdparty/HCppBox/master/hbox HBox  EXCLUDE_FROM_ALL)
#启用HBox
hbox_enable(${PROJECT_NAME})



#添加simpleble库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../SimpleBLE//simpleble/ SimpleBLE  EXCLUDE_FROM_ALL)
target_link_libraries(${PROJECT_NAME} simpleble::simpleble)
file(GLOB_RECURSE SIMPLEBLE_HEADER_FILES SimpleBLE/simpleble/*.h )
target_sources(${PROJECT_NAME} PUBLIC  ${SIMPLEBLE_HEADER_FILES})

