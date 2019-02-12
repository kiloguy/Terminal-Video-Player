all: main.cpp
	g++ main.cpp -lm -pthread
clean:
	rm -f record*jpeg ffmpeg_info info
