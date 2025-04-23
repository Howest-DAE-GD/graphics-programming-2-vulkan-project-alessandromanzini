#ifndef RELEASABLE_H
#define RELEASABLE_H

#include <vulkan/vulkan_core.h>

#include <functional>


namespace cobalt_vk::cleanup
{
    class Releasable
    {
    public:
        Releasable( )          = default;
        virtual ~Releasable( ) = default;

        Releasable( const Releasable& )                = delete;
        Releasable( Releasable&& ) noexcept            = delete;
        Releasable& operator=( const Releasable& )     = delete;
        Releasable& operator=( Releasable&& ) noexcept = delete;


        virtual void release( VkDevice device ) = 0;


        [[nodiscard]] std::function<void( )> get_deletor( VkDevice device )
        {
            return std::bind( &Releasable::release, this, device );
        }

    };

}


#endif //!RELEASABLE_H
