#!/usr/bin/env julia

# todo implement argument parser -b bands (default 32, range 1-256)
num_bands = 32

s = "0123456789abcdefghijklmnopqrstuvwxyz"
n = length(s)
for_line_len = 255 / 73.99

print("\e[2J\e[H")
println("Audio Spectrum with $(num_bands) band(s)")
println('-' ^ 79)

while true
  spectrum = read(stdin, num_bands)
  if length(spectrum) != num_bands break end

  for i in 1:num_bands
    write(stdout, s[i % n:i % n])
    write(stdout, lpad(spectrum[i], 4, ' '), ":")
    len = Int(floor(spectrum[i] / for_line_len))
    write(stdout, '#' ^ len, ' ' ^ (73 - len), '\n')
  end
  sleep(0.1)
  print("\e[$(num_bands)A");
end
print("\e[$(num_bands)B");
println('-' ^ 79)
