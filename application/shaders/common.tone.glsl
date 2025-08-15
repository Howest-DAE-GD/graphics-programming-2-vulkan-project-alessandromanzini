// FUNCTIONS
vec3 ACES_film_tone_mapping( in vec3 color )
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return clamp( ( color * ( a * color + b ) ) / ( color * ( c * color + d ) + e ), 0.f, 1.f );
}

vec3 reinhard_tone_mapping( in vec3 color )
{
    return color / ( color + vec3( 1.f ) );
}


vec3 uncharted2_tone_mapping_curve( in vec3 color )
{
    const float a = 0.15f;
    const float b = 0.50f;
    const float c = 0.10f;
    const float d = 0.20f;
    const float e = 0.02f;
    const float f = 0.30f;
    return ( color * ( a * color + c * b ) + d * e ) / ( color * ( a * color + b ) + d * f ) - e / f;
}


vec3 uncharted2_tone_mapping( in vec3 color )
{
    const float W = 11.2f; // White point
    const vec3 curved_color = uncharted2_tone_mapping_curve( color );
    float white_scale = 1.f / uncharted2_tone_mapping_curve( vec3( W ) ).r;
    return clamp( curved_color * white_scale, 0.f, 1.f );
}
