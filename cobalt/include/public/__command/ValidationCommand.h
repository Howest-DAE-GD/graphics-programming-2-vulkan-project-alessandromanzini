#ifndef VALIDATIONCOMMAND_H
#define VALIDATIONCOMMAND_H

#include <__command/Command.h>


namespace cobalt::exe
{
    class ValidationCommand : public Command
    {
    public:
        void execute( ) final;
        [[nodiscard]] bool is_valid( ) const;

    protected:
        [[nodiscard]] virtual bool validate( ) const = 0;

    private:
        bool valid_{ false };

    };

}


#endif //!VALIDATIONCOMMAND_H
