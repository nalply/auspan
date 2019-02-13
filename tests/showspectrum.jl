#!/usr/bin/env julia

using Printf

function usage(msg)
  print("""
    Error: $msg

    Read u8 data from stdin and show spectrum lines with Unicode block elements
    Usage: $PROGRAM_FILE [options]

    Option: -l number of spectrum lines (default 32, range 1-9999)
    """)
  exit(1)
end

function parse_args()
  size = 32
  freq = 1
  arglen = length(ARGS)
  i = 0

  while i < arglen
    if 2 + i <= arglen && "-l" == ARGS[1 + i]
      if !occursin(r"^\d{1,4}$", ARGS[2 + i]) usage("-l malformed") end
      size = parse(Int, ARGS[2 + i])
      if !(1 <= size <= 9999) usage("-l outside range 1-9999") end
      i += 2
# for more options
#    elseif 2 + i <= arglen && "-f" == ARGS[1 + i]
#      try 
#        freq = parse(Float64, ARGS[2 + i]) 
#      catch 
#        usage("-f malformed")
#      end
#      i += 2
    else
      usage("unknown parameter")
    end
  end

  return size
end

size = parse_args()

print("\e[2J\e[?25l\e[H")
println("Audio Spectrum with $size line(s)")
println('-' ^ 69)
print("\e[$(size)B");
println('-' ^ 69)
print("\e[$(size + 1)A");

run(`stty -echo`)


# Space and Unicode block characters (only every second used)
blocks = [" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉"]

# https://stackoverflow.com/a/29270413
ccall(:jl_exit_on_sigint, Nothing, (Cint,), 0)
try
  while true
    spectrum = read(stdin, size)
    if length(spectrum) != size break end

    for i in 1:size
      @printf("%3d ", spectrum[i])
      write(stdout, '█' ^ div(spectrum[i], 4))
      m8 = mod(spectrum[i], 4) * 2
      if 0 != m8 write(stdout, blocks[m8]) end
      write(stdout, "\e[K\n")
    end
    print("\e[$(size)A");
  end
catch e
  if isa(e, InterruptException)
  else
    throw(e)
  end
end
print("\e[?25h\e[H\e[$(size + 3)B")
run(`stty echo`)
