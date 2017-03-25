c_synth
============

c_synth is a chiptune-ish software synthesizer. It takes a MML (actually a
language based on MML) song as input and compiles it into the memory. The
compiled buffer may then be played through whichever backend is desired, as long
as it accepts samples in little endian and use either 16 or 8 bits samples.

## Compiling and installing

All the following steps where also tested on Windows, using
[MinGW](http://www.mingw.org/).

There are no dependencies to build the library. After cloning it, simply run:

```
$ sudo make install DEBUG=yes
$ sudo make install RELEASE=yes
```

The library will be installed on /usr/lib/c_synth and the headers on
/usr/include/c_synth.

## Testing and running

There are a few songs on the directory 'samples/'. They may be compiled and
played using the examples on the directory 'tst/'.

Some tests uses SDL2 to actually play the compiled songs. On debian-based
distros, that library may be installed by running:

```
$ sudo apt-get install libsdl2-dev
```

For other linux distros, look into your package manager or download the
[source](https://www.libsdl.org/download-2.0.php) and compile it manually.

To build the tests, run:

```
$ make tests
```

To play one of the songs:

```
./bin/Linux/tst_repeatSongSDL2 --frequency 44100 --mode 2chan-16 --file samples/sonic2-chemical.mml
```

To exit, press Ctrl-C.

