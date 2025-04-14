# Master CMakeList.txt, "Author": alessandromanzini
# Project specific logic here.
#
cmake_minimum_required (VERSION 3.20)

# set the C++ standard to C++20 and ensure it's required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
message(STATUS "C++20 support has been enabled by default.")

# set maximum warning level and treat warnings as errors
# add_compile_options(-Wall -Wextra -Wpedantic -Werror)
# message(STATUS "W4 warning level and treat warnings as errors has been enabled.")

# set the project name
project (VulkanRasterizer)

# enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# append cmake module path
# making sure cmake finds our custom modules (.cmake)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Add subdirectories
add_subdirectory("cobalt_vk")
add_subdirectory("xos")
add_subdirectory("application")
