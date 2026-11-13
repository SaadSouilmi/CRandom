# CRandom

Simple RNG tools for C projects. I generally use this same base for every project, I am a big proponent
of understanding as much as possible how things work. I use components from different codebases, mainly
for now I use [PCG64](https://www.pcg-random.org/index.html) as the main rng. I also use numpy's implementation
for ziggurat.

**Distributions implemented so far are:**
| Distribution | Time (ns/sample) | Throughput (million samples/s) |
|---|---:|---:|
| Uniform 64-bit integer | 1.10 | ≈909 |
| Uniform 32-bit integer | 1.10 | ≈909 |
| Uniform (0, 1) | 1.13 | ≈885 |
| Standard normal | 1.62 | ≈617 |
| Exponential (rate = 1) | 1.75 | ≈571 |

I also have some ideas I would like to explore, such as SIMDing RandomNormal first of all for ziggurat to see
how SIMD friendly it is (I have a hunch) and through something very SIMDable like BoxMuller and compare throughput.
I also plan to venture into GPU programming at some point. But keeping it simple for now.
