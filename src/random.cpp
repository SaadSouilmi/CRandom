/* PCG XSL-RR 128/64 adapted from pcg-c, Copyright 2014-2017 Melissa O'Neill.
   Apache-2.0.
   sampling zig follows NumPy. Copyright 2005-2025 NumPy Developers; BSD-3-Clause,
   see licenses/numpy.txt.*/
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define ZIG_LAYERS 256
typedef struct zig_layer { U64 K; F64 W; } zig_layer;
typedef struct zig_density
{
	F64 (*F)(F64);
	F64 (*FInv)(F64);
	F64 (*Tail)(F64);
} zig_density;

local_persist zig_layer ZigNor[ZIG_LAYERS] __attribute__((aligned(64)));
local_persist F64 ZigNorPdf[ZIG_LAYERS], ZigNorR, ZigNorInvR;
local_persist zig_layer ZigExp[ZIG_LAYERS] __attribute__((aligned(64)));
local_persist F64 ZigExpPdf[ZIG_LAYERS], ZigExpR;
local_persist pthread_once_t GlobalZigOnce = PTHREAD_ONCE_INIT;
read_only F64 RandomSign[2] = {1, -1};

internal F64 NormalF(F64 X) { return exp(-0.5*X*X); }
internal F64 NormalFInv(F64 Y) { return sqrt(-2*log(Y)); }
internal F64 NormalTail(F64 R) { return sqrt(0.5*PI)*erfc(R*0.7071067811865475); }
internal F64 ExponentialF(F64 X) { return exp(-X); }
internal F64 ExponentialFInv(F64 Y) { return -log(Y); }
internal F64 ExponentialTail(F64 R) { return exp(-R); }

internal F64
ZigResidual(zig_density D, F64 R)
{
	F64 V = R*D.F(R) + D.Tail(R), X = R;
	for (I32 J = ZIG_LAYERS - 2; J > 0; --J)
	{
		F64 A = V/X + D.F(X);
		if (A >= 1) { return 1; }
		X = D.FInv(A);
	}
	return V/X + D.F(X) - 1;
}

internal F64
ZigSolveR(zig_density D, F64 Lo, F64 Hi)
{
	for (I32 J = 0; J < 64; ++J)
	{
		F64 Mid = 0.5*(Lo + Hi);
		if (ZigResidual(D, Mid) > 0) { Lo = Mid; } else { Hi = Mid; }
	}
	return 0.5*(Lo + Hi);
}

internal void
ZigBuild(zig_density D, F64 R, F64 M, zig_layer *Layers, F64 *Pdf)
{
	F64 X[ZIG_LAYERS], V = R*D.F(R) + D.Tail(R);
	X[ZIG_LAYERS - 1] = R;
	for (I32 J = ZIG_LAYERS - 2; J > 0; --J)
	{
		X[J] = D.FInv(V/X[J + 1] + D.F(X[J + 1]));
	}
	X[0] = V/D.F(R);
	Layers[0] = (zig_layer){(U64)(R/X[0]*M), X[0]/M};
	Pdf[0] = 1;
	Layers[1] = (zig_layer){0, X[1]/M};
	Pdf[1] = D.F(X[1]);
	for (I32 J = 2; J < ZIG_LAYERS; ++J)
	{
		Layers[J] = (zig_layer){(U64)(X[J - 1]/X[J]*M), X[J]/M};
		Pdf[J] = D.F(X[J]);
	}
}

internal void
ZigInit(void)
{
	zig_density Normal = {NormalF, NormalFInv, NormalTail};
	ZigNorR = ZigSolveR(Normal, 3, 4);
	ZigNorInvR = 1/ZigNorR;
	ZigBuild(Normal, ZigNorR, 0x1p52, ZigNor, ZigNorPdf);

	zig_density Exponential = {ExponentialF, ExponentialFInv, ExponentialTail};
	ZigExpR = ZigSolveR(Exponential, 7, 8.5);
	ZigBuild(Exponential, ZigExpR, 0x1p53, ZigExp, ZigExpPdf);
}

internal void 
RandomInit(void) { pthread_once(&GlobalZigOnce, ZigInit); }

internal void
RandomSeed(rng *R, U64 Seed, U64 Stream)
{
	RandomInit();
	*R = (rng){.Stream = ((U128)Stream << 1) | 1};
	RandomU64(R); R->State += Seed; RandomU64(R);
}

internal U64
RandomU64(rng *R)
{
	U128 Multiplier = ((U128)2549297995355413924ULL << 64) | 4865540595714422341ULL;
	R->State = R->State*Multiplier + R->Stream;
	U64 Bits = (U64)(R->State >> 64) ^ (U64)R->State;
	U32 Rotation = (U32)(R->State >> 122);
	return (Bits >> Rotation) | (Bits << ((-Rotation) & 63));
}

internal U32 
RandomU32(rng *R) { return (U32)(RandomU64(R) >> 32); }

internal F64 
RandomUniform(rng *R) { return (RandomU64(R) >> 11)*0x1p-53; }

internal F64 
RandomUniformClosed(rng *R) { return ((F64)(RandomU64(R) >> 12) + 0.5)*0x1p-52; }

internal F64
RandomNormal(rng *Rng)
{
	for (;;)
	{
		U64 R = RandomU64(Rng);
		I32 Idx = R & 0xFF, Sign = (R >> 8) & 1;
		U64 RAbs = (R >> 9) & 0x000FFFFFFFFFFFFFULL;
		zig_layer L = ZigNor[Idx];
		F64 X = RAbs*L.W*RandomSign[Sign];
		if (RAbs < L.K) { return X; }
		if (Idx == 0)
		{
			for (;;)
			{
				F64 XX = -ZigNorInvR*log1p(-RandomUniform(Rng));
				F64 YY = -log1p(-RandomUniform(Rng));
				if (YY + YY > XX*XX)
				{
					return ((RAbs >> 8) & 1) ? -(ZigNorR + XX) : ZigNorR + XX;
				}
			}
		}
		else
		{
			F64 Pdf = exp(-0.5*X*X);
			F64 Sample = (ZigNorPdf[Idx - 1] - ZigNorPdf[Idx])*RandomUniform(Rng) + ZigNorPdf[Idx];
			if (Sample < Pdf) { return X; }
		}
	}
}

internal F64
RandomExponential(rng *Rng)
{
	for (;;)
	{
		U64 Ri = RandomU64(Rng) >> 3;
		U8 Idx = Ri & 0xFF;
		Ri >>= 8;
		zig_layer L = ZigExp[Idx];
		F64 X = Ri*L.W;
		if (Ri < L.K) { return X; }
		if (Idx == 0) { return ZigExpR - log1p(-RandomUniform(Rng)); }
		F64 Pdf = exp(-X);
		F64 Sample = (ZigExpPdf[Idx - 1] - ZigExpPdf[Idx])*RandomUniform(Rng) + ZigExpPdf[Idx];
		if (Sample < Pdf) { return X; }
	}
}


internal U64 
RandomCategorical(rng *R, const categorical* Cat)
{
	U64 J = RandomInt(R, Cat->NCat);
	Alias* A = &Cat->AliasTable[J];
	if (A->Single) { return A->C1; }
	return (RandomUniform(R) < A->P) ? A->C1 : A->C2;
}

// Sample a uniform in [0, N) via rejection sampling
// TODO: Improve this could def be more efficient
internal U64 
RandomInt(rng *R, U64 N)
{
	U64 Threshold = -N%N; // This is a trick to get UINT64_MAX % N
	for (;;) {
		U64 U = RandomU64(R);
		if (U < Threshold) { continue; }
		return U % N;
	}
}

internal I32 
MakeCategorical(U64 NCat, const F64 *p, categorical* Cat)
{
	F64 Sum = 0;
	for (U64 J = 0; J < NCat; ++J) { Sum += p[J]; }
	F64* q = (F64*)malloc(sizeof(F64)*NCat);
	U64* Small = (U64*)malloc(sizeof(I32)*NCat);
	U64* Large = (U64*)malloc(sizeof(I32)*NCat);
	U64 NSmall = 0, NLarge = 0;
	if (q == NULL || Small == NULL || Large == NULL) { return 1; }
	for (U64 J = 0; J < NCat; ++J) {
		q[J] = (p[J]/Sum) * (F64)NCat;
		if (q[J] < 1) { Small[NSmall++] = J; }
		else { Large[NLarge++] = J; }
	}

	Cat->NCat = NCat;
	Cat->AliasTable = (Alias*)malloc(sizeof(Alias)*NCat);
	if (Cat->AliasTable == NULL) { return 1; }
	
	while (NSmall > 0 && NLarge > 0)
	{
		U64 S = Small[--NSmall];
		U64 L = Large[--NLarge];

		Alias* A = &Cat->AliasTable[S];
		A->C1 = S;
		A->C2 = L;
		A->Single = 0;
		A->P = q[S];

		q[L] -= 1.0 - q[S];

		if (q[L] < 1) { Small[NSmall++] = L; }
		else { Large[NLarge++] = L; }
	}

	while (NSmall > 0 || NLarge >0)
	{
		U64 J = (NSmall > 0) ? Small[--NSmall] : Large[--NLarge];

		Alias* A = &Cat->AliasTable[J];
		A->C1 = J;
		A->C2 = UINT64_MAX;
		A->Single = 1;
		A->P = 1.0;
	}
	return 0;
}
