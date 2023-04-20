
cmake_minimum_required(VERSION 3.16)

project(BUBYFU)

if(MSVC)
	add_compile_options(/EHsc /W4)
else()
	add_compile_options(-fno-asynchronous-unwind-tables -fno-exceptions -Wall -Wextra)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(_DEBUG MUNGWALL_NO_LINE_TRACKING)
endif()

set(sourceFiles "ls.cpp")

find_library(StdFuncs StdFuncs PATHS ../StdFuncs PATH_SUFFIXES build)
find_package(Qt5 COMPONENTS Core Gui Widgets)

if(${Qt5_FOUND} EQUAL 1)
	message(STATUS "Configuring using Qt5 directory ${Qt5_DIR}")

	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)

	add_executable(ls ${sourceFiles})
	add_compile_definitions(QT_GUI_LIB)
	target_link_libraries(ls ${StdFuncs})
	target_link_libraries(ls Qt5::Core Qt5::Gui Qt5::Widgets)
else()
	message(STATUS "Qt5 not found, configuring basic build")

	add_executable(ls ${sourceFiles})
	target_link_libraries(ls ${StdFuncs})
endif()

set_property(TARGET ls PROPERTY CXX_STANDARD 14)
set_property(TARGET ls PROPERTY CXX_STANDARD_REQUIRED ON)

if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "/SUBSYSTEM:CONSOLE")
	set_target_properties(ls PROPERTIES LINK_FLAGS /SUBSYSTEM:CONSOLE)
endif()

target_include_directories(ls PRIVATE "../StdFuncs")

if(USE_CLANG_TIDY)
	find_program(ClangTidy NAMES "clang-tidy")

	if(ClangTidy)
		message(STATUS "Found clang-tidy as ${ClangTidy}")
		set(RunClangTidy ${ClangTidy})
		set_target_properties(ls PROPERTIES C_CLANG_TIDY "${RunClangTidy}" CXX_CLANG_TIDY "${RunClangTidy}")
	else()
		message(STATUS "clang-tidy not found, so will not be used")
	endif()
endif()
