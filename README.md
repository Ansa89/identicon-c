# Identicon
C library for creating identicons from strings.

The original algorithm for identicon creation is from [identicon.js](https://github.com/stewartlord/identicon.js) by [stewartlord](https://github.com/stewartlord).


### Compiling
You can choose from 3 different libraries to calculate SHA512:
* [libs/sha512.c](libs/sha512.c) is from [coreutils](http://www.gnu.org/s/coreutils) and doesn't need any external library (this is the default)
* [libsodium](https://github.com/jedisct1/libsodium) (`make USE_SODIUM=1`)
* [openssl](https://www.openssl.org/) (`make USE_OPENSSL=1`)

You can choose from 2 different libraries to write PNGs:
* [lodepng](https://github.com/lvandeve/lodepng) (this is the default)
* [stb](https://github.com/nothings/stb) (`make USE_STB=1`)

Both libraries don't need any external library.

The [cairo](cairo) directory contains a cairo based version, but consider it more like a proof of concept.


### Example code
You can build example code with `make example` and then run `./example some_string file.png` to create your identicon.
