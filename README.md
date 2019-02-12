# Terminal Video Player
A video player that can play video file under a terminal(urxvt, xterm...) or a console(linux tty) using ANSI escape code.

## Requirements
* FFmepg (Check out your distro's package repositories or get it at [FFmpeg](https://www.ffmpeg.org)

## Build
	$ make

## Usgae
	$ term-player [FileName]
**If you close the program or terminate it in weired way(like Ctrl-C), please clear the data manually by:**
	$ make clean

## How it works

**video**  --ffmpeg--> **image sequences** --stb_image.h--> **pixels** --ANSI escape ocde--> **show in temrinal**
*(term-player now only supports for 8-bits colors(256 colors), so it will be weired in linux tty(usually supports only 16 basic colors).)*
