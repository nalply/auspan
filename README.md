# Simple Audio Spectrum Analyser (simple-asa)

Take a mono raw audio stream, PCM 16bit little endian, as produced by the fifo
output of mpd and generate a binary spectrum with a number of 8bit bands
directly usable by visual spectrum analyser projects like for Raspberry Pi.

Inspired by [cava](https://github.com/karlstav/cava).

The main difference is that this tool doesn't do platform specific things or
acesses audio drivers like ALSA. It just reads PCM 16 bit little endian signed
mono audio and outputs 8bit unsigned spectrum magnitudes.

Because, duh, that's simple!

## Note!

This software is not ready for everybody's use. Not all options
are implemented, also tests are still missing.

## Roadmap

- Implement all options
- Implement tests (especially byte-compare spectrum from synthesised audio)
