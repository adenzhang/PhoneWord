project(phonewordcpp)

cmake_minimum_required( VERSION 2.6 )

set(PROJNAME phonewordcpp)

# check 64 bit
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( ARCH x64 )
else()
    set( ARCH x86 )
endif()

if( NOT CMAKE_BUILD_TYPE )
	set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	set( BUILDTYPE debug)
else()
	set( BUILDTYPE release)
endif()
message("*** ${ARCH}/${BUILDTYPE} ***")

# boost libraries
#set (BOOST_LIBS system program_options)

if( NOT Boost_DIR )
    if( EXISTS $ENV{BOOST_DIR} )
        set(Boost_DIR $ENV{BOOST_DIR})
        #message("will use boost path in env $ENV{BOOST_DIR}")
    else()
        #message("not set boost path")
    endif()
endif()
set(INCLUDES ${INCLUDES} ${Boost_DIR}/include)
#set(LIBPATHS ${LIBPATHS} ${Boost_DIR}/lib)
set(LIBPATHS ${LIBPATHS} ${Boost_DIR}/lib/${ARCH}/${BUILDTYPE})
if( NOT WIN32 )
	foreach(F ${BOOST_LIBS})
		set(LIBS ${LIBS} boost_${F})
	endforeach()
endif()
#find_library(Boost_LIBRARIES NAMES boost_system ${Boost_DIR}/lib/${ARCH}/${BUILDTYPE})
#set(LIBS ${LIBS} ${Boost_DIR}/lib/${ARCH}/${BUILDTYPE}/boost_system)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

add_definitions( -DBOOST_ALL_DYN_LINK -std=c++0x)

LINK_DIRECTORIES(${LIBPATHS})

message("- link libs: ${LIBS}")
message("- link dirs: ${LIBPATHS}")

#----------------------------------

if( NOT WIN32 )
	set(LIBS ${LIBS} -lrt)
endif()

set(INCLUDES ${INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/../src)

message("- INCLUDES dirs: ${INCLUDES}")

include_directories(${INCLUDES})

#file(GLOB SRC
#    "${CMAKE_CURRENT_SOURCE_DIR}/../src/*.h"
#    "${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp"
#)

set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cpp")
add_executable( ${PROJNAME} ${SRC} )
target_link_libraries( ${PROJNAME} ${LIBS})

#############

set(SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/../src/TriestoreMain.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/../src/TrieStore.c"
)
add_executable( TrieStore ${SRC} )
target_link_libraries( TrieStore ${LIBS})
