cmake_minimum_required (VERSION 3.19)

set(PROJECT "ndk-opengl-sample")
set(TARGET "ndkogl_sample")
project(${PROJECT})

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

set(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
cmake_path(NORMAL_PATH PROJECT_DIR)
set(SOURCE_DIR "${PROJECT_DIR}/src")
set(INCLUDE_DIR "${PROJECT_DIR}/include")

file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.c")
file(GLOB_RECURSE HEADERS "${SOURCE_DIR}/*.hpp" "${SOURCE_DIR}/*.h" "${INCLUDE_DIR}/*.hpp" "${INCLUDE_DIR}/*.h")

# Set their directories
foreach (SOURCE_FILE ${SOURCES};${HEADERS})

	# Get relative path
	cmake_path(RELATIVE_PATH SOURCE_FILE BASE_DIRECTORY ${PROJECT_DIR})

	# Extract directory
	get_filename_component(SOURCE_FILE_DIR ${SOURCE_FILE} DIRECTORY)

	# Set source group
	string(REPLACE "/" "\\" SOURCE_FILE_DIR ${SOURCE_FILE_DIR})
	source_group("${SOURCE_FILE_DIR}" FILES ${SOURCE_FILE})

endforeach ()

# Set include directories
set(INCLUDE_DIR "${SOURCE_DIR}/" "${INCLUDE_DIR}/")

add_library(${TARGET} SHARED ${SOURCES} ${HEADERS})
#add_executable(${TARGET} ${SOURCES} ${HEADERS})

if (ANDROID) # android

	add_library(native_app_glue STATIC
		${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

	target_include_directories(${TARGET} PRIVATE
		${ANDROID_NDK}/sources/android/native_app_glue)

	set(CMAKE_SHARED_LINKER_FLAGS
		"${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

	target_link_libraries(${TARGET} native_app_glue android EGL GLESv3 log)

else ()

	message(FATAL_ERROR "This project is meant only for Android")

endif ()

include_directories(${INCLUDE_DIR})
add_definitions(-DTARGET="${TARGET}")

# Enable compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Hide ZERO_CHECK and ALL_BUILD targets
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER meta)
