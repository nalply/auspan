# AUdio SPectrum ANalyser (auspan)

Take a mono raw audio stream, PCM 16bit little endian, as produced by the fifo
output of mpd and generate a binary spectrum with a number of unsigned 8 bit
lines for visual spectrum analyser projects like for Raspberry Pi.

Inspired by [cava](https://github.com/karlstav/cava).

Differences:

- Only mono
- No audio drivers or other platform specific things (let other software
  interface the hardare, because, duh, that's simple!)
- Just output, no spectrum display (except test/spectrum.jl)
- Somewhat different handling of data (for example no frequency cutoff but you
  can omit lower- and higher-frequency bins)
- Logarithmic scaling of lines
- Usage of Julia for tests

## Note

This software is not ready for everybody's use. Not all options are
implemented, also tests are still missing. It works, I get some nice spectrums
however I need to make it more user-friendly, too.

## Todo

- More tests (especially integration tests from generated audio from pcm.jl)
- Gravity and other scaling options
- Options to make frequency calculations easier "so this spectrum line is at
  440 Hz"
- Option `-r` to average over several samples

## Usage examples

First install [fftw3](http://fftw.org) then `$ make auspan`.

1. Generate a sine wave and display it as a spectrum with 10 lines:
  <br>`$ cd test`
  <br>`$ pcm.jl -n 2048 "3000*sin(x/10)" | ../auspan -s 2048 -l 10 > asa-spectrum`
  <br>`$ spectrum.jl -l 10`

1. From mpd generate a 10-line spectrum about every 93 ms:
   <br>`$ auspan -s 4096 -l 10 /tmp/mpd.fifo /tmp/spectrum.fifo`
  <br>It's possible to configure mpd to output **mono** audio to a fifo. The
   audio data is PCM with signed 16bit little-endian integer samples at a
   sample rate of 44.1 kHz. The option `-s` defines the sample span size for
   FFT. So we have a spectrum every 4096 / 44100 ≈ 93 ms.