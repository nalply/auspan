# The Math behind simple-asa

simple-asa uses FFT to get the frequency domain of sampled audio data with the
help of the fftw3 library.

1. Take a sequence: $s$ PCM samples
1. Stop if there were not enough samples
1. Window the samples (Hann, Boxcar, Blackman-Harris, Flat Top, etc.)
1. Optionally zero-pad to the FFT size $n$
1. Run FFT to get $m = 1 + n / 2$ bins
1. Use only $b$ bins from $b_0$  to $b_1$
1. Calculate the bin magnitudes
1. Optionally combine bins to get $l$ analyser lines, see "Power of Two"
1. Scale, optionally apply analyser gravity and convert to unsigned 8-bit
1. Output the $l$ bytes
1. Advance the start of the next sequence by $d$ samples
1. Go back to step 1.

cava calculates frequency cut-offs, but this is not neccessary if we just omit
frequency bins.

An example: You want a visualiser with 28 bins with a frequency resolution of
200 Hz with the first starting at 600 Hz and 27 additional bins of each 200 Hz
higher, so that the last one is at 6000 Hz and you have data sampled at
44100 Hz.

A frequency resolution of 200 Hz at 44100 Hz means 220.5 bins. We round up to
$m = 221$ so that we can use an input size $n = 440$. The frequency resolution
is 44100 Hz / 119 = 201.4 Hz, which is fine. The first bin is DC and the last
bin is Nyquist frequency, so they aren't considered.

We run FFT and omit the first 3 bins and all others after the 31th bin.

If we discover that FFT at size 440 runs slowly, we can try 512 instead and
omit the first 4 bins and all others after the 32th bin and get a frequency
resolution of 44100 Hz / 255 = 173 Hz and the first starting at 4 · 173 Hz =
692 Hz and the last at 31 · 173 Hz = 5361 Hz. Or we can try with an FFT size
of 384. This means, you are going to need some experiments to find out what
works for you.

## Power of Two

If we combine bins, we gain even more flexibility but must use rather large
FFT sizes. This opens up another possbility: lines spaced by power of two.
The frequency bins are spaced linearly, but this is not what we hear. A note
one octave higher is double the frequency. This means, frequencies in music
are governed by the **Power of Two**.

For this we need to allocate $b$ bins to $l$ lines such that a line $k \in \{1,
l\}$ has about the double bins than a line representing one octave lower. If we
do this carefully we might even be able to map musical notes to analyser lines.

### Approach

The basis is the power of two $g(x) = a 2^x$ and we solve for $a$ such that we get what we want. Because we distribute only whole bins we need to work with integers.

$$\bar{g}(k) = \lceil a 2^k \rceil $$

The rounding up introduces distortions, for example if $g(x) \ll 1$ for the
lower lines they take away bins from the higher lines. At least it's a better
approximation to the natural experience of sound than the linear distribution
of lines. Even a simple stepwise linear increase of bins would be better.

The sum of bins over all lines is:

$$\sum_{k=0}^{l-1}{\bar{g}(k) = b}$$

however $\bar{g}(k)$ is not easily solvable. Without the rounding up the
solution is $a=\frac{b}{2^l-1}$. So let's approach this heuristically instead.
To explain the approach I draw $g(x)$ on grid paper. Each box corresponds to
a bin, this means we should have about $b$ boxes below the graph. We start
with the bottommost horizontal line of boxes between $k_0$ and $k_0+l-1$.

First we solve for $a$ the sum of $g(x_j)$ where $x_j=k_0+j$ and have
 $a=b/(2^l-1)$; and also for $x$ where the graph intersects with the horizontal
 line and have $x=lb\;i-lb\;a$ and round up to get $k_i$. This is the left side
 of the new bottommost horizonal line of boxes then we repeat till all the
 boxes aka the bins are exhausted.

So we start with:

1. $i=0$
1. $b_0=b$
1. $l_0=l$
1. $k_0=k$

then we repeat the following steps:

1. $i=i+1$
1. $a_i = b/(2^{l_{i-1}}-1)\qquad$ solution of the sum of $g(x_j)$
1. $x_i = lb\;i-lb\;a\qquad$ solution of $g(x_i) = i$
1. $k_i = \lceil x_i \rceil$
1. $\delta_i=k_{i-1}-I_{k_i-1}^{k_i}\qquad$ see below for $I$
1. $l_i = l_{i-1} + k_0 - k_{i-1} + 1$  
1. $b_i = b_{i-1} - \delta - l$
1. Go back to step $\;1\quad$ if $b > 0$

With $\delta$ we try to correct the rounding distortion for the next
iteration:

$$I_{k_i-1}^{k_i} = \int_{k_i-1}^{k_i} g(x)dx 
  = \frac{a2^{k_{i-1}}(2^{k_{i-1} - k_i} - 1)}{ln\,2} $$

$\delta$ is the difference between the area below the smooth curvature of
$g(x)$ and the blocky distribution of whole bins. We transfer the error to
the next iteration and resolve for $a$. This means that we get a piecewise
function but that's better than an iteration where the number of bins doesn't
match up at the end.

**Variation** for step 3: use $lb(i + \epsilon)$ for $0 < \epsilon ≤ 1$,
perhaps this minimises the error















