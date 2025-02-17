#ifndef TRIANGLEAPPLICATION_H
#define TRIANGLEAPPLICATION_H

namespace engine
{
    class TriangleApplication
    {
    public:
        TriangleApplication( )           = default;
        ~TriangleApplication( ) noexcept = default;

        TriangleApplication( const TriangleApplication& )                = delete;
        TriangleApplication( TriangleApplication&& ) noexcept            = delete;
        TriangleApplication& operator=( const TriangleApplication& )     = delete;
        TriangleApplication& operator=( TriangleApplication&& ) noexcept = delete;

        void run( );

    private:
        void init_vulkan( );
        void main_loop( );
        void cleanup( );

    };
}

#endif //TRIANGLEAPPLICATION_H
