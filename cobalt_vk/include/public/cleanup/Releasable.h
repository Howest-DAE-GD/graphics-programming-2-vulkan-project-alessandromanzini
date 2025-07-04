#ifndef RELEASABLE_H
#define RELEASABLE_H

#include <instance/VulkanInstance.h>

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

        virtual void release( VulkanInstance& instance ) = 0;


        [[nodiscard]] std::function<void( )> get_deleter( VulkanInstance& instance )
        {
            return std::bind( &Releasable::release, this, std::ref( instance ) );
        }

    };

}


#endif //!RELEASABLE_H
