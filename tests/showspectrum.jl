#!/usr/bin/env julia

function usage(msg)
  print("""
    Error: $msg

    Read spectrum data from stdin and print bars on stdout.
    Usage: $PROGRAM_FILE [options]

    Options:
      -b number of spectrum bands (default 32, range 1-9999)
    """)
  exit(1)
end

num_bands = 32
if 2 == length(ARGS) && "-b" == ARGS[1] && occursin(r"^\d{1,4}$", ARGS[2])
  num_bands = parse(Int, ARGS[2])
  if !(1 <= num_bands <= 9999) usage("num_bands outside range 1-9999") end
elseif  0 != length(ARGS)
  usage("wrong number of arguments")
end

s = "0123456789abcdefghijklmnopqrstuvwxyz"
n = length(s)
for_line_len = 255 / 73.99

print("\e[2J\e[H")
println("Audio Spectrum with $num_bands band(s)")
println('-' ^ 79)

while true
  spectrum = read(stdin, num_bands)
  if length(spectrum) != num_bands break end

  for i in 1:num_bands
    write(stdout, s[i % n + 1:i % n + 1])
    write(stdout, lpad(spectrum[i], 4, ' '), "|")
    len = Int(floor(spectrum[i] / for_line_len))
    write(stdout, '#' ^ len, ' ' ^ (73 - len), '\n')
  end
  sleep(0.1)
  print("\e[$(num_bands)A");
end
print("\e[$(num_bands)B");
println('-' ^ 79)
