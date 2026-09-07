#!/usr/bin/env bash

sudo apt update
sudo apt install build-essential ninja-build cmake git libntl-dev libgmp-dev

git clone --depth 1 --branch 2026.7.1 \
  https://github.com/cryptopp-modern/cryptopp-modern.git
cd cryptopp-modern
cmake --preset=default
cmake --build build/default --parallel
./build/default/cryptest.exe v
sudo cmake --install build/default
sudo ldconfig
cd ..

test -f /usr/local/include/cryptopp/blake3.h \
  || test -f /usr/include/cryptopp/blake3.h

g++ -std=c++17 -O2 \
  gen_puzzle.cpp \
  ope_interface.cpp \
  poly_field.cpp \
  rsa.cpp \
  tlp.cpp \
  commitment.cpp \
  ole.cpp \
  OT_1of2.cpp \
  TestRoutines.cpp \
  helper_functions.cpp \
  ole_enhanced.cpp \
  prf.cpp \
  setup.cpp \
  coin_toss.cpp \
  linear_comb.cpp \
  poly_interpolate.cpp \
  solve_puzzles.cpp \
  verify.cpp \
  -L/usr/local/lib -lntl -lgmp -lcryptopp -pthread \
  -o VHLC_TLP.exe

echo "Dependencies Installed, Experiments Compiled. Please run VHLC_TLP.exe"
