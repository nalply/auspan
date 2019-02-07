#!/usr/bin/env julia

# Distribute b bins to l lines starting with band k

# TODO: reintroduce parameter k
# (makes the power function not as steep)

function usage(msg)
  print("""
    Error: $msg

    Distribute b bins to l lines matching with the power of p
    Usage: $PROGRAM_FILE <b> <l> <p>
      where 0 < b ≤ l as integers and 1 ≤ p ≤ 2 as reals
  """)
  exit(1)
end


function parse_args()
  if length(ARGS) != 3 usage("needs three arguments <b> <l> <p>") end

  try
    b = parse(Int, ARGS[1])
    if (b < 1) usage("<b> is smaller than 1") end
    l = parse(Int, ARGS[2])
    if (b < l) usage("<b> is smaller than <l>") end
    p = parse(Float64, ARGS[3])
    if (p < 1 || p > 2) usage("<p> outside range") end
    return (b + 0.0, l, p)
  catch e
    println(e)
    usage("Failed to parse")
  end
end

function max_index(array)
  max = typemin(eltype(array))
  index = 0
  for i in 1:length(array)
    if array[i] > max max, index = array[i], i end
  end
  return index
end

using Printf

function main()
  b, l, p = parse_args()
 
  g = if p == 0
    j -> b * (p - 1) * p ^ j / (p ^ l - 1)
  else
    j -> b / l
  end
  g = [g(j) for j in 0:l-1]
  min1 = j -> if j < 1 1 else j end
  lines = [min1(trunc(Int, g[j] + 1)) for j in 1:l]

  #println("    g   ", (@sprintf("%6.1f ", g[j]) for j in 1:l)...)
  #print("lines ", (@sprintf("%6d ", lines[j]) for j in 1:l)...)
  #println(" sum ", sum(lines), " ", sum(lines) - b)

  d = j -> lines[j] / (abs(lines[j] - g[j]) + 1) * sign(log(lines[j]))
  while b - sum(lines) < 0
    diffs = [d(j) for j in 1:l]
    max = -Inf
    index = 0
    for j in 1:l
      if diffs[j] > max max, index = diffs[j], j end
    end
    lines[index] -= 1

    #@printf("max %8.3g g %8.3g index %2d\n", max, g[index], index)
    #println("diffs   ", (@sprintf("%6.1f ", diffs[j]) for j in 1:l)...)
    #println(" sum ", sum(lines), " ", sum(lines) - b)
    ##print("lines ", (@sprintf("%6d ", lines[j]) for j in 1:l)...)
  end

  println(join(lines, " "))
end

main()
