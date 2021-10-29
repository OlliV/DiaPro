#!/usr/bin/env bash
set -euo pipefail
ver=3.3.10
./brew_downloader.sh fftw ${ver} big_sur
./brew_downloader.sh fftw ${ver} arm64_big_sur
tar xf "fftw_${ver}-big_sur.tar.gz"
mv fftw fftw_big_sur
tar xf "fftw_${ver}-arm64_big_sur.tar.gz"
mv fftw fftw_arm64_big_sur
