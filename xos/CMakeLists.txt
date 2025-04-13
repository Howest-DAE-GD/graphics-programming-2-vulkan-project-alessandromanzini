# XOS CMakeList.txt, "Author": alessandromanzini
# Engine specific logic here.
#
project("xos")

# create library target
add_library(${PROJECT_NAME} SHARED )

if (APPLE)
    target_sources(${PROJECT_NAME}
            PUBLIC "src/apple/filesystem.cpp"
            PUBLIC "src/apple/info.cpp" )

    # for apple frameworks
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation")

elseif(WIN32)
    target_sources(${PROJECT_NAME}
            PUBLIC "src/win32/filesystem.cpp"
            PUBLIC "src/win32/info.cpp" )

    # for windows DLLs
    target_compile_definitions(${PROJECT_NAME} PRIVATE MYLIB_EXPORTS)

endif()

# set warning level to W4 and warnings as errors
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

target_include_directories(${PROJECT_NAME}
        PUBLIC "./include"
        PUBLIC "./include/xos"
        PRIVATE "./src"
        PRIVATE "./src/apple"
        PRIVATE "./src/win32" )
