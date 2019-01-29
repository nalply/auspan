# The Math behind simple-asa

simple-asa uses FFT to get the frequency domain of sampled audio data with the
help of the fftw3 library. A span of samples, for example of length n=2048 is
converted into 1 + n / 2 = 1025 frequency bins.

**Note** There's a bug in `asa_bands()` which doesn't distribute the
frequencies evenly to the bands. However a redesign is needed. I think a
redistribution of bins to visualiser bands and frequency cutoff are
superfluous. Just run FFTs of the desired size directly and omit the lower and
higher bins.

An example: You have a visualiser with 28 bands with a frequency resolution of
200Hz with the first starting at 600Hz and 27 additional bands of each 200Hz
higher, so that the last one is at 6000Hz and you have data sampled at 44100Hz.

A frequency resolution of 200Hz at 44100Hz means 220.5 bins. We round down to
220 and get a frequency resolution of 200.45 Hz, which is fine. We run FFT at
the size of 220 and omit the first 3 bins and all other after the 28th bin we
used.

This works rather well with some compromises.

A variant would be that higher frequencies are coalesced to achieve logarithmic
behaviour like musical notes. More on that later.