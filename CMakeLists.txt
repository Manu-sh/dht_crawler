project(main_dht_crawler)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
	# set(CMAKE_BUILD_TYPE Release)
	set(CMAKE_BUILD_TYPE Debug) # keep assertion() on
endif()

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# equivalent of: strip --strip-all
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
set(EXECUTABLE_OUTPUT_PATH, ${PROJECT_SOURCE_DIR}/bin)
set(INC_PATH  ${PROJECT_SOURCE_DIR})
set(SRC_PATH  ${PROJECT_SOURCE_DIR})

# cmake file name globbing
# file(GLOB SRC_FILES ${SRC_PATH}/torrent_info/*.cpp)
# ${SRC_FILES}	

add_executable(${PROJECT_NAME} 
	${PROJECT_NAME}.cpp
	${SRC_PATH}/dht_crawler.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${INC_PATH})
target_link_libraries(${PROJECT_NAME} PRIVATE torrent-rasterbar)
target_link_libraries(${PROJECT_NAME} PRIVATE pthread)

target_compile_options(${PROJECT_NAME}
	PRIVATE
		-Wall -Wextra -Wno-unused-function -pipe -pedantic
		-fstack-protector-all -fstack-protector-strong
		-O3 -ffast-math -funroll-loops -march=native
)
