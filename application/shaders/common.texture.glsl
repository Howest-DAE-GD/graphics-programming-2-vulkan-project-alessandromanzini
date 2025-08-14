
// BINDINNGS
layout ( constant_id = 0 ) const uint TEXTURE_COUNT = 1u;
layout ( set = 0, binding = 1 ) uniform sampler shared_sampler;
layout ( set = 0, binding = 2 ) uniform texture2D textures[TEXTURE_COUNT];
