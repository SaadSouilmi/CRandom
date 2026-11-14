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
internal F64 RandomUniform(rng *R); // [0,1)
internal F64 RandomUniformClosed(rng *R); // (0,1)
internal F64 RandomNormal(rng *R);
internal F64 RandomExponential(rng *R);

// Sampling discrete distributions using the alias method
// Samples in O(1), Setup in O(N)
typedef struct Alias { U64 C1; U64 C2; F64 P; I32 Single; } Alias;
typedef struct categorical {
	U64 NCat;
	Alias* AliasTable;
} categorical;
internal I32 MakeCategorical(U64 NCat, const F64 *p, categorical* Cat);
internal U64 RandomCategorical(rng *R, const categorical* Cat);
internal U64 RandomInt(rng *R, U64 N);

#endif // RANDOM_H
