#!/usr/bin/env julia

import Random
Random.seed!(0)
r = () -> rand()
n = 1024
bias = 0
noise_amplitude = 0
phi = MathConstants.golden
#periods = [1phi] #, 2phi, 3phi, 4phi, 6phi]
periods = [20]
amplitudes = [1] #, 10, 10, 10, 10]

# f() is expected to return values in range [-1, 1]
function f(x :: Number)
  noise = 2 * noise_amplitude * r() - noise_amplitude
  wave = sum(sin.(2 .* pi .* float(x) ./ float(periods)) .* amplitudes)

  y0 = noise + wave + bias
  amplitude = noise_amplitude + sum(amplitudes) + bias
  @assert abs(y0) <= amplitude 
  y0 / amplitude # return such that -1 <= y <= 1
end

max = typemax(Int16)
for x in 0:n - 1
  y = floor(f(x) * max)
  y = Int16(y)

  try
     write(stdout, y)
  catch ex
    println(stderr, ex)
    exit(1)
  end
end
