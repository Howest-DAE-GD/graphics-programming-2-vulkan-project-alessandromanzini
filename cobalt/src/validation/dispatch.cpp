#include <validation/dispatch.h>

#include <stdexcept>


namespace cobalt::validation
{
    void throw_runtime_error( std::string const& error_message )
    {
        throw std::runtime_error( error_message );
    }

}
