#ifndef COBALTINSTANCE_H
#define COBALTINSTANCE_H

#include <cleanup/DeletionQueue.h>
#include <cleanup/Releasable.h>
#include <instance/VulkanInstance.h>

#include <cassert>
#include <stack>


namespace cobalt_vk
{
    class CobaltInstance final
    {
    public:
        ~CobaltInstance( ) noexcept;

        CobaltInstance( const CobaltInstance& )                = delete;
        CobaltInstance( CobaltInstance&& ) noexcept            = delete;
        CobaltInstance& operator=( const CobaltInstance& )     = delete;
        CobaltInstance& operator=( CobaltInstance&& ) noexcept = delete;

        static CobaltInstance& get_instance( );

        [[nodiscard]] Window& get_window( ) const;
        [[nodiscard]] VulkanInstance& get_vk_instance( ) const;

        void register_window( std::unique_ptr<Window>&& window );
        void register_vk_instance( std::unique_ptr<VulkanInstance>&& instance );

        template <typename resource_t, typename... args_t>
            requires std::derived_from<resource_t, cleanup::Releasable>
        [[nodiscard]] resource_t& create_resource( args_t&&... args );

        void clear_cobalt_instance( );

    private:
        cleanup::DeletionQueue deletion_queue_{};
        std::vector<std::unique_ptr<cleanup::Releasable>> resources_{};

        std::unique_ptr<Window> window_ptr_{ nullptr };
        std::unique_ptr<VulkanInstance> vk_instance_ptr_{ nullptr };

        CobaltInstance( ) = default;

    };


    template <typename resource_t, typename... args_t>
        requires std::derived_from<resource_t, cleanup::Releasable>
    resource_t& CobaltInstance::create_resource( args_t&&... args )
    {
        assert( vk_instance_ptr_ && "Vulkan instance is not set up. Call setup_vulkan_instance() first!" );
        auto& resource = resources_.emplace_back( std::make_unique<resource_t>( std::forward<args_t>( args )... ) );
        deletion_queue_.push( resource->get_deleter( *vk_instance_ptr_ ) );
        return static_cast<resource_t&>( *resource );
    }


    extern CobaltInstance& CVK_INSTANCE;

}


#endif //!COBALTINSTANCE_H
