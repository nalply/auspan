#!/usr/bin/env julia

using Random
Random.seed!(0)

# Generate PCM s16le data by some sine waves and noise


# f() is expected to return values in range [-1, 1]
function f(x :: Number)
  bias = 0

  noise_amplitude = 10
  noise = 2 * noise_amplitude * rand() * rand() - noise_amplitude

  periods = [2.2, 3, 4.9, 5.3]
  amplitudes = [23, 7, 11, 10]
  x = float(x)
  wave = sum(sin.(2 .* pi .* x ./ float(periods)) .* amplitudes)

  y = noise + wave + bias
  amplitude = noise_amplitude + sum(amplitudes) + bias
  @assert abs(y) <= amplitude
  y = y / amplitude

  y
end

max = typemax(Int16)
for x in 1:2048
  y = floor(f(x) * max)
  y = Int16(y)

  try
     write(stdout, y)
  catch ex
    println(stderr, ex)
    exit(1)
  end
end
