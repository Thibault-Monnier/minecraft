// PerlinNoise.h - v0.5 - perlin noise
// public domain single-file C implementation by Sean Barrett
//
// Documentation:
//
// float  stb_perlin_noise3( float x,
//                           float y,
//                           float z,
//                           int   x_wrap=0,
//                           int   y_wrap=0,
//                           int   z_wrap=0)
//
// This function computes a random value at the coordinate (x,y,z).
// Adjacent random values are continuous but the noise fluctuates
// its randomness with period 1, i.e. takes on wholly unrelated values
// at integer points. Specifically, this implements Ken Perlin's
// revised noise function from 2002.
//
// The "wrap" parameters can be used to create wraparound noise that
// wraps at powers of two. The numbers MUST be powers of two. Specify
// 0 to mean "don't care". (The noise always wraps every 256 due
// details of the implementation, even if you ask for larger or no
// wrapping.)
//
// float  stb_perlin_noise3_seed( float x,
//                                float y,
//                                float z,
//                                int   x_wrap=0,
//                                int   y_wrap=0,
//                                int   z_wrap=0,
//                                int   seed)
//
// As above, but 'seed' selects from multiple different variations of the
// noise function. The current implementation only uses the bottom 8 bits
// of 'seed', but possibly in the future more bits will be used.
//
//
// Fractal Noise:
//
// Three common fractal noise functions are included, which produce
// a wide variety of nice effects depending on the parameters
// provided. Note that each function will call stb_perlin_noise3
// 'octaves' times, so this parameter will affect runtime.
//
// float stb_perlin_ridge_noise3(float x, float y, float z,
//                               float lacunarity, float gain, float offset, int octaves)
//
// float stb_perlin_fbm_noise3(float x, float y, float z,
//                             float lacunarity, float gain, int octaves)
//
// float stb_perlin_turbulence_noise3(float x, float y, float z,
//                                    float lacunarity, float gain, int octaves)
//
// Typical values to start playing with:
//     octaves    =   6     -- number of "octaves" of noise3() to sum
//     lacunarity = ~ 2.0   -- spacing between successive octaves (use exactly 2.0 for wrapping output)
//     gain       =   0.5   -- relative weighting applied to each successive octave
//     offset     =   1.0?  -- used to invert the ridges, may need to be larger, not sure
//

#pragma once

extern "C" {

float stb_perlin_noise3(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap);
float stb_perlin_noise3_seed(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap,
                             int seed);
float stb_perlin_ridge_noise3(float x, float y, float z, float lacunarity, float gain, float offset,
                              int octaves);
float stb_perlin_fbm_noise3(float x, float y, float z, float lacunarity, float gain, int octaves);
float stb_perlin_turbulence_noise3(float x, float y, float z, float lacunarity, float gain,
                                   int octaves);
float stb_perlin_noise3_wrap_nonpow2(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap,
                                     unsigned char seed);

}  // extern "C"
