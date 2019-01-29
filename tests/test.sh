#!/bin/bash

cd $(dirname "$0")

rm -f /tmp/asa-spectrum
./genwaves.jl > /tmp/asa.pcm
ASA_DBG=T ../asa-s16le -b 100 /tmp/asa.pcm /tmp/asa-spectrum 2> /tmp/asa.log
./showspectrum.jl -b 1000 < /tmp/asa-spectrum
