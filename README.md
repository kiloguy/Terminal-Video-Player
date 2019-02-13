# Terminal Video Player
A video player that can play video file under a terminal(urxvt, xterm...) or a console(linux tty) using ANSI escape code.

## Requirements
* FFmepg(with ffplay)

Check out your distro's package repositories or get it at [FFmpeg](https://www.ffmpeg.org). It will run ffmpeg and ffplay(for the sound) in background.

## Build
	$ make

## Usgae
	$ ./term-player <option> [FileName]
        Available options:
                -256        using 256(8bits) colors display (normal terminal emulators under X usually support this)
                -8          using 8 colors(or 16 colors if supported) display(use for Linux tty console, or some ssh software that doesn't have 256 colors)
		-gray	    using 4 gray scale colors to display(for tty console)
~**If you close the program or terminate it in weired way(like Ctrl-C), please clear the data manually by:**~

	$ make clean

## How it works

**video**  --ffmpeg--> **image sequences** --stb_image.h--> **pixels** --ANSI escape ocde--> **show in temrinal**
