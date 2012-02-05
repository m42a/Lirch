# Welcome to Lirch!

## Communication
Chat in the Lirch IRC channel, #Lirch.

## License
Lirch uses the BSD license, so it is open-source software.
(See ./LICENSE and ./CONTRIBUTORS.)

## Building Lirch
There are currently two ways to build Lirch.

### Using qmake (quick and messy, less configurable)
> `qmake lirch.pro && make`

### Using cmake (better for developers, isolated source)
1. Create a directory to perform the build. `../lirch-build` is a good choice. We will use this in our example.
2. Navigate to this directory. We will assume the source is located in `../lirch-source`; if not, adjust accordingly.
3. Point cmake to the top source directory, which contains CMakeLists.txt. We will use the ncurses interface to cmake, ccmake: `ccmake ../lirch-source`
4. Use the keyboard to select the desired build options. You will need Qt4. Press `c` to configure, then `g` to generate.
5. You can select a variety of generators. See `cmake` for list. If you used a Makefile generator option, run `make`.
