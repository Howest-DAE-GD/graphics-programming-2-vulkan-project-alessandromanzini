#ifndef VALIDATIONCOMMANDBIND_H
#define VALIDATIONCOMMANDBIND_H

#include <__command/ValidationCommand.h>
#include <__meta/function_traits.h>


namespace cobalt::exe
{
    template <typename predicate_fn_t, typename predicate_arg_t>
    class ValidationCommandBind final : public ValidationCommand
    {
    public:
        explicit ValidationCommandBind( predicate_fn_t predicate );
        void set_parameter( predicate_arg_t& param );

    private:
        predicate_fn_t predicate_{};
        predicate_arg_t* param_ptr_{ nullptr };

        [[nodiscard]] bool validate( ) const override;

    };


    template <typename predicate_fn_t, typename predicate_arg_t>
    ValidationCommandBind<predicate_fn_t, predicate_arg_t>::ValidationCommandBind( predicate_fn_t predicate )
        : predicate_{ predicate } { }


    template <typename predicate_fn_t, typename predicate_arg_t>
    void ValidationCommandBind<predicate_fn_t, predicate_arg_t>::set_parameter( predicate_arg_t& param )
    {
        param_ptr_ = &param;
    }


    template <typename predicate_fn_t, typename predicate_arg_t>
    bool ValidationCommandBind<predicate_fn_t, predicate_arg_t>::validate( ) const
    {
        return predicate_( *param_ptr_ );
    }


}


#endif //!VALIDATIONCOMMANDBIND_H
