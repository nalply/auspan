# Simple Audio Spectrum Analyser (simple-asa)

Take a mono raw audio stream, PCM 16bit little endian, as produced by the fifo
output of mpd and generate a binary spectrum with a number of 8bit bands
directly usable by visual spectrum analyser projects like for Raspberry Pi.

Inspired by [cava](https://github.com/karlstav/cava), especially for options
like frequency cut-off, autosense and gravity (still not implemented).

The main difference is that this tool doesn't do platform specific things or
acesses audio drivers like ALSA. It just reads PCM 16 bit little endian signed
mono audio and outputs 8bit unsigned spectrum magnitudes.

Because, duh, that's simple!

## Note!

This software is not ready for everybody's use. Not all options
are implemented, also tests are still missing.

## Roadmap

- option -s
- options frequency cut-off, autosense and gravity similar to cava
- option to average or skip spans
- tests (especially byte-compare spectrum from synthesised audio)

## Usage example

`$ simple-asa -s 2 -b 10 /tmp/mpd.fifo /tmp/asa.fifo`

From mpd generate a 10-band spectrum about every 93 ms.

It's possible to configure mpd to output **mono** audio to a fifo. The audio
data is PCM with signed 16bit little-endian integer samples at a sample rate
of 44.1 kHz. By default simple-asa analyses audio data in spans of 2048
samples (use the option -n to change that). The option -s 2 lets simple-asa
output a spectrum every 2 spans.

This means the spectrums are written every 2⋅2048 / 44100 ≈ 0.0929 s ≈ 93 ms.
