# tasarch [![Build Status](https://travis-ci.org/flagbot/tasarch.svg?branch=master)](https://travis-ci.org/flagbot/tasarch)

**tasarch** is a small [libretro](https://www.libretro.com/) frontend that focuses on TASing.
It will be used in our upcoming CTF ;)

Currently it will load libgambatte core by default (hardcoded path).

## Building

``` bash
mkdir build
cd build
cmake ..
make -j8 # or something like that
```

## Running

```bash
# inside build
./tasarch
```
