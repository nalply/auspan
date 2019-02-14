#!/usr/bin/env julia

import Random
Random.seed!(0)


function usage(msg)
  if (msg != "") println("Error: $msg") end
  print("""

    Generate PCM s16le from a Julia function and write to stdout.

    Usage: $PROGRAM_FILE [-n length] "function"
    where function is a valid Julia function body of f(x), eg. "3000*sin(x/10)"
      f(x) should be in the Int16 range or an InexactError is thrown
    """)
  exit(1)
end

function parse_args()
  index = 1
  n = 2048
  if 0 == length(ARGS) usage("") end

  while true
    if "-h" == ARGS[index] usage("") end
    if "-n" == ARGS[index]
      try n = parse(Int, ARGS[1 + index ])
      catch e
        if isa(e, ArgumentError) usage("-n invalid integer: $(e.msg)") 
        else throw(e)
        end
      end
      index += 2
    else
      break
    end
  end

  # todo check argument count
  f = ARGS[index]
  try
    return (n, eval(Meta.parse("function(x) $f end")))
  catch e
    if isa(e, Meta.ParseError)
      usage("Julia parse error: $(e.msg)")
    else
      throw(e)
    end
  end
end

using Printf

(n, f) = parse_args()

for x in 0:n - 1
  y = Int16(floor(f(x)))

  try
    #println("x $x y $y")
    write(stdout, y)
  catch ex
    println(stderr, ex)
    exit(1)
  end
end
