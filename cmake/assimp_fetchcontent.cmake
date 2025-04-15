include(FetchContent)

# Fetch Assimp
FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v5.3.1
)

if (APPLE)
    set(ASSIMP_BUILD_ZLIB OFF CACHE BOOL "Disable Assimp's internal zlib" FORCE)
    set(ASSIMP_USE_SYSTEM_ZLIB ON CACHE BOOL "Use system zlib" FORCE)
endif ()

FetchContent_MakeAvailable(assimp)
