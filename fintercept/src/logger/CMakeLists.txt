file(GLOB_RECURSE src_files
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(library_name "logger")

if(WIN32)
    add_executable(${library_name} WIN32 ${src_files})
	add_library(${library_name} SHARED)

	set_target_properties(
		${PROJECT_NAME}
		PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE"
				   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
				   ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
	)
endif()
