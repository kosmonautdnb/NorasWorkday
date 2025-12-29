# Nora's Workday

**A presentation of the game on YouTube**: https://www.youtube.com/watch?v=sGpmDqRiE00

Nora's Workday is a small example game that demonstrates the possibilities of WatcomGL. WatcomGL is an OpenGL framework for FreeDOS and uses **WatcomC / OpenWatcom / DJGPP**.

In this quickly programmed game, you have to change the color of bouncing balls moving through Nora's world.

You play as the protagonist, Nora, and can use her "change" portals to alter her clothing and recolor the bouncing and moving balls.

By changing the colors of the balls, you earn money. At the end of the workday, you can see how much money you have collected.

You may use the key **END** or **ESCAPE** to "prematurely" finish the game. And when you see the "certficate" you may use the key **SPACE** to finally finish the game. This game is just a quick hack. Not much code involved in it.  

There are two pre-compiled .exe files for FreeDOS:

**"NORA1_DJ.EXE"** starts the game using DJGPP. The resolution is 640x480. The significantly higher performance demonstrates the strength of DJGPP as a compiler.

**"NORA1_OW.EXE"** starts the game with OpenWatcom 2.0. The resolution is 320x200. This file also runs very efficiently.

I recommend using WatcomC++ 11.0 for development, as it compiles quickly, and DJGPP for the finished game, as it offers good performance.

**Compilation:**

You will need a **RAM disk on R:**. Installation instructions can be found in AUTOEXEC.BAT in the _BUILD/ folder. The RAMDISK file (.exe) is also located in the _BUILD/ folder.

**Run !CLEAN.BAT** to clean the R:/ directory.


**Run !MAKE.BAT** to compile the entire project and create the file **R:/MAIN.EXE** with WatcomC 11.0 or another WatcomC/OpenWatcom version. Alternatively, you can select DJGPP in the batch file by setting DJGPPCOMPILE to 1.

**Run !RUN.BAT** to launch the game from **R:/MAIN.EXE**.

The game will access the **DATA/** folder, which contains all the game data.
