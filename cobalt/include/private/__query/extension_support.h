#ifndef EXTENSION_SUPPORT_H
#define EXTENSION_SUPPORT_H

#include <vector>


namespace cobalt::query
{
    [[nodiscard]] bool check_instance_extensions_support( std::vector<char const*>& extensions );
    [[nodiscard]] bool check_instance_extensions_support( std::vector<char const*> const& extensions );

    [[nodiscard]] bool check_validation_layers_support( std::vector<char const*>& layers );
    [[nodiscard]] bool check_validation_layers_support( std::vector<char const*> const& layers );

}


#endif //!EXTENSION_SUPPORT_H
