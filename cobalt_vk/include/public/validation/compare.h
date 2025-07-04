#ifndef COMPARE_H
#define COMPARE_H

#include <functional>
#include <vector>


namespace cobalt_vk::validation
{
    template <typename vk_property_t>
    bool compare_support_containers( const std::vector<const char*>& required, const std::vector<vk_property_t>& available,
                                     const std::function<const char*( const vk_property_t& )>& getName )
    {
        // Refactor to use the erase method. return boolean but pass vector by reference to keep the not found items.
        for ( const auto& requiredProperty : required )
        {
            bool found{ false };
            for ( const auto& availableProperty : available )
            {
                if ( std::strcmp( requiredProperty, getName( availableProperty ) ) == 0 )
                {
                    found = true;
                    break;
                }
            }

            if ( !found )
            {
                return false;
            }
        }
        return true;
    }

}


#endif //!COMPARE_H
