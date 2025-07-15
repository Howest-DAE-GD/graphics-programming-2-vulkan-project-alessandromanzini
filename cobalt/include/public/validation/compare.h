#ifndef COMPARE_H
#define COMPARE_H

#include <functional>
#include <set>
#include <vector>


namespace cobalt::validation
{
    template <typename vk_property_t, std::invocable<vk_property_t const&> getter_fn_t>
    [[nodiscard]] bool compare_support_containers( std::vector<char const*>& required,
                                                   std::vector<vk_property_t> const& available,
                                                   getter_fn_t get_name )
    {
        std::set<std::string_view> required_unique{ required.begin( ), required.end( ) };
        for ( auto const& prop : available )
        {
            required_unique.erase( std::string_view{ get_name( prop ) } );
        }
        return required_unique.empty( );
    }

}


#endif //!COMPARE_H
