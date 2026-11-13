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

Throughput was measured under heavy optimisation i.e: `-O3` and `-march=native`.

### Sampling uniforms
Generating a uniform(0, 1) sounds relatively straighforward, I mean you just have to do the following:
```C
F64 U = (F64)RandomU64(&Rng) / UINT64_MAX;
```
Right, right? Well one could do so but we should be very careful here. Also the cleaner version of that
is:
```C
F64 U = (F64)(RandomU64(&Rng) >> 11)*0x1p-53;
```
We will get into why that is later on. But with how floats are encoded in the [IEEE754](https://en.wikipedia.org/wiki/IEEE_754) standard, we essentially output with uniform probability
one of the following numbers (which are exactly encoded in the [IEEE754](https://en.wikipedia.org/wiki/IEEE_754)
standard)

$$U \sim \{\frac{k}{2^{53}}\qquad,\forall k \in \{0, 2^{-53}, ..., 1-2^{-53}\}\}$$

Which in itself is a reasonnable proxy for a uniform distribution, $2^{-53}\leq 10^{-15}$ not far off 
double precision and floats are inheritly discrete so how good can our uniform distribution be anyways.

One of the problems with that is that for example $\log(1-U) \leq 53$, and this might turn out pretty bad
when sampling certain distributions or rare events.


### Future Plans
I also have some ideas I would like to explore, such as SIMDing RandomNormal first of all for ziggurat to see
how SIMD friendly it is (I have a hunch) and through something very SIMDable like BoxMuller and compare throughput.
I also plan to venture into GPU programming at some point. But keeping it simple for now.
