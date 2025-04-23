#ifndef SINGLETON_H
#define SINGLETON_H


namespace cobalt_vk
{
    template <typename derived_t>
    class Singleton
    {
    public:
        virtual ~Singleton( ) = default;

        Singleton( const Singleton& )                = delete;
        Singleton( Singleton&& ) noexcept            = delete;
        Singleton& operator=( const Singleton& )     = delete;
        Singleton& operator=( Singleton&& ) noexcept = delete;

        [[nodiscard]] static derived_t& get_instance( )
        {
            static derived_t instance{};
            return instance;
        }

    protected:
        Singleton( ) = default;

    };

}


#endif //!SINGLETON_H
