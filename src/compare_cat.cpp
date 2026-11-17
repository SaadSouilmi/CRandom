#include <stdio.h>

#include "base.h"
#include "profile.h"
#include "random.h"

#include "profile.cpp"
#include "random.cpp"

internal U64
Bisect(F64 U, const F64 *CDF, U64 N) // Binary search the index J s.t U < CDF[J]
{
	U64 L = 0, R = N;
	while (L < R)
	{
		U64 Mid = L + (R-L)/2;
		if (CDF[Mid] <= U) { L = Mid + 1; }
		else { R = Mid; }
	}
	return L;
}

typedef struct categorical_bisect {
	U64 NCat; F64* CDF;
} categorical_bisect;

internal I32
MakeCatBisect(categorical_bisect* Cat, const F64* p, U64 N)
{
	Cat->NCat = N;
	Cat->CDF = (F64*)malloc(sizeof(F64)*N);
	if (!Cat->CDF) { return 1; }
	F64 Sum = 0;
	for (U64 J = 0; J < N; ++J) { Sum += p[J]; Cat->CDF[J] = Sum; }
	for (U64 J = 0; J < N; ++J) { Cat->CDF[J] /= Sum; }
	Cat->CDF[N-1] = 1;
	return 0;
}

internal U64
RandomCat(rng* Rng, const categorical_bisect* Cat)
{
	F64 U = RandomUniform(Rng);
	return Bisect(U, Cat->CDF, Cat->NCat);
}

I32
main()
{
	#define Warmup   1000
	#define NBatches 1000
	#define NSamples 10000

	rng WeightRng = {};
	RandomSeed(&WeightRng, 1337, 1);
	GetTSCCalibration();

	const U64 NList[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 4096, 16384, 65536, 262144, 1048576};
	const U64 NListCount = sizeof(NList)/sizeof(NList[0]);
	for (U64 K = 0; K < NListCount; ++K)
	{
		U64 N = NList[K];
		F64* p = (F64*)malloc(sizeof(F64)*N);
		if (!p) { return 1; }
		for (U64 J = 0; J < N; ++J) { p[J] = 0.5 + RandomUniform(&WeightRng); }

		categorical_bisect CatBisect = {};
		categorical CatAlias = {};
		if (MakeCatBisect(&CatBisect, p, N) || MakeCategorical(N, p, &CatAlias))
		{
			free(p);
			free(CatBisect.CDF);
			free(CatAlias.AliasTable);
			return 1;
		}

		rng BisectRng = {}, AliasRng = {};
		RandomSeed(&BisectRng, 1337, 0);
		RandomSeed(&AliasRng, 1337, 0);

		profile_stats BisectProfile = {};
		volatile U64 x;
		for (I32 J = 0; J < Warmup; ++J) { x = RandomCat(&BisectRng, &CatBisect); }
		for (I32 B = 0; B < NBatches; ++B) {
			U64 Start = ReadTSC();
			for (I32 J = 0; J < NSamples; ++J) { x = RandomCat(&BisectRng, &CatBisect); }
			U64 Elapsed = ReadTSC() - Start;
			IncrementStats(&BisectProfile, Elapsed);
		}

		profile_stats AliasProfile = {};
		for (I32 J = 0; J < Warmup; ++J) { x = RandomCategorical(&AliasRng, &CatAlias); }
		for (I32 B = 0; B < NBatches; ++B) {
			U64 Start = ReadTSC();
			for (I32 J = 0; J < NSamples; ++J) { x = RandomCategorical(&AliasRng, &CatAlias); }
			U64 Elapsed = ReadTSC() - Start;
			IncrementStats(&AliasProfile, Elapsed);
		}

		F64 BisectMean = TSCToWall(BisectProfile.AvgTicks, TimeUnit_Ns)/NSamples;
		F64 BisectSD = TSCToWall(BisectProfile.StdTicks, TimeUnit_Ns)/NSamples;
		F64 AliasMean = TSCToWall(AliasProfile.AvgTicks, TimeUnit_Ns)/NSamples;
		F64 AliasSD = TSCToWall(AliasProfile.StdTicks, TimeUnit_Ns)/NSamples;
		printf("N = %llu | Bisect: %.2f ± %.2f ns / Sample | Alias: %.2f ± %.2f ns / Sample | Speedup: %.2fx\n",
			(unsigned long long)N, BisectMean, BisectSD, AliasMean, AliasSD, BisectMean/AliasMean);
		fflush(stdout);

		free(p);
		free(CatBisect.CDF);
		free(CatAlias.AliasTable);
	}
	return 0;
}
