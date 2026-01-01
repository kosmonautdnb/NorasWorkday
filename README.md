# Nora's Workday

**A presentation of the game on YouTube**: https://www.youtube.com/watch?v=sGpmDqRiE00

Nora's Workday is a small example game that demonstrates the possibilities of WatcomGL. WatcomGL is an OpenGL framework for FreeDOS and uses **WatcomC / OpenWatcom / DJGPP**. It needs around **200mb** of accessable memory (by a Dos extender).

In this quickly programmed game, you have to change the color of bouncing balls moving through Nora's world.

You play as the protagonist, Nora, and can use her "change" portals to alter her clothing and recolor the bouncing and moving balls.

By changing the colors of the balls, you earn money. At the end of the workday, you can see how much money you have collected.

You can end the game prematurely by pressing the **END** ​​or **ESC** key. Once the certificate appears, you can permanently exit the game by pressing the **SPACEBAR** key. This game is just a small hack and contains very little code.

## WatcomGL

It has a fixed **16:9** aspect ratio (e.g., 1280x720). Therefore, it will appear **distorted on 4:3** monitors (e.g., 640x480).

You can find **WatcomGL** (an OpenGL for DJGPP/WatcomC/OpenWatcom) here:  
https://github.com/kosmonautdnb/WatcomGL  

And some **useful libraries** for DJGPP/(actually all are done for)WatcomC 11.0/OpenWatcom here:  
https://github.com/kosmonautdnb/WatcomLibs  

## DosBOX and DosBOX-X  
It is not yet memory-optimized and uses very large data files. Therefore, it doesn't run in DOSBox, which, according to the internet, only supports 64 MB of RAM. DOSBox-X with **memsize=256** does execute **NORA1_DJ.EXE**, but it is **far too slow**. The OpenWatcom version, **NORA1_OW.EXE**, **fails** due to the limited 64 MB. On a real FreeDOS machine, WatcomC (PMode/W) can access approximately **256 MB** of RAM instead of just 64 MB. DOSBox and DOSBox-X are far too slow for this; it is recommended to use VirtualBox or a real FreeDOS machine. Furthermore, DOSBox does not provide a virtual **16:9** display by default.

## Executables

There are two pre-compiled .exe files for FreeDOS:

**"NORA1_DJ.EXE"** starts the game using DJGPP. The resolution is 640x480. The significantly higher performance demonstrates the strength of DJGPP as a compiler.

**"NORA1_OW.EXE"** starts the game with OpenWatcom 2.0. The resolution is 320x200. This file also runs very efficiently.

I recommend using WatcomC++ 11.0 for development, as it compiles quickly, and DJGPP for the finished game, as it offers good performance.

## Compilation

You will need a **RAM disk on R:**. Installation instructions can be found in AUTOEXEC.BAT in the _BUILD/ folder. The RAMDISK file (.exe) is also located in the _BUILD/ folder. I choose a Ramdisk because USB Sticks wear out if used for a longer time (write cycles).

**Run !CLEAN.BAT** to clean the R:/ directory.

**Run !MAKE.BAT** to compile the entire project and create the file **R:/MAIN.EXE** with WatcomC 11.0 or another WatcomC/OpenWatcom version. Alternatively, you can select DJGPP in the batch file by setting DJGPPCOMPILE to 1.

**Run !RUN.BAT** to launch the game from **R:/MAIN.EXE**.

The game will access the **DATA/** folder, which contains all the game data.

You could **Run !BATCH.BAT** to build all the files (some settings are in there) of the game if you are in a non FreeDOS environment.  

## config.sys additions for WatcomC's PMODE/W:  

**WatcomC with 256 MB of memory on FreeDOS (with PMODE/W) instead of the 32 MB (with Dos4gw)**

DEVICE=C:\_DOS_\BIN\HIMEMX.exe  
DEVICE=C:\_DOS_\BIN\JEMM386.exe /SB /MAX=262144  

/MAX=262144 means 256MB may be available.  
