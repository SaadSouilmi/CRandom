# CRandom

<p align="center">
  <a href="https://saadsouilmi.github.io/CRandom/">
    <img src="assets/blog-badge.svg" width="88" height="28" alt="blogpost">
  </a>
</p>

Simple RNG tools for C projects. I generally use this same base for every project, I am a big proponent
of understanding as much as possible how things work. I use components from different codebases, mainly
for now I use [PCG64](https://www.pcg-random.org/index.html) as the main rng. I also use numpy's implementation
for ziggurat.

**Distributions implemented so far are:**
| Distribution | Time (mean ± SD, ns/sample) | Throughput (million samples/s) |
|---|---:|---:|
| Uniform 64-bit integer | 1.05 ± 0.03 | 952 |
| Uniform 32-bit integer | 1.05 ± 0.02 | 952 |
| Uniform [0, 1) | 1.05 ± 0.03 | 952 |
| Standard normal | 3.41 ± 0.18 | 293 |
| Exponential (rate = 1) | 3.43 ± 0.22 | 292 |
| Categorical | 5.62 ± 0.14 | 178 |

Throughput was measured under `-O2` and `-march=native`. The reported SD is the sample standard deviation across 1,000 batches.


