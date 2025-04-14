#include "TriangleApplication.h"

#include <xos/filesystem.h>
#include <xos/info.h>

#include <filesystem>
#include <iostream>


int main( )
{
    try
    {
        xos::info::log_info( std::clog );
        xos::filesystem::configure_relative_path( );

        engine::TriangleApplication app;
        app.run( );
    }
    catch ( const std::exception& e )
    {
        std::cerr << e.what( ) << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
