#ifndef RANDOM_H
#define RANDOM_H
#include "base.h"

#if !defined(__SIZEOF_INT128__)
#error "PCG64 requires compiler support for unsigned 128-bit integers."
#endif

typedef unsigned __int128 U128;
typedef struct rng { U128 State, Stream; } rng;

// PCG XSL-RR 128/64; Seed and Stream use the low 64 bits of PCG's seed inputs.
internal void RandomSeed(rng *R, U64 Seed, U64 Stream);

internal U64 RandomU64(rng *R);
internal U32 RandomU32(rng *R);
internal F64 RandomUniform(rng *R); // (0,1), 52 random bits; HQE/log(U).
internal F64 RandomUniformClosedOpen(rng *R); // [0,1), 53 bits; Ziggurat/log1p(-U).
internal F64 RandomNormal(rng *R);
internal F64 RandomExponential(rng *R);

#endif // RANDOM_H
