#ifndef INITWIZARD_H
#define INITWIZARD_H

#include <__meta/index_of.h>

#include <cassert>
#include <tuple>
#include <optional>


namespace cobalt
{
    template <typename... invariants_t>
    struct InitWizard
    {
        using args_tuple_t = std::tuple<invariants_t...>;

        template <typename... features_t>
        class WithFeatures
        {
        public:
            using features_tuple_t = std::tuple<std::optional<features_t>...>;

            /**
             * Constructor that initializes the InitWizard with required arguments.
             * @param args
             */
            constexpr explicit WithFeatures( invariants_t&&... args );

            /**
             * Extend with a feature
             * @tparam feature_t
             * @tparam args_t
             * @param feature_args
             * @return
             */
            template <typename feature_t, typename... args_t>
            auto& with( args_t&&... feature_args ) &;

            /**
             * Extend with a feature
             * @tparam feature_t
             * @tparam args_t
             * @param feature_args
             * @return
             */
            template <typename feature_t, typename... args_t>
            auto&& with( args_t&&... feature_args ) &&;

            /**
             * Access required argument by type.
             * @tparam invariant_t
             * @return
             */
            template <typename invariant_t>
            constexpr invariant_t& get( );

            /**
             * Access required argument by type.
             * @tparam invariant_t
             * @return
             */
            template <typename invariant_t>
            constexpr invariant_t& get( invariant_t&& );

            /**
             * Access feature by type.
             * @tparam feature_t
             * @return
             */
            template <typename feature_t>
            constexpr feature_t& feat( );


            /**
             * Move feature by type.
             * @tparam feature_t
             * @return
             */
            template <typename feature_t>
            constexpr feature_t&& steal( );


            /**
             * Check if the feature was provided by type.
             * @tparam feature_t
             * @return
             */
            template <typename feature_t>
            constexpr bool has( ) const;

        private:
            args_tuple_t args_;
            features_tuple_t features_{};

        };

    };


    template <typename... invariants_t>
    template <typename... features_t>
    constexpr InitWizard<invariants_t...>::WithFeatures<features_t...>::WithFeatures( invariants_t&&... args )
        : args_{ std::forward<invariants_t>( args )... } { }


    template <typename... invariants_t>
    template <typename... features_t>
    template <typename feature_t, typename... args_t>
    auto& InitWizard<invariants_t...>::WithFeatures<features_t...>::with( args_t&&... feature_args ) &
    {
        meta::element_of_type<std::optional<feature_t>>( features_ ).emplace( std::forward<args_t>( feature_args )... );
        return *this;
    }


    template <typename ... invariants_t>
    template <typename ... features_t>
    template <typename feature_t, typename ... args_t>
    auto&& InitWizard<invariants_t...>::WithFeatures<features_t...>::with( args_t&&... feature_args ) &&
    {
        meta::element_of_type<std::optional<feature_t>>( features_ ).emplace( std::forward<args_t>( feature_args )... );
        return std::move( *this );
    }


    template <typename... invariants_t>
    template <typename... features_t>
    template <typename invariant_t>
    constexpr invariant_t& InitWizard<invariants_t...>::WithFeatures<features_t...>::get( )
    {
        return meta::element_of_type<invariant_t>( args_ );
    }


    template <typename ... invariants_t>
    template <typename ... features_t>
    template <typename invariant_t>
    constexpr invariant_t& InitWizard<invariants_t...>::WithFeatures<features_t...>::get( invariant_t&& )
    {
        return get<invariant_t>( );
    }


    template <typename... invariants_t>
    template <typename... features_t>
    template <typename feature_t>
    constexpr feature_t& InitWizard<invariants_t...>::WithFeatures<features_t...>::feat( )
    {
        assert( has<feature_t>( ) && "feature not found!" );
        return meta::element_of_type<std::optional<feature_t>>( features_ ).value( );
    }


    template <typename ... invariants_t>
    template <typename ... features_t>
    template <typename feature_t>
    constexpr feature_t&& InitWizard<invariants_t...>::WithFeatures<features_t...>::steal( )
    {
        return std::move( feat<feature_t>( ) );
    }


    template <typename... invariants_t>
    template <typename... features_t>
    template <typename feature_t>
    constexpr bool InitWizard<invariants_t...>::WithFeatures<features_t...>::has( ) const
    {
        return meta::element_of_type<std::optional<feature_t>>( features_ ).has_value( );
    }

}


#endif //!INITWIZARD_H
