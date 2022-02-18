
# Reverse engineering GPG's Dungeon Siege

This repository contains tools and code to extract and convert game assets
of the [Dungeon Siege][link_dsiege] and [Dungeon Siege: Legends of Aranna][link_ds_loa]
games, from developer *Gas Powered Games* (GPG).

**UPDATE:** See the [OpenSiege](https://github.com/OpenSiege) project for other more recent work on Dungeon Siege.

This project aims at reverse engineering the main file formats used by Dungeon Siege
to store its game assets (i.e.: textures, 3D models, sounds) and to provide tools
for converting them to more standard formats that can be opened/viewed outside the game.

At the current stage, the following file formats are implemented:

- Tank files (`.dsres|.dsmap`): Full support for opening and decompression (thanks to [Scott Bilas][link_scott]).

- Aspect models (`.asp`): Partial import and a tool that converts static geometry to Wavefront OBJ.

- Siege Nodes (`.sno`): Partial import and a tool that converts the geometry to Wavefront OBJ.

- RAW textures (`.raw`): Full support for importing and tools to convert to PNG and TGA formats.

- Skrit and Gas files are plain text, so they can be easily viewed and edited once extracted from a Tank.

Hint: Check the `misc/` directory for detailed descriptions of each format.

**Disclaimer:**

The goal of this project is to experiment, research, and educate on the topics
of game development and game asset management. All information was obtained via
reverse engineering of legally purchased copies of the games and information
made public on the Internet.

## Project structure / dependencies

Relevant source code for this project is located inside the `source/` directory.
The `misc/` directory contains additional information such as more detailed description
of the file formats, screenshots from extracted models, among other things not
directly related to the code. `build/` is just an empty dummy dir, but when compiling
the source, that's where the binaries are outputted.

`source/` is further divided into the following subdirectories:

- `siege/`: Source code for `LibSiege`, the main static library containing code
and data structures for interacting with Dungeon Siege game data.

- `utils/` Source code for `LibUtils`, a tiny static library with miscellaneous
helper code and infrastructure. This library is referenced by `LibSiege` and all
of the tools.

- `tools/`: Source code for the command line tools, such as the ASP/SNO model
converters and a tool to open and extract files from a Tank archive.

- `thirdparty/`: Third party code and libraries. It also contains copies of
a few source files made available by Scott Bilas [in his website][link_scott],
for quick reference and safe-keeping. Those files are not directly compiled into
this project and fall under a different license.

### Dependencies

The only dependencies of the project, at the moment, are a few Unix system calls (namely `stat` and `mkdir`)
which are all contained inside `LibUtils`. The libraries and tools are all written using the full
feature set of **C++11**, plus its Standard Library, so compiling will require a recent compiler like
Clang or GCC. It should also compile with Visual Studio 2015 or more recent.

[Mini-Z][link_mz] is currently the only external code required, and it is included in the `thirdparty/` dir.

### Building

A `premake4` script is included, which was only tested on Mac OSX with Clang
and will probably not work out-of-the-box with GCC & Linux. Fixing eventual
issues should be easy, nevertheless. This script is deprecated. We now have
a `CMakeLists.txt` that should be used instead. It can generate the projects
for Windows (VS) or Linux/OSX (GCC/Clang).

To generate the project files using CMake, you can use the command line or the 
[CMake GUI for Windows](https://cmake.org/).

To build using Premake on Linux/OSX, navigate to the source code root and run:

>     premake4 gmake

Then navigate into `build/` and just run `make`.

## Running the tools

The project is currently comprised of five command line tools, besides the static libraries.

- `tankdump`: Tool for opening and displaying information about a Tank archive.
It can also perform a full or partial decompression of a Tank into normal files in the file system.

- `raw2tga`: Converts RAW textures to the Targa Truevision (TGA) format (uncompressed).

- `raw2png`: Converts RAW textures to compressed PNGs.

- `tga2raw`: Converts TGA images back into Dungeon Siege RAW format.

- `asp2obj`: Converts ASP models to portable OBJ models. No animation support is available.

- `sno2obj`: Converts SNO models to portable OBJ models. SNO models are always static geometry used for the terrain/buildings.

All the above tools can be called with the `-h` or `--help` flags to display more
detailed usage information and the other available command line flags.

Prebuild Windows binaries are provided in the [build folder](https://github.com/glampert/reverse-engineering-dungeon-siege/tree/master/build).

## Special thanks

A big thanks to the folks at GPG for making this awesome game, one of the best
RPGs of all times, no doubt, and for making a huge amount of technical information
about it available. If only the bulk of the games industry was like that, we'd
have a lot better and more game developers today...

Another big thanks to Scott Bilas for providing detailed documentation and source code
samples on the Tank format and other details about the inner workings of Dungeon Siege.

Sam Brkopac (sbrkopac) for the CMake script for Windows!

And lastly, the [SiegeTheDay](http://www.siegetheday.org/) community and forums for making
the 3DMax import/export scripts available. It would have taken many more months to reverse
the 3D model formats if it wasn't for them!

## License

This project's source code is released under the [MIT License](http://opensource.org/licenses/MIT).

## Eye candy

![ASP models extracted from the game](https://raw.githubusercontent.com/glampert/reverse-engineering-dungeon-siege/master/misc/screenshots/montage.png "ASP models extracted from the game")

---

The blessings of Azunai The Defender upon you! Travel safely!


[link_dsiege]: https://en.wikipedia.org/wiki/Dungeon_Siege
[link_ds_loa]: https://en.wikipedia.org/wiki/Dungeon_Siege:_Legends_of_Aranna
[link_scott]:  http://scottbilas.com/games/dungeon-siege/
[link_mz]:     https://code.google.com/p/miniz/

