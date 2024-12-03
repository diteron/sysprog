file(GLOB_RECURSE src_files
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(library_name "logger")

if(WIN32)
	add_library(${library_name} SHARED ${src_files})

	set_target_properties(
		${library_name}
		PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
				   ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
	)
endif()
