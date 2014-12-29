#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include "dds.h"
#include "csv.h"

uint32_t ReverseBits( uint32_t v )
{
    v = ( ( v >> 1 ) & 0x55555555 ) | ( ( v & 0x55555555 ) << 1 );
    v = ( ( v >> 2 ) & 0x33333333 ) | ( ( v & 0x33333333 ) << 2 );
    v = ( ( v >> 4 ) & 0x0F0F0F0F ) | ( ( v & 0x0F0F0F0F ) << 4 );
    v = ( ( v >> 8 ) & 0x00FF00FF ) | ( ( v & 0x00FF00FF ) << 8 );
    v = (   v >> 16               ) | (   v                << 16 );
    return v;
}

float GSmith( float roughness, float ndotv, float ndotl )
{
    float const m2   = roughness * roughness;
    float const visV = ndotv + sqrt( ndotv * ( ndotv - ndotv * m2 ) + m2 );
    float const visL = ndotl + sqrt( ndotl * ( ndotl - ndotl * m2 ) + m2 );

    return 1.0f / ( visV * visL );
}

int main()
{
    float const MATH_PI         = 3.14159f;
    unsigned const LUT_WIDTH    = 128;
    unsigned const LUT_HEIGHT   = 128;
    unsigned const sampleNum    = 128;

    float lutData[ LUT_WIDTH * LUT_HEIGHT * 4 ];

    for ( unsigned y = 0; y < LUT_HEIGHT; ++y )
    {
        float const ndotv = ( y + 0.5f ) / LUT_WIDTH;

        for ( unsigned x = 0; x < LUT_WIDTH; ++x )
        {
            float const gloss       = ( x + 0.5f ) / LUT_HEIGHT;
            float const roughness   = powf( 1.0f - gloss, 4.0f );

            float const vx = sqrtf( 1.0f - ndotv * ndotv );
            float const vy = 0.0f;
            float const vz = ndotv;

            float scale = 0.0f;
            float bias  = 0.0f;

            
            for ( unsigned i = 0; i < sampleNum; ++i )
            {
                float const e1 = (float) i / sampleNum;
                float const e2 = (float) ( (double) ReverseBits( i ) / (double) 0x100000000LL );

                float const phi         = 2.0f * MATH_PI * e1;
                float const cosPhi      = cosf( phi );
                float const sinPhi      = sinf( phi );
                float const cosTheta    = sqrtf( ( 1.0f - e2 ) / ( 1.0f + ( roughness * roughness - 1.0f ) * e2 ) );
                float const sinTheta    = sqrtf( 1.0f - cosTheta * cosTheta );

                float const hx  = sinTheta * cosf( phi );
                float const hy  = sinTheta * sinf( phi );
                float const hz  = cosTheta;

                float const vdh = vx * hx + vy * hy + vz * hz;
                float const lx  = 2.0f * vdh * hx - vx;
                float const ly  = 2.0f * vdh * hy - vy;
                float const lz  = 2.0f * vdh * hz - vz;

                float const ndotl = std::max( lz,   0.0f );
                float const ndoth = std::max( hz,   0.0f );
                float const vdoth = std::max( vdh,  0.0f );

                if ( ndotl > 0.0f )
                {
                    float const gsmith      = GSmith( roughness, ndotv, ndotl );
                    float const ndotlVisPDF = ndotl * gsmith * ( 4.0f * vdoth / ndoth );
                    float const fc          = powf( 1.0f - vdoth, 5.0f );

                    scale   += ndotlVisPDF * ( 1.0f - fc );
                    bias    += ndotlVisPDF * fc;
                }
            }
            scale /= sampleNum;
            bias  /= sampleNum;

            lutData[ x * 4 + y * LUT_WIDTH * 4 + 0 ] = scale;
            lutData[ x * 4 + y * LUT_WIDTH * 4 + 1 ] = bias;
            lutData[ x * 4 + y * LUT_WIDTH * 4 + 2 ] = 0.0f;
            lutData[ x * 4 + y * LUT_WIDTH * 4 + 3 ] = 0.0f;
        }
    }   

    SaveDDS( "integrateDFG.dds", LUT_WIDTH, LUT_HEIGHT, lutData );
    SaveCSV( "ndotv.csv", LUT_WIDTH );
    SaveCSV( "gloss.csv", LUT_HEIGHT );
    SaveCSV( "scale.csv", lutData, LUT_WIDTH, LUT_HEIGHT, 0 );
    SaveCSV( "bias.csv",  lutData, LUT_WIDTH, LUT_HEIGHT, 1 );

    return 0;
}