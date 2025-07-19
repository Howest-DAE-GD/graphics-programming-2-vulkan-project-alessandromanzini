#ifndef COBALT_LOG_H
#define COBALT_LOG_H

#include <format>
#include <string_view>


namespace cobalt::log
{
    void logerr( std::string_view const& sender, std::string_view const& message, bool conditional = true );
    void loginfo( std::string_view const& sender, std::string_view const& message, bool conditional = true );

    template <typename class_t>
    void logerr( std::string_view const& sender, std::string_view const& message, bool const conditional = true )
    {
        logerr( std::format( "{}::{}", typeid( class_t ).name( ), sender ), message, conditional );
    }

    template <typename class_t>
    void loginfo( std::string_view const& sender, std::string_view const& message, bool const conditional = true )
    {
        loginfo( std::format( "{}::{}", typeid( class_t ).name( ), sender ), message, conditional );
    }

}


#endif //!COBALT_LOG_H
