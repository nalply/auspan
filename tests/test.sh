#!/bin/bash

./genwaves.jl > /tmp/asa.pcm
rm /tmp/asa-spectrum
ASA_DBG=T ../asa-s16le -b 32 /tmp/asa.pcm /tmp/asa-spectrum 2> /tmp/asa.log
./showspectrum.jl < /tmp/asa-spectrum
