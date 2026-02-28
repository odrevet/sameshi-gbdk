This is a GBDK version of sameshi https://github.com/datavorous/sameshi chess engine for GBDK. 

* The sameshi.h compile as-is with GBDK toolchain. unsing the readable version, the stack has been lowered 

* The main file has been adapted to input coords with the joypad instead of a keyboard, but other function also remains as-is : board print using printf and no input verification

Graphics from: 

* Pieces: https://berryarray.itch.io/chess-pieces-16x16-one-bit

* Hand: https://aspecsgaming.itch.io/cursor-hands

# build

```sh
make -f Makefile.targets gb
```