#include <log.h>

#include <cstdio>


namespace cobalt::log
{
    void log_impl( FILE* stream, std::string_view const& sender, std::string_view const& message )
    {
        fprintf( stream, "%s", std::format( "{}: {}\n", sender, message ).c_str( ) );
    }


    void logerr( std::string_view const& sender, std::string_view const& message, bool const conditional )
    {
        if ( conditional )
        {
            log_impl( stderr, sender, message );
        }
    }


    void loginfo( std::string_view const& sender, std::string_view const& message, bool const conditional )
    {
        if ( conditional )
        {
            log_impl( stdout, sender, message );
        }
    }

}
