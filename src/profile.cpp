#if defined(__i386__) || defined(__x86_64__)
#include <x86intrin.h>
#elif defined(__aarch64__)
#include <arm_acle.h>
#endif
#include <math.h>
#include <stdio.h>
#include <time.h>

internal U64
ReadTSC()
{
#if defined(__i386__) || defined(__x86_64__)
	U32 Aux;
	U64 Ticks = __rdtscp(&Aux);
	_mm_lfence();
	return Ticks;
#elif defined(__aarch64__)
	// Keep the counter read ordered with the measured work.
	__isb(15);
	U64 Ticks = __arm_rsr64("cntvct_el0");
	__isb(15);
	return Ticks;
#else
#error "Unsupported profiler architecture"
#endif
}

internal U64
ReadOSTime()
{
	struct timespec T;
#if defined(CLOCK_MONOTONIC_RAW)
	clock_gettime(CLOCK_MONOTONIC_RAW, &T);
#else
	clock_gettime(CLOCK_MONOTONIC, &T);
#endif
	return (U64)T.tv_sec * 1000000000ULL + (U64)T.tv_nsec;
}

internal tsc_calibration
GetTSCCalibration()
{
	local_persist tsc_calibration Calibration = {0, 0};

	if (Calibration.TSCTicks != 0.0) { return Calibration; }

	U64 OSStart  = ReadOSTime();
	U64 TSCStart = ReadTSC();
	
	U64 OSEnd;
	do 
	{
		OSEnd = ReadOSTime();
	} while((OSEnd - OSStart) < 1000000000ULL);

	Calibration.TSCTicks = ReadTSC() - TSCStart;
	Calibration.OSNanos  = OSEnd - OSStart;
	
	return Calibration;
}

internal F64
TSCToWall(U64 ElapsedTicks, time_unit Unit)
{
	tsc_calibration Calibration = GetTSCCalibration();
	F64 Elapsed = ((F64)ElapsedTicks *
						(F64)Calibration.OSNanos /
						(F64)Calibration.TSCTicks);

	switch (Unit)
	{
		case TimeUnit_Sec:
		{
			return Elapsed * 1e-9;
		} break;
		case TimeUnit_Ms:
		{
			return Elapsed * 1e-6;
		} break;
		case TimeUnit_Us:
		{
			return Elapsed * 1e-3;
		} break;
		case TimeUnit_Ns:
		{
			return Elapsed;
		} break;
	}
	return -1;
}


internal void
IncrementStats(profile_stats *Stats, U64 Elapsed)
{
	Stats->Count++;
	F64 Delta = (F64)Elapsed - Stats->AvgTicks;
	Stats->AvgTicks += Delta / (F64)Stats->Count;

	F64 Delta2 = (F64)Elapsed - Stats->AvgTicks;
	Stats->M2Ticks += Delta * Delta2;

	if (Stats->Count > 1)
	{
		Stats->StdTicks = sqrt(Stats->M2Ticks / (F64)(Stats->Count - 1));
		Stats->MinTicks = Min(Stats->MinTicks, Elapsed);
		Stats->MaxTicks = Max(Stats->MaxTicks, Elapsed);
	}
	else
	{
		Stats->MinTicks = Elapsed;
		Stats->MaxTicks = Elapsed;
	}

};
