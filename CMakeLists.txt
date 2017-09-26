cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS "--std=c++14 ${CMAKE_CXX_FLAGS} -O3")


# Use the variable PROJECT_NAME for changing the target name
set(star_glob "star_glob")
set(star_glob_tool "star_globber")

# Set our project name
project(${star_glob})

# Use all the *.cpp files we found under this folder for the project
file(GLOB sources "*.cpp" 					 
)

set(star_glob_tool_sources ${sources})
list(REMOVE_ITEM sources "tool_main.cpp")

# Define the library
add_library(${star_glob} ${sources})
add_executable(${star_glob_tool} ${star_glob_tool_sources})

# Libraries
# Boost
find_package(Boost REQUIRED COMPONENTS
	system
	filesystem
	program_options
)

find_library(LSTARTAPE NAMES startape PATHS "../star-tape/build" "star-tape/build" STATIC)
find_library(LSIMPLEJSON NAMES SimpleJSON PATHS "../SimpleJSON/build" "SimpleJSON/build" STATIC)

message("-- External libraries")
message("	${LSIMPLEJSON}")
message("	${LSTARTAPE}")

# link
target_link_libraries(${star_glob} ${LSTARTAPE} ${LSIMPLEJSON} ${Boost_LIBRARIES})
target_link_libraries(${star_glob_tool} ${LSTARTAPE} ${LSIMPLEJSON} ${Boost_LIBRARIES})

# include
target_include_directories(${star_glob} PUBLIC "..")
target_include_directories(${star_glob_tool} PUBLIC "..")