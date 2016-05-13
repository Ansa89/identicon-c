# Identicon
C library for creating identicons from strings.

The original algorithm for identicon creation is from [identicon.js](https://github.com/stewartlord/identicon.js) by [stewartlord](https://github.com/stewartlord).

The SHA512 code is from [coreutils](http://www.gnu.org/s/coreutils).

The library used to write PNGs ([lodepng](https://github.com/lvandeve/lodepng)) is created by [lvandeve](https://github.com/lvandeve), but you can find a [cairo](https://www.cairographics.org) implementation in [cairo](cairo) directory.


### Example code
You can build example code with `make example` and then run `./example some_string file.png` to create your identicon.
