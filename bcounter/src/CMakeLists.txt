file(GLOB_RECURSE src_files
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${src_files})

	set_target_properties(
		${PROJECT_NAME}
		PROPERTIES LINK_FLAGS_DEBUG   "/SUBSYSTEM:CONSOLE"
				   LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
				   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
	)
endif()
