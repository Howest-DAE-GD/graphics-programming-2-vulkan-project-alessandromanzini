// FUNCTIONS
float EV100_from_physical_camera( in float aperture, in float shutter_speed, in float ISO )
{
    return log2( pow( aperture, 2.f ) / shutter_speed * 100.f / ISO );
}

float EV100_from_average_luminance( in float avg_luminance )
{
    const float K = 12.5f;
    return log2( ( avg_luminance * 100.f ) / K );
}

float EV100_to_exposure( in float EV100, in float q )
{
    const float max_luminance = q * pow( 2.f, EV100 );
    return 1.f / max( max_luminance, 0.0001f );
}
