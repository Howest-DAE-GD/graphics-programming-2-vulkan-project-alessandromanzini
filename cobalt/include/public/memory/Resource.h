#ifndef RESOURCE_H
#define RESOURCE_H


namespace cobalt::memory
{
    class Resource
    {
    public:
        Resource( )          = default;
        virtual ~Resource( ) = default;

        Resource( Resource const& )                = delete;
        Resource( Resource&& ) noexcept            = delete;
        Resource& operator=( Resource const& )     = delete;
        Resource& operator=( Resource&& ) noexcept = delete;

    };

}


#endif //!RESOURCE_H
