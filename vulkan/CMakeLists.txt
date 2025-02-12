# VulkanEngine CMakeList.txt, "Author": Manzini Alessandro
# Engine specific logic here.
#
project("VulkanEngine")

# create library target
add_library("${PROJECT_NAME}"
	"src/test.cpp" )
		
# the include folder and resources has headers that will be shared to external project.
# the src folder also need to be included in case we have 
# headers for internal use.
target_include_directories("${PROJECT_NAME}"
		PUBLIC "./include"
		PRIVATE "./src" )

# fetch libraries
include(glfw_fetchcontent)
include(glm_fetchcontent)

target_link_libraries("${PROJECT_NAME}"
		PUBLIC glfw 
		PUBLIC glm )
