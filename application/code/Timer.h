#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <cstdint>
#include <vector>


class Timer final
{
    using clock_t      = std::chrono::steady_clock;
    using time_point_t = std::chrono::time_point<clock_t>;

public:
    explicit Timer( float elapsed_upper_bound = 0.f ) noexcept;
    ~Timer( ) noexcept = default;

    Timer( const Timer& )                = delete;
    Timer( Timer&& ) noexcept            = delete;
    Timer& operator=( const Timer& )     = delete;
    Timer& operator=( Timer&& ) noexcept = delete;

    void start_benchmark( uint32_t num_frames );

    void reset( ) noexcept;
    void update( );

    void start( ) noexcept;
    void stop( ) noexcept;

    [[nodiscard]] uint32_t fps( ) const noexcept;
    [[nodiscard]] float dfps( ) const noexcept;

    [[nodiscard]] float total( ) const noexcept;
    [[nodiscard]] float elapsed( ) const noexcept;
    [[nodiscard]] bool is_running( ) const noexcept;

private:
    float const elapsed_upper_bound_{ .03f };
    bool const force_elapsed_upper_bound_{ false };

    time_point_t base_time_;
    time_point_t pause_time_{};
    time_point_t stop_time_{};

    time_point_t previous_time_{};
    time_point_t current_time_{};

    uint32_t fps_{};
    double dfps_{};
    uint32_t fps_count_{};

    bool is_stopped_{ true };

    float total_time_{};
    float elapsed_time_{};
    float fps_timer_{};

    bool benchmark_active_{ false };
    uint32_t benchmark_frames_{ 0 };
    uint32_t benchmark_current_frame_{ 0 };
    std::vector<float> benchmarks_{};

    static void save_benchmarks( std::ostream&, float low, float high, float avg, uint32_t num_frames );

};


#endif //!TIMER_H
