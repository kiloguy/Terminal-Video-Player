all: main.cpp
	g++ main.cpp -lm -pthread -o term-player
clean:
	rm -f record*jpeg ffmpeg_info info
