cmake_minimum_required (VERSION 3.2)

project(udr)

if (CMAKE_OPENXGUDR_BUILD_TYPE STREQUAL Debug)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -DDEBUG_IS_ON=1 -DTRACE_IS_ON=1 -DINFO_IS_ON=1")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_OPENXGUDR_DIR}/build/ext)
set(SRC_DIR ${CMAKE_OPENXGUDR_DIR}/src)


include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

include_directories(${SRC_DIR}/model)
include_directories(${SRC_DIR}/api)
include_directories(${SRC_DIR}/impl)
include_directories(${SRC_DIR}/common)
include_directories(${SRC_DIR}/udr_app)

# ${CMAKE_CURRENT_SOURCE_DIR}
file(GLOB SRCS
    ${SRC_DIR}/api/*.cpp
    ${SRC_DIR}/impl/*.cpp
    ${SRC_DIR}/model/*.cpp
    ${SRC_DIR}/common/*.cpp
    ${SRC_DIR}/udr_app/*.cpp

    ${EXTERNAL_INSTALL_LOCATION}/build/src/spdlog/*.cpp
)

add_executable(${PROJECT_NAME} ${SRCS} )
target_link_libraries(${PROJECT_NAME} pistache pthread mysqlclient config++)
