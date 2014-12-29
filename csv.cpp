#include <stdio.h>
#include "csv.h"

void SaveCSV( char const* name, unsigned sampleNum )
{
    FILE* f = fopen( name, "w" );
    for ( unsigned i = 0; i < sampleNum; ++i )
    {
        float const sample = ( i + 0.5f ) / sampleNum;
        fprintf( f, "%f%s", sample, i + 1 < sampleNum ? ", " : "" );
    }
    fclose( f );
}

void SaveCSV( char const* name, float* lutData, unsigned lutWidth, unsigned lutHeight, unsigned elemOffset )
{
    FILE* f = fopen( name, "w" );
    for ( unsigned y = 0; y < lutHeight; ++y )
    {
        for ( unsigned x = 0; x < lutWidth; ++x )
        {
            fprintf( f, "%f%s", lutData[ x * 4 + y * lutWidth * 4 + elemOffset ], x + 1 < lutWidth ? ", " : "" );
        }

        if ( y + 1 < lutHeight )
        {
            fputs( "\n", f );
        }
    }
    fclose( f );
}