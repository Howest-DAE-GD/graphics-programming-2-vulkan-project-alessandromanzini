#include <validation/dispatch.h>


namespace cobalt_vk::validation
{
    void throw_runtime_error( const std::string& errorMessage )
    {
        throw std::runtime_error( errorMessage );
    }

}
