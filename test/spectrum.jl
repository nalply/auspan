#!/usr/bin/env julia

using Printf

function usage(msg)
  if (msg != "") println("Error: $msg") end
  print("""

    Read u8 data and show spectrum lines with Unicode block elements.

    Usage: $PROGRAM_FILE [-l number-of-lines] [input-file]
    where number-of-lines is a number in range from 1 to 9999, default 32
    and input-file is a file name with the u8 spectrum, default asa-spectrum
    """)
  exit(1)
end

function parse_args()
  arglen = length(ARGS)
  index = 1
  l = 32

  while index <= arglen
    if "-h" == ARGS[index] usage("") end
    if "-l" == ARGS[index]
      try l = parse(Int, ARGS[1 + index])
      catch e
        if isa(e, ArgumentError) usage("-l invalid integer: $(e.msg)")
        else throw(e)
        end
      end
      index += 2
    else
      break
    end
  end

  ifile = open(index <= length(ARGS) ? ARGS[index] : "asa-spectrum", "r")
  return (l, ifile)
end

l, ifile = parse_args()

println("Audio Spectrum with $l line(s)")
println('-' ^ 69)
print("\n" ^ l);
println('-' ^ 69)
print("\e[$(l + 1)A");


# Space and Unicode block characters (only every second used)
blocks = [" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉"]

# https://stackoverflow.com/a/29270413
ccall(:jl_exit_on_sigint, Nothing, (Cint,), 0)
i = 0
try
  while true
    spectrum = read(ifile, l)
    if length(spectrum) != l break end

    for i in 1:l
      @printf("%3d ", spectrum[i])
      write(stdout, '█' ^ div(spectrum[i], 4))
      m8 = mod(spectrum[i], 4) * 2
      if 0 != m8 write(stdout, blocks[m8]) end
      write(stdout, "\e[K\n")
    end
    print("\e[$(l)A");
  end
catch e
  if !isa(e, InterruptException) throw(e) end
end
print("\e[?25h\e[$(l - i + 1)B")
