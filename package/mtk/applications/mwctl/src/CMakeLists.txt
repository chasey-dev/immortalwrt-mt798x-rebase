cmake_minimum_required(VERSION 2.8)

PROJECT(mwctl C)
ADD_DEFINITIONS(-Os -Wall -Werror --std=gnu99 -g3)

file(GLOB SRC_LIST "*.c")
ADD_EXECUTABLE(mwctl ${SRC_LIST})
TARGET_LINK_LIBRARIES(mwctl nl-tiny)

SET(CMAKE_INSTALL_PREFIX /usr)

INSTALL(TARGETS mwctl
	RUNTIME DESTINATION sbin
)
