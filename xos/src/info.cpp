#include <xos/info.h>

#include <dlfcn.h>

namespace xos::info
{
    void log_info( std::ostream& stream )
    {
        Dl_info info;
        if ( dladdr( reinterpret_cast<void*>( log_info ), &info ) )
        {
            stream << "XOS lib loaded from: " << info.dli_fname << std::endl;
        }
        else
        {
            throw std::runtime_error( "Failed to retrieve library path!" );
        }
    }

}
