cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")


# Use the variable PROJECT_NAME for changing the target name
set(star_glob "star_glob")
set(star_glob_tool "star_globber")

# Set our project name
project(${star_glob})

# Use all the *.cpp files we found under this folder for the project
file(GLOB sources "*.cpp"
				  "filesystem/*.cpp"
)

set(star_glob_tool_sources ${sources})
list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/tool_main.cpp)

# Define the library
add_library(${star_glob} ${sources})
add_executable(${star_glob_tool} ${star_glob_tool_sources})

# Libraries
# Boost
find_package(Boost REQUIRED COMPONENTS
	system
	iostreams
	filesystem
	program_options
)

find_package(BZip2)
find_package(ZLIB)

if(BZIP2_FOUND)
	message("-- BZip2 found, adding to target")
	target_compile_definitions(${star_glob} PUBLIC ENABLE_BZIP2=1)
	target_compile_definitions(${star_glob_tool} PUBLIC ENABLE_BZIP2=1)
else()
	set(BZIP2_LIBRARIES "")
endif()

if(ZLIB_FOUND)
	message("-- zlib found, adding to target")
	target_compile_definitions(${star_glob} PUBLIC ENABLE_GZIP=1)
	target_compile_definitions(${star_glob_tool} PUBLIC ENABLE_GZIP=1)
else()
	set(ZLIB_LIBRARIES "")
endif()


find_library(LSTARTAPE NAMES star_tape PATHS "../star-tape/build" "star-tape/build" STATIC)
find_library(LSIMPLEJSON NAMES SimpleJSON PATHS "../SimpleJSON/build" "SimpleJSON/build" STATIC)

message("-- External libraries")
message("	${LSIMPLEJSON}")
message("	${LSTARTAPE}")

# link
target_link_libraries(${star_glob} ${LSTARTAPE} ${LSIMPLEJSON} Boost::program_options Boost::iostreams Boost::filesystem Boost::system ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES} cryptopp)
target_link_libraries(${star_glob_tool} ${LSTARTAPE} ${LSIMPLEJSON} Boost::program_options Boost::iostreams Boost::filesystem Boost::system ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES} cryptopp)

# include
target_include_directories(${star_glob} PUBLIC "..")
target_include_directories(${star_glob_tool} PUBLIC "..")