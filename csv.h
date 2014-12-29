#pragma once

void SaveCSV( char const* name, unsigned sampleNum );
void SaveCSV( char const* name, float* lutData, unsigned lutWidth, unsigned lutHeight, unsigned elemOffset );