// Apple filesystem functions
#include "xos/filesystem.h"

#include <stdexcept>

#include "CoreFoundation/CoreFoundation.h"

namespace xos::filesystem
{
    // This function makes relative paths work in C++ by changing directory to the Resources folder inside the .app bundle
    void configure_relative_path( )
    {
        const CFBundleRef mainBundle = CFBundleGetMainBundle( );
        const CFURLRef resourcesURL  = CFBundleCopyResourcesDirectoryURL( mainBundle );
        char path[PATH_MAX];
        if ( !CFURLGetFileSystemRepresentation( resourcesURL, TRUE, reinterpret_cast<UInt8*>( path ), PATH_MAX ) )
        {
            throw std::runtime_error( "Failed to get file system representation of bundle's resources directory." );
        }
        CFRelease( resourcesURL );

        chdir( path );
    }

}
