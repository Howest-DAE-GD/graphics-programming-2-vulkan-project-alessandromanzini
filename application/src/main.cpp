#include "TriangleApplication.h"

#include <iostream>


int main( )
{
    try
    {
        cobalt::TriangleApplication app{};
        app.run( );
    }
    catch ( std::exception const& e )
    {
        std::cerr << e.what( ) << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
