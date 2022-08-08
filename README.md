
# ZDoom Road Generator

A C++ tool which can generate 2.5D maps for ZDoom-based games in UDMF format that look like a road. The tool is designed to use under Windows systems, but the source code does not use Win API and may be compiled under other systems.


## Purpose

I've created this tool primarily for my own ZDoom maps - to generate smooth and perfect road figures surrounded by a fence.
The tool takes an input data containing figures to draw with the road brush (lines or arcs optionally sloped). Also, the tool may take optional config file to change look of the roads (sizes, textures and light level).


## Theory

[ZDoom](https://zdoom.org/) is a modern source port made to play Doom (1993) and Doom-based video games. [UDMF (Universal Doom Map Format)](https://zdoom.org/wiki/Universal_Doom_Map_Format) is the modern map format for Doom-based games that can be consumed by ZDoom or other modern source ports (e. g. [GZDoom](https://zdoom.org/wiki/GZDoom)). Any Doom map is a [graph](https://en.wikipedia.org/wiki/Graph_(discrete_mathematics)) consisting of vertices, edges (linedefs with sidedefs), faces (sectors). Also, it contains things - interactive and technical objects (monsters, ammo, weapons, teleport destinations, players start points, etc.) and maybe more. See [UDMF specification](https://github.com/ZDoom/gzdoom/blob/master/specs/udmf.txt) and [ZDoom extensions](https://github.com/ZDoom/gzdoom/blob/master/specs/udmf_zdoom.txt) for more information about UDMF.
Valid Doom map must contain correctly linked vertices, linedefs, sidedefs and sectors without any extra units. So, drawing a doom map includes:

  1. Creation of vertices with correct coordinates x and y (floating point values).

  2. Creation of sectors, that represent rooms on the map. Sectors must have valid textures and z positions for floor and ceiling.

  3. Creation of sidedefs linked with existing sectors (linedefs are linked with sectors via sidedefs). They must have valid textures (if required) or no textures. There are three positions for texture on a sidedef: top, bottom and middle.

  4. Creation of linedefs linked with existing vertices (two) and sidedefs (one or two). Inner linedefs must be linked with two sidedefs and bounding linedefs must be linked with one. Linedefs have front and back sides (one-sided linedefs have only front side).

Also, at least a player start thing must be presented on the map to make its launch possible.
Drawing required figures programmatically may be performed via dragging virtual vertices in a direction (to create straight corridors) and/or rotating them around another vertex (to create arcs). On each iteration required vertices must be placed on the map. Then, on the same iteration it is usually required to create sidedefs, linedefs and (sometimes) sectors. First and final drawing iterations should be unique. This allows to create straight and curved corridors with constant z position.
Then, ZDoom allows useful UDMF extensions to slope a figure:

  1. Vertex properties ```zfloor``` and ```zceiling```. These properties represent explicitly set z positions for the floor and ceiling of a vertex respectively. This allows to slope triangular sectors which includes the vertex with these properties set as a part of its bound.

  2. Linedef specials. [181:Plane_Align](https://zdoom.org/wiki/Plane_Align) and [118:Plane_Copy](https://zdoom.org/wiki/Plane_Copy) allow creating slopes via setting linedef properties. 181:Plane_Align allows the easiest way to create slopes for mappers. After setting correct parameters for this special the sector in front of the linedef or behind it will have sloped floor or ceiling.

  3. Sector plane equations. Sector can be assigned a plane equation in the form of ax + by + cz + d = 0 to make slopes. Four properties for floor sloping are ```floorplane_a```, ```floorplane_b```, ```floorplane_c```, ```floorplane_d``` and four properties for ceil sloping are ```ceilingplane_a```, ```ceilingplane_b```, ```ceilingplane_c```, ```ceilingplane_d```. All four values must be set (for floor and/or for ceiling) for a sector to make a slope. This is the hardest way to make slopes.

Using arcs and lines even spiral figures can be created and combining these with slopes allows to create a [helix](https://en.wikipedia.org/wiki/Helix)!


## Automation

### Tool description

The tool implement drawing as moving virtual vertices across the 2D surface (and creating real vertices when needed). After vertices are created and in some other cases tool creates required sectors, sidedefs and linedefs. Slopes are made primarily via linedef special 181:Plane_Align, but for road mark sectors ```zfloor``` vertex property is used (the tool does not slope ceilings). Because mark sectors are surrounded by a single sector (road body) that's must be sloped itself, linedef specials for sloping in this case does not work. Also, tool creates a player start to make map testing possible.
The generated road is packed into a WAD file as a Doom map in UDMF format (WAD file path and map name may be choosen by user).
The tool accepts a single set of parameters on launch for all roads it will generate in a road config file (optional). The parameters include the sizes of road parts, textures for road walls, floors and ceilings and a single value representing the light level for the entire map (range 0-255).
The tool also accepts road input data - description of figures it will generate (mandatory). The tool may generate several figures for a single map (although the player start will be put inside the first figure making others accessible only via ```noclip``` cheat code). These figures may consist of several lines and/or arcs possibly sloped.
After generation the resulting map may be opened in some visual editor for further editing (e.g. with [Ultimate Doom Builder](https://zdoom.org/wiki/Doom_Builder#Ultimate_Doom_Builder_.28UDB.29)).


### Road Config

Road config file may provide road sizes, textures and light level of the map. This file has special format. The example of Road Config file with all possible data entries is given below:

```

Sizes:		#{
			   // distance between sky sector and road side
			   // must be an integer in the range between 33 and 4096, default - 128
			   BackgroundDist=128,

			   // fence height as it says
			   // must be an integer between 0 and smallest number among figure heights, default - 128
			   FenceHeight=128,

			   // width of the actual road
			   // must be an integer between 'RoadMarkWidth + 2' and 4096, default - 384
			   RoadWidth=384,

			   // width of a road side (each of two)
			   // must be an integer between 1 and 4096, default - 128
			   RoadSideWidth=128,

			   // height of road side sectors
			   // must not be greater than the fence height and greater than or equal to 0, default - 8
			   RoadSideHeight=8,

			   // width of road mark sectors
			   // must be an integer between 4 and 'RoadWidth - 2', default - 16
			   RoadMarkWidth=16,

			   // length of a single road mark
			   // large values are allowed and result in a continuous mark
			   // must be an integer between 16 and 65535, default - 256
			   RoadMarkLength=256,

			   // length of a gap between two road marks
			   // large values are allowed and result in a clear road without any marks
			   // must be an integer between 16 and 65535, default - 512
			   RoadMarkGap=512
			}

Textures:	#{
			   // sky texture (for background, etc.), default - "F_SKY1"
			   Sky="F_SKY1",

			   // floor texture of the sector surrounding the actual road, default - "FWATER1"
			   Background="FWATER1",

			   // wall texture of the fence surrounding the road, default - "BIGBRIK1"
			   Fence="BIGBRIK1",

			   // floor texture of the fence surrounding the road, default - "FLOOR7_1"
			   FenceFloor="FLOOR7_1",

			   // road side floor texture, default - "SLIME14"
			   RoadSide="SLIME14",

			   // road side wall texture, default - "STEP4"
			   RoadSideWall="STEP4",

			   // texture of the road itself, default - "CEIL5_1"
			   RoadBody="CEIL5_1",

			   // road mark texture, default - "FLAT19"
			   RoadMark="FLAT19"
			}

// light level of the whole map
// must be an integer between 0 and 255, default - 192
LightLevel:	#192

```


### Road Input Data

Road input data is a set of figures to draw. Figures may contain Lines and/or Arcs. Lines and Arcs may or may not be sloped. The file must contain no more than one definition per line from the set "Figure, Line, Arc, Slope". All of four possible definitions must contain the exact number of parameters of certain types (integer or floating point values). The file may also contain line comments starting with '#'. Examples of each definition:

  1. ```Figure 0 -1000 -90 128 4096 256``` says to start drawing a new figure at x0=```0``` and y0=```-1000```. Drawing must be directed downwards (angle=```-90``` in degrees means the negative direction of Y axis when angle=```0``` would mean positive direction of X axis). Starting floor position must be ```128``` and ceiling position must be ```4096``` + ```128```. Last number ```256``` represents a shift of the road mark coordinate. x0 and y0 must be between ```-30000``` and ```30000```. Floor position must be between ```-32757``` and ```32767```. Height must be between ```0``` and ```32767``` and resulting ceiling position must not be greater than ```32767```. Also, ceiling cannot be lower than the floor of the fence. Angle can be any real number and mark shift can be any integer number.
  
  2. ```Line 1000``` says to draw a line with a length of ```1000```. Line length must be between ```1``` and ```32767```.
  
  3. ```Arc 2500 45 128``` says to draw an arc with radius ```2500``` rotating by angle of ```45``` degrees counter-clockwise. ```128``` means arc must be divided by ```128``` parts for better drawing quality. Radius must be greater than or equal to one plus the total width of the figure divided by 2 and less than or equal to ```65535```. Divider must be small enough to draw proper lines with the given radius and sizes. Angle may be negative resulting in the change of the turning direction, but it must be small enough to avoid figure intersections (this will happen with absolute angle values close to ```360``` degrees).
  
  4. ```Slope 0.1``` says to start the sloping with the tangent equal to ```0.1```. This means that each 10 unints of further figure length will result in figure elevation by 1 unit. Tangent may have values between ```-0.5``` and ```0.5```. Positive tangent leads to figure elevation when negative leads to reverse process lowering figure floor position. Slopes are not allowed in the beginning or in the end of the figure.


### Usage

ZDoom Road Generator is command line tool. Here is how to use the tool (help command output):

```
road-gen - generates roads for Doom maps in UDMF format. The tool takes road input data with optional config file and creates the TEXTMAP. TEXTMAP is packed into the output WAD file.
Usage:
  road-gen.exe <input-file-path> [<output-file-path>] [<options>]
Arguments:
  <input-file-path> - file path with road figures to generate (required)
  <output-file-path> - output WAD file path (optional, default - 'roads.wad')
Options:
  -config <file-path> - road config file path; allows to change road sizes, textures, light settings, etc.
  -mapname <map-marker-lump-name> - output map name in the resulting WAD file; must be valid ZDoom map name, 8 chars maximum length (default - MAP01)
  --help, -h or /? - display this message
```


### Build

Visual Studio 2017 was used to create, build and test this project. To build the project:

1. Open the solution in Visual Studio 2017.

2. Choose any platform (x86 or x64) and Release configuration.

3. Build the solution.


### Test

For testing [ZDoom](https://zdoom.org/downloads) is required and an IWAD DOOM2.WAD. They should be put together in a folder. Also, ZDoom Road Generator tool should be built.

1. Launch Run.bat - it will produce the file "roads.wad".

2. Drag roads.wad onto zdoom.exe.

3. Start new game. You should see a special map where you are standing in the middle of the road seeing it continues further.

4. You may run around to observe the resulting map (figure).


## Examples

For the following input:

```

Figure -12000 -6000 0 0 4096 50
Line 12000
Arc 4000 30 16
Slope 0.1
Arc 7000 70 48
Slope 0.2
Arc 3000 80 64
Slope 0.15
Line 3000
Slope 0
Arc 8000 45 128
Slope -0.22
Line 4000
Arc 2000 -85 96
Slope -0.05
Line 4000
Slope 0
Line 1500

```

The tool produces this figure (screenshot from Ultimate Doom Builder):

![A test figure produced by the tool](/screenshots/screenshot-2d-1.png)

For another input:

```

Figure 0 -1000 -90 128 4096 256
Line 1000
Arc 2000 45 24
Slope 0.1
Arc 2000 45 128
Arc 2500 45 128
Arc 3000 45 128
Arc 3500 45 128
Arc 4000 45 128
Arc 4500 45 128
Arc 5000 45 128
Arc 5500 45 128
Arc 6000 45 128
Arc 6500 45 128
Arc 7000 45 128
Slope 0
Line 1000

```

It produces a helix-like figure (screenshot from Ultimate Doom Builder, numerous green lines are linedefs with special 181:Plane_Align set):

![A helix-like figure produced by the tool](/screenshots/screenshot-2d-2.png)

The road in the game looks like this (screenshots from ZDoom):

![An in-game screenshot #1 of a figure](/screenshots/screenshot-3d-1.png)
![An in-game screenshot #2 of a figure](/screenshots/screenshot-3d-2.png)
![An in-game screenshot #3 of a figure](/screenshots/screenshot-3d-3.png)
![An in-game screenshot #4 of a figure](/screenshots/screenshot-3d-4.png)


## Known issues

Sloped arcs with small divider values causes glitches like steps instead of a slope. To avoid that choose dividers big enough for arcs.
This tool was not tested much so upon a successful generation the map must be visually checked for any glitches using a visual map editor or ZDoom itself.


## License

Copyright (c) 2022 Andrey Anisimov <<https://github.com/varyier>>

This software is released under the terms of the MIT License.
See the [LICENSE](LICENSE) file for further information.
