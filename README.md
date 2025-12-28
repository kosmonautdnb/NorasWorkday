# Nora's workday

**A presentation of the game on Youtube**: https://www.youtube.com/watch?v=sGpmDqRiE00

Nora's workday is a tiny example game, of what you can actually do with WatcomGL. An OpenGL for FreeDOS and **WatcomC / OpenWatcom / DJGPP**.

It is a fast coded game where you have to recolor some jumping spheres which are on their way through Nora's realm.

In the game you are the protagonist Nora which can use it's "recoloring" portals to change the clothes and recolor the jumping and moving spheres differently.

You get some money by recoloring the spheres. If the workday is done you see the amount of money you collected throughout your entire workday.

There are two precompiled .exe files for FreeDOS:

"NORA1_DJ.EXE" which starts the game with DJGPP, it has the resolution of 640x480 then and shows the strength of DJGPP as a compiler by the much faster performance.
"NORA1_OW.EXE" which starts the game with OpenWatcom2.0, it has the resolution of 320x200 and runs also very performant. 

I think it is a good way to use WatcomC++ 11.0 for developing, because of the compilation speed, and DJGPP for the final product / game because of it's good performance.

How to build it:

You need a **RamDisk at R:**, instructions on how to install it are in the AUTOEXEC.BAT in the _BUILD/ folder, the RAMDISK exe is in the _BUILD/ folder, too.

**run !CLEAN.BAT** that does a clean of R:/.
**run !MAKE.BAT** compiles the whole project and creates R:/MAIN.EXE with WatcomC 11.0 or any other WatcomC/OpenWatcom, you can also choose DJGPP in the batch file by setting DJGPPCOMPILE to 1.
**run !RUN.BAT** starts the game from R:/MAIN.EXE.

The game just accesses the DATA/ folder which includes all the assets of the game.
