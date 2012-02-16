# Welcome to Lirch!

Lirch [lərtʃ] is an program that allows you to chat with other people on your local network.
It is meant to replicate the IRC protocol as closely as possible, with some potential extensions.
(As you might have noticed, you can't spell Lirch without IRC, but there are a few things tacked-on.)

## Planned features
1. Providing a simple and intuitive method for digital communication with those near you
2. Multiple use patterns:
    - A Qt4-based GUI, designed for easy use by normal end-users
    - An ncurses interface, designed for advanced users and/or those desiring light-weight installs
3. File transfer capabilities (unicast to single users or multicast to a channel)
4. Private (encrypted) sub-channels using pre-shared keys
5. The ability to persist conversations to a variety of log formats

## Communication
Chat about the project in the Lirch IRC channel, #Lirch, on Freenode.

## License
Lirch uses the BSD license, so it is open-source software.
(See ./LICENSE and ./CONTRIBUTORS.)

## Building Lirch
There are currently two ways to build Lirch.

### Using qmake (quick and messy, less configurable)
Just run: `qmake lirch.pro && make`

### Using cmake (better for developers, isolated source)
1. Create a directory to perform the build. `../lirch-build` is a good choice. We will use this in our example.
2. Navigate to this directory. We will assume the source is located in `../lirch-source`; if not, adjust accordingly.
3. From here, point cmake to the top source directory, which contains a file named CMakeLists.txt. We will use the ncurses interface to cmake, ccmake: `ccmake ../lirch-source`
    - You can also select from a variety of generators, using (c)cmake's -G argument. See `cmake` for list.
4. Use the keyboard to select the desired build options. You will need Qt4. Press `c` to configure, then `g` to generate.
5. If you used a Makefile generator option, run `make`. Otherwise, load the project into your desired environment.

Happy Coding!
