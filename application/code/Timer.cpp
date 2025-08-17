#include "Timer.h"

#include <assert.h>
#include <cfloat>
#include <iostream>
#include <numeric>

#include <fstream>
#include <algorithm>


namespace dae
{
    Timer::Timer( float const elapsed_upper_bound ) noexcept
        : elapsed_upper_bound_{ elapsed_upper_bound }
        , force_elapsed_upper_bound_{ elapsed_upper_bound != 0.f } { }


    void Timer::start_benchmark( uint32_t const num_frames )
    {
        assert( not benchmark_active_ && "Timer::start_benchmark: benchmark already active!" );

        benchmark_active_ = true;

        benchmark_frames_        = num_frames;
        benchmark_current_frame_ = 0;

        benchmarks_.clear( );
        benchmarks_.resize( benchmark_frames_ );

        std::cout << "**BENCHMARK STARTED**\n";
    }


    void Timer::reset( ) noexcept
    {
        auto const now = clock_t::now( );
        base_time_     = now;
        previous_time_ = now;
        stop_time_     = time_point_t{};
        fps_timer_     = 0.f;
        fps_count_     = 0;
        is_stopped_    = false;
    }


    void Timer::update( )
    {
        if ( is_stopped_ )
        {
            fps_ = 0;
            elapsed_time_ = 0.f;
            total_time_ = std::chrono::duration<float>( ( stop_time_ - pause_time_ ) - base_time_.time_since_epoch( ) ).count( );
            return;
        }

        auto const now = clock_t::now( );
        elapsed_time_  = std::chrono::duration<float>( now - previous_time_ ).count( );
        previous_time_ = now;
        if ( force_elapsed_upper_bound_ && elapsed_time_ > elapsed_upper_bound_ )
        {
            elapsed_time_ = elapsed_upper_bound_;
        }

        total_time_ += elapsed_time_;

        // FPS LOGIC
        fps_timer_ += elapsed_time_;
        ++fps_count_;
        if ( fps_timer_ >= 1.f )
        {
            // register fps
            dfps_ = static_cast<double>( fps_count_ / fps_timer_ );
            fps_  = fps_count_;

            // reset fps counters
            fps_count_ = 0;
            fps_timer_ = 0.f;

            if ( benchmark_active_ )
            {
                benchmarks_[benchmark_current_frame_++] = static_cast<float>( dfps_ );

                if ( benchmark_current_frame_ >= benchmark_frames_ )
                {
                    benchmark_active_ = false;

                    auto const [low, high] = std::ranges::minmax_element( benchmarks_ );

                    auto const avg = std::accumulate( benchmarks_.begin( ), benchmarks_.end( ), 0.f )
                                     / static_cast<float>( benchmark_frames_ );

                    save_benchmarks( std::cout, *low, *high, avg, benchmark_frames_ );

                    std::ofstream file_stream{ "benchmark.txt" };
                    save_benchmarks( file_stream, *low, *high, avg, benchmark_frames_ );
                    file_stream.close( );
                }
            }
        }
    }


    void Timer::start( ) noexcept
    {
        auto const start_time = clock_t::now( );
        if ( is_stopped_ )
        {
            pause_time_ += ( start_time - stop_time_ );

            previous_time_ = start_time;
            stop_time_     = time_point_t{};
            is_stopped_    = false;
        }
    }


    void Timer::stop( ) noexcept
    {
        if ( not is_stopped_ )
        {
            stop_time_  = clock_t::now( );
            is_stopped_ = true;
        }
    }


    uint32_t Timer::fps( ) const noexcept
    {
        return fps_;
    }


    double Timer::dfps( ) const noexcept
    {
        return dfps_;
    }


    float Timer::total( ) const noexcept
    {
        return total_time_;
    }


    float Timer::elapsed( ) const noexcept
    {
        return elapsed_time_;
    }


    bool Timer::is_running( ) const noexcept
    {
        return !is_stopped_;
    }


    void Timer::save_benchmarks( std::ostream& stream, float const low, float const high, float const avg,
                                 uint32_t const num_frames )
    {
        std::cout << "**BENCHMARK RESULTS**\n";
        stream << "-> FRAMES = " << num_frames << std::endl;
        stream << "-> HIGH   = " << high << std::endl;
        stream << "-> LOW    = " << low << std::endl;
        stream << "-> AVG    = " << avg << std::endl;
    }

}
