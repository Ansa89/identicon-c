# Identicon
C library for creating identicons from strings.

The original algorithm for identicon creation is from [identicon.js](https://github.com/stewartlord/identicon.js) by [stewartlord](https://github.com/stewartlord).


### Compiling
You can choose from 3 different libraries to calculate the hash:
* [libs/md5.c](libs/md5.c), [libs/sha1.c](libs/sha1.c), [libs/sha256.c](libs/sha256.c), [libs/sha512.c](libs/sha512.c) are from [coreutils](http://www.gnu.org/s/coreutils) and don't need any additional dependency (this is the default)
* [libsodium](https://github.com/jedisct1/libsodium)<sup>1</sup> (`make USE_SODIUM=1`)
* [openssl](https://www.openssl.org/) (`make USE_OPENSSL=1`)

You can choose from 2 different libraries to write PNGs:
* [lodepng](https://github.com/lvandeve/lodepng) (this is the default)
* [stb](https://github.com/nothings/stb) (`make USE_STB=1`)

Both libraries don't need any additional dependency.

The [cairo](cairo) directory contains a cairo based version, but consider it more like a proof of concept.

<sup>1</sup> WARNING: libsodium doesn't have functions to calculate MD5 and SHA1, so I used `crypto_generichash` which produces a different hash compared to coreutils and openssl counterparts (and thus a different identicon will be created).

### Example code
You can build example code with `make example` and then run `./example` to see what options it needs.
