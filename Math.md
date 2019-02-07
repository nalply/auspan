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

If we combine bins, we gain even more flexibility but must use rather large FFT
sizes. This opens up another possbility: lines spaced by power of two. The
frequency bins are spaced linearly, but this is not what we hear. A musical
note one octave higher is double the frequency and another octave higher four
times the frequency and so on: this is the musical law of the Power of Two.

The visual analyser should respect this law. An analyser with a linear sequence
of lines appears to have more «resolution» for the higher musical notes. Notes
C and D might be too near to each other to be separated in a visual spectrum
analyse, but the notes C'' and D'' two octaves higher show up as two different
analyser lines. This can feel wrong.

### Approach

The basis is this function: $g(j) = a p^j$, with $j \in \{0,\,\ldots,\,l-1 \}$,
where $l$ is the number of lines or bands of the analyser and $p$ the
exponentiation base. If a line represents a musical note in the twelve-tone
equal temperament, use $p = \sqrt[12]{2}$.

$g()$ is the number of bins the line $j$ is assigned to. However we need
positive integer numbers. This introduces a possibly strong distortion, however
even with the distortion we would get a more natural representation. Any
monotonic rising function is better than a linear representation. If there are
many bins that even low lines get a few bins, there is little distortion. The
distortion is the strongest if there are not enough bins so that the first
lines all get only one bin. You can experiment with the tool `bins.jl`.

So we first calculate $\lceil g()\rceil$ naively then take away bins where it
hurts the least, mostly at the higher frequency because it doesn't matter if
a line has 500 or 499 bins. See `bins.jl` how this is implemented.
