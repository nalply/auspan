# List of spectrums

A list of generated spectrum data with their generation parameters, parsed
by run.sh to automatically generate spectrums and diff the result with the
files.

To add a new test case: add `## spectrum-<id>` and a Markdown fenced code
block with the generation parameters. Don't use spaces in names.

## spectrum-0

```text
asa-s16le -b 32
```

## spectrum-5

```text
genwaves
   Random.seed!(0)
   n = 2048
   bias = 0
   noise_amplitude = 10
   periods = [2.2, 3, 4.9, 5.3]
   amplitudes = [23, 7, 11, 10]
asa-s16le -b 8
```
