file(GLOB_RECURSE src_files
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${src_files})

	set_target_properties(
		${PROJECT_NAME}
		PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE"
				   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
	)
endif()
