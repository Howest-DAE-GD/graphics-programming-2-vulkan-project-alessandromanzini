#ifndef RELEASABLE_H
#define RELEASABLE_H

#include <vulkan/vulkan_core.h>

#include <functional>


namespace cobalt_vk::cleanup
{
    class Releasable
    {
    public:
        Releasable( ) = default;
        virtual ~Releasable( ) = default;

        Releasable( const Releasable& )                = delete;
        Releasable( Releasable&& ) noexcept            = delete;
        Releasable& operator=( const Releasable& )     = delete;
        Releasable& operator=( Releasable&& ) noexcept = delete;

        virtual void release( VkDevice device ) = 0;


        [[nodiscard]] std::function<void( VkDevice )> get_deletor( )
        {
            return std::bind( &Releasable::release, this, std::placeholders::_1 );
        }

    };

}


#endif //!RELEASABLE_H
