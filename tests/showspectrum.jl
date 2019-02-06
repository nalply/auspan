#!/usr/bin/env julia

using Printf

function usage(msg)
  print("""
    Error: $msg

    Read spectrum data from stdin and print bars on stdout.
    Usage: $PROGRAM_FILE [options]

    Options:
      -b number of spectrum bins (default 32, range 1-9999)
      -f frequency resolution (default 1)
    """)
  exit(1)
end

function parse_args()
  size = 32
  freq = 1
  arglen = length(ARGS)
  i = 0

  while i < arglen
    if 2 + i <= arglen && "-b" == ARGS[1 + i]
      if !occursin(r"^\d{1,4}$", ARGS[2 + i]) usage("-b malformed") end
      size = parse(Int, ARGS[2 + i])
      if !(1 <= size <= 9999) usage("-b outside range 1-9999") end
      i += 2
    elseif 2 + i <= arglen && "-f" == ARGS[1 + i]
      try 
        freq = parse(Float64, ARGS[2 + i]) 
      catch 
        usage("-f malformed")
      end
      i += 2
    else
      usage("unknown parameter")
    end
  end

  return (size, freq)
end

size, freq = parse_args()
for_line_len = 255 / 73.99

#print("\e[2J\e[H")
println("Audio Spectrum with $size bin(s)")
println('-' ^ 79)

while true
  spectrum = read(stdin, size)
  if length(spectrum) != size break end

  for i in 1:size
    @printf("%7.1f %3d|", i * freq, spectrum[i])
    len = Int(floor(spectrum[i] / for_line_len))
    write(stdout, '#' ^ len, ' ' ^ (73 - len), '\n')
  end
  sleep(0.1)
  print("\e[$(size)A");
end
print("\e[$(size)B");
println('-' ^ 79)
