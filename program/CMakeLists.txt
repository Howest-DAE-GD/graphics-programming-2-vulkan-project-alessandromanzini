# Program CMakeList.txt, "Author": Manzini Alessandro
# Program specific logic here.
#
project("Rasterizer")

# create program target WIN32
add_executable(${PROJECT_NAME}
		"src/main.cpp" )
		
# link library
target_link_libraries(${PROJECT_NAME}
		PRIVATE "VulkanEngine" )

# include program directories
target_include_directories(${PROJECT_NAME}
		PRIVATE "./include"
		PRIVATE "./src" )

# Set the source and destination directories for resources
set(REFRESH_SCRIPTS_COMMAND "RefreshResources")
set(RESOURCES_SRC_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(RESOURCES_DST_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/resources")

# Gather all resources in the source directory
file(GLOB_RECURSE MY_RESOURCES "${RESOURCES_SRC_FOLDER}/*")

# Define a custom command to copy resources
# This command depends on the files in RESOURCES_SRC_FOLDER
add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${RESOURCES_SRC_FOLDER} ${RESOURCES_DST_FOLDER} 
	DEPENDS ${MY_RESOURCES}
	COMMENT "Refreshing resources ..." )

# Create a custom target that always checks files
# ALL specifier makes sure it runs everytime if and only if the output of the custom command 
# is out of date with respect to its DEPENDS file(s)
add_custom_target(${REFRESH_SCRIPTS_COMMAND} ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR} )

# Ensure your main project depends on the custom command
add_dependencies(${PROJECT_NAME} ${REFRESH_SCRIPTS_COMMAND})
