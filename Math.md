# The Math behind simple-asa

`simple-asa` uses FFT to get the frequency domain of sampled audio data with the
help of the fftw3 library.

This is a description of `simple-asa`'s internal mathematical workings
and not a manual or guide!

## The steps

`simple-asa` is essentially an endless loop working on an audio stream with these
steps:

1. Take a sequence: $s$ PCM samples
1. Stop if there were not enough samples
1. Window the samples (Hann, Boxcar, Blackman-Harris, Flat Top, etc.)
1. Zero-pad to the FFT size $n$ if $s < n$.
1. Run FFT to get $m = 1 + n / 2$ bins
1. Use $b$ bins from $b_0$  to $b_1$ where $b ≤ m$
1. Calculate the bin magnitudes
1. Combine bins to get $l$ analyser lines, see "Power of Two"
1. Scale, optionally apply analyser gravity and convert to unsigned 8-bit
1. Output the $l$ bytes
1. Advance the start of the next sequence by $d$ samples
1. Go back to step 1.

It's a variant of the procedure described in this [StackOverflow answer](https://stackoverflow.com/a/2885833).

## Window function

FFT as an implementation of DFT has the assumption that the signal repeats
smoothly beyond the ends of the sequence. Usually this is not the case for
music (except for the rare case of samples being same at the start and at the
end of the sequence, for example for a sine wave of exactly the right period).
So FFT treats the abrupt changes between sequences as high-frequency signals.
Window function smooth down the sequence boundaries to zero such that these
changes disappear, however this introduces other distortions.

For details about the different window functions read the Wikipedia page on
[Window Function](https://en.wikipedia.org/wiki/Window_function) and about the
rationale of windowing this [StackOverflow answer](https://stackoverflow.com/a/7339777).


## Bins and lines

The FFT algorithm produces bins and the analyser shows lines. The number of
bins is: $m = 1 + n / 2$ where $n$ is the FFT size. The number of lines is: $l$
where $l ≤ m$.

## Frequency cutoffs

cava calculates frequency cut-offs, but this is not neccessary if we just omit
frequency bins.

FFT has as result $m$ bins linearly spaced in frequency from 0 to the Nyquist
frequency ($f / 2$ where $f$ is the sampling frequency). So if we run an FFT
with $n = 1024$ on a source with the sampling frequency $44.1$ kHz, we have
$1025$ bins at a resolution of $f / n = 43.1$ Hz.

A practical example on how to cutoff frequencies: You want a visualiser with 28
lines with a frequency resolution of 200 Hz with the first line starting at 600
Hz and 27 additional lines of each 200 Hz higher, so that the last one is at
6000 Hz and you have data sampled at 44100 Hz.

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

When combining bins we can try to mimic music perception. Octaves are separated
by doubling the frequency. For example the note C'' has four times the
frequency than C. This is the musical octave law.

However a naive spectrum analyser might neglect this law. For example it shows
C and D in the same line but C'' and D'' in two different lines, which can feel
subtly wrong.

### Approach

The base is the function $g(j) = a p^j$, with $j \in \{0,\,\ldots,\,l-1 \}$,
where $l$ is the number of analyser lines and $p$ the exponentiation base. If a
line roughly corresponds to a musical note in the twelve-tone equal
temperament, use $p=\sqrt[12] 2$.

$g()$ is rounded up to the number of bins the line $j$ is assigned to. This
introduces possibly strong distortions, but even if it is so the lines still
conform better to the natural sense than the linear spacing. The distortion is
the strongest if there are not enough bins so that the first lines all get only
one bin. You can experiment with the tool `test/power`.

#### The steps in the function `asa_distribute_bins()`

If $p = 1$ we have a linear distribution instead. Distribute on average $b/l$
bins to each line then stop. Don't use the following steps because the formula
diverges for $p = 1$.

We have solved for $a$ in $g(j) = a p^j$ by setting the sum of the bins of all
lines to $b$ and use $a = b\cdot (1-p) / (1-p^l)$ in $g(j)$, then:

1. Calculate naively
   $\bar{g}(j) = \lceil g(j) \rceil$ for all $j$.
1. However the sum overshoots $b:\quad\sum \bar{g}(j) > b$
1. Let's correct the overshoot by calculating weights 
   $\omega(j) = \left| \bar{g}(j) - g(j) \right|\cdot\mathrm{ln}(g(j))$
   for all $j$.
1. And take away one bin for line $j$ where $\omega(j)$ is at maximum
1. Repeat the previous two steps till the overshoot disappeared
