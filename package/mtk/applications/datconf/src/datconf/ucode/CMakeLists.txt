cmake_minimum_required(VERSION 3.10)

project(datconf-ucode)

# look for ucode library
FIND_LIBRARY(ucode_library NAMES ucode)

# find header directory
# find ucode include dir
FIND_PATH(ucode_include_dir ucode/module.h)

IF(NOT ucode_library OR NOT ucode_include_dir)
	MESSAGE(FATAL_ERROR "ucode library or headers not found")
ENDIF()

# include ucode
include_directories(${ucode_include_dir})
# include kvcutil, datconf
include_directories(${kvcutil_SOURCE_DIR})
include_directories(${datconf_SOURCE_DIR})

# compiling options refer to ucode src repo
add_definitions(-Os -Wall -Werror -g3 -std=gnu99)
IF(BUILD_UCODE)
	# add datconf-ucode library, MODULE as a plugin
	add_library(datconf-ucode MODULE datconf.c)

	# remove 'lib' prefix in .so
	set_target_properties(datconf-ucode PROPERTIES OUTPUT_NAME datconf PREFIX "")

	# link ucode, datconf, kvcutil explicitly
	target_link_libraries(datconf-ucode ${ucode_library} datconf kvcutil)

	# optimize linking
	set_target_properties(datconf-ucode PROPERTIES LINK_FLAGS "-Wl,--gc-sections")

	# install
	install(TARGETS datconf-ucode LIBRARY DESTINATION lib/ucode)
ENDIF()