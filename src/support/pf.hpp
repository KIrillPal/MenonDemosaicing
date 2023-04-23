#pragma once

// define 'size_t pf' - variable shows the first red or blue pixel position in x-th row
#ifdef RGGB
#define SIZE_T_PF(x) size_t pf = x & 1;
#else
// For GRBG
#define SIZE_T_PF(x) size_t pf = (~x) & 1;
#endif
