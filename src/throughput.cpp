#include <stdio.h>

#include "base.h"
#include "profile.h"
#include "random.h"

#include "profile.cpp"
#include "random.cpp"

I32
main()
{
	rng Rng = {};
	RandomSeed(&Rng, 1337, 0);

	#define Warmup   1000
	#define NBatches 1000
	#define NSamples 10000
	GetTSCCalibration();

	profile_stats U64Profile = {};
	{
		volatile U64 x;
		for (I32 J = 0; J < Warmup; ++J) {
			x = RandomU64(&Rng);
		}

		for (I32 B = 0; B < NBatches; ++B) {
			U64 Start = ReadTSC();
			for (I32 J = 0; J < NSamples; ++J) {
				x = RandomU64(&Rng);
			}
			U64 Elapsed = ReadTSC() - Start;
			IncrementStats(&U64Profile, Elapsed);
		}
	}

	profile_stats U32Profile = {};
	{
		volatile U32 x;
		for (I32 J = 0; J < Warmup; ++J) {
			x = RandomU32(&Rng);
		}

		for (I32 B = 0; B < NBatches; ++B) {
			U64 Start = ReadTSC();
			for (I32 J = 0; J < NSamples; ++J) {
				x = RandomU32(&Rng);
			}
			U64 Elapsed = ReadTSC() - Start;
			IncrementStats(&U32Profile, Elapsed);
		}
	}

	profile_stats UnifProfile = {};
	{
		volatile F64 x;
		for (I32 J = 0; J < Warmup; ++J) {
			x = RandomUniform(&Rng);
		}

		for (I32 B = 0; B < NBatches; ++B) {
			U64 Start = ReadTSC();
			for (I32 J = 0; J < NSamples; ++J) {
				x = RandomUniform(&Rng);
			}
			U64 Elapsed = ReadTSC() - Start;
			IncrementStats(&UnifProfile, Elapsed);
		}
	}

	profile_stats ZigNorProfile = {};
	{
		volatile F64 x;
		for (I32 J = 0; J < Warmup; ++J) {
			x = RandomNormal(&Rng);
		}

		for (I32 B = 0; B < NBatches; ++B) {
			U64 Start = ReadTSC();
			for (I32 J = 0; J < NSamples; ++J) {
				x = RandomNormal(&Rng);
			}
			U64 Elapsed = ReadTSC() - Start;
			IncrementStats(&ZigNorProfile, Elapsed);
		}
	}

	profile_stats ZigExpProfile = {};
	{
		volatile F64 x;
		for (I32 J = 0; J < Warmup; ++J) {
			x = RandomExponential(&Rng);
		}

		for (I32 B = 0; B < NBatches; ++B) {
			U64 Start = ReadTSC();
			for (I32 J = 0; J < NSamples; ++J) {
				x = RandomExponential(&Rng);
			}
			U64 Elapsed = ReadTSC() - Start;
			IncrementStats(&ZigExpProfile, Elapsed);
		}
	}

	printf("RandomU64 Throughput : %.2f ± %.2f ns / Sample\n",
		TSCToWall(U64Profile.AvgTicks, TimeUnit_Ns)/NSamples,
		TSCToWall(U64Profile.StdTicks, TimeUnit_Ns)/NSamples);

	printf("RandomU32 Throughput : %.2f ± %.2f ns / Sample\n",
		TSCToWall(U32Profile.AvgTicks, TimeUnit_Ns)/NSamples,
		TSCToWall(U32Profile.StdTicks, TimeUnit_Ns)/NSamples);

	printf("RandomUniform Throughput : %.2f ± %.2f ns / Sample\n",
		TSCToWall(UnifProfile.AvgTicks, TimeUnit_Ns)/NSamples,
		TSCToWall(UnifProfile.StdTicks, TimeUnit_Ns)/NSamples);

	printf("RandomNormal Throughput : %.2f ± %.2f ns / Sample\n",
		TSCToWall(ZigNorProfile.AvgTicks, TimeUnit_Ns)/NSamples,
		TSCToWall(ZigNorProfile.StdTicks, TimeUnit_Ns)/NSamples);

	printf("RandomExponential Throughput : %.2f ± %.2f ns / Sample\n",
		TSCToWall(ZigExpProfile.AvgTicks, TimeUnit_Ns)/NSamples,
		TSCToWall(ZigExpProfile.StdTicks, TimeUnit_Ns)/NSamples);
}
