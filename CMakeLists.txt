cmake_minimum_required(VERSION 2.8)

project(Luasel)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(CreateGroups)

find_package(Boost REQUIRED)
find_package(Lua51 REQUIRED)

if (UNIX OR MINGW)
    add_definitions(-std=c++0x)
    set(CMAKE_CXX_CUSTOM_FLAGS "-Wall -Wextra -Winvalid-pch")
    set(CMAKE_CXX_CUSTOM_FLAGS_DEBUG "-g -DDEBUG -D_DEBUG -UNDEBUG")
    set(CMAKE_CXX_CUSTOM_FLAGS_RELEASE "-O3 -DNDEBUG -UDEBUG -U_DEBUG")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
else ()
    set(CMAKE_CXX_CUSTOM_FLAGS "/MP -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS")
    set(CMAKE_CXX_CUSTOM_FLAGS_DEBUG "-DDEBUG -D_DEBUG")
    set(CMAKE_CXX_CUSTOM_FLAGS_RELEASE "-DNDEBUG /Ox /Ot")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_CUSTOM_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_CUSTOM_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_CUSTOM_FLAGS_RELEASE}")

include_directories(${Boost_INCLUDE_DIR} ${LUA_INCLUDE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB luasel_inc "${CMAKE_CURRENT_SOURCE_DIR}/include/luasel/*.hpp")
file(GLOB luasel_inc_details "${CMAKE_CURRENT_SOURCE_DIR}/include/luasel/details/*.hpp")
file(GLOB luasel_src "${CMAKE_CURRENT_SOURCE_DIR}/src/*.[ch]pp")

set(src
	${luasel_inc}
	${luasel_inc_details}
	${luasel_src}
)

CreateGroups("${src}")
add_library(luasel ${src})

target_link_libraries(luasel ${LUA_LIBRARIES})

add_subdirectory(example)