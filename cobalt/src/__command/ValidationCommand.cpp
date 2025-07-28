#include <__command/ValidationCommand.h>


namespace cobalt::exe
{
    void ValidationCommand::execute( )
    {
        valid_ = validate( );
    }


    bool ValidationCommand::is_valid( ) const
    {
        return valid_;
    }

}
