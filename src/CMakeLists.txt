cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Iconv REQUIRED)

add_executable(
	${PROJECT_NAME}
	chset.cpp
	config.cpp
	convert.cpp
	main.cpp
	TSDescriptor.cpp
	TSHeader.cpp
	TSNITSection.cpp
	TSPacket.cpp
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${CMAKE_SOURCE_DIR}/json/single_include/nlohmann
	PRIVATE ${CMAKE_SOURCE_DIR}/iconvpp
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE Iconv::Iconv
)
