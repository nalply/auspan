#!/usr/bin/env julia

# Distribute b bins to l lines starting with band k

function usage(msg)
  print("""
    Error: $msg

    Distribute b bins to l lines starting with line at k
    Usage: $PROGRAM_FILE <b> <l> <k>
      where all parameters are integer numbers, 0 > b >= l and k >= 0.
  """)
  exit(1)
end


function parse_args()
  if length(ARGS) != 3 usage("needs three arguments <b> <l> <k>") end

  try
    b = parse(Int, ARGS[1])
    if (b < 1) usage("<b> is smaller than 1") end
    l = parse(Int, ARGS[2])
    if (b < l) usage("<b> is smaller than <l>") end
    k = parse(Int, ARGS[3])
    if (k < 0) usage("<k> is negative") end
    return (b + 0.0, l, k)
  catch e
    println(e)
    usage("Failed to parse")
  end
end


function lb(x) return log(x) / log(2) end


using Printf

function main()
  b, l, k = parse_args()

  bins = b
  len = l
  lines = zeros(len)
  k0 = kprev = k
  i = 0
  ln2 = log(2)

  println(" i ze/on    b    l        a       x   kp    k   kd       I   delta")
  while sum(lines) < bins
    num0   = k - k0
    num1   = len + k0 - k
    i     += 1

    @printf("%2d %2d/%2d %4.0f %4d", i, num0, num1, b, l)

    lines += vcat(zeros(num0), ones(num1))

    a      = b / (2 ^ l - 1)                      ; @printf(" %8.3g", a)
    x      = lb(i) - lb(a)                        ; @printf(" %7.2f", x)
    kp     = k                                    ; @printf(" %4d", kp)
    k      = trunc(Int, x + 1)                    ; @printf(" %4d", k)
    kd     = k - kp                               ; @printf(" %4d", kd)
    I      = a * 2 ^ kp * (2 ^ kd - 1) / ln2      ; @printf(" %7.2f", I)
    delta  = kp - I                               ; @printf(" %7.2f", delta)
    l      = l - kd
    b      = b - delta - l
                                                  ; println()

  end
end

main()
