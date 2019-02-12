#include	<stdio.h>
#include	<string.h>
#include	<thread>
#include	<chrono>
#include	<sys/ioctl.h>
#include	<unistd.h>
#define		STB_IMAGE_IMPLEMENTATION
#include	"stb_image.h"

int flag = 1;
bool stopTimer = false;
bool couldplay = false;
int frame = 0;

void counting(){
	std::this_thread::sleep_for(std::chrono::seconds(3));
	while(true){
		std::this_thread::sleep_for(std::chrono::milliseconds(125));
		flag++;
		if(stopTimer)
			break;
	}
}

int count(){
	FILE* ls = popen("ls -al | grep \"record.*.jpeg\" | wc", "r");
	char line[1024];
	fgets(line, 1024, ls);
	pclose(ls);
	return atoi(line);
}

void init_video(const char* filename){
	int second = 0;
	FILE* bin = NULL;
	char line[1024];
	char* temp = NULL;
	sprintf(line, "ffmpeg -i %s -hide_banner 2>info", filename);
	bin = popen(line, "r");
	pclose(bin);
	bin = popen("cat info | grep Duration | cut -d \' \' -f 4", "r");
	fgets(line, 1024, bin);
	temp = line;
	frame += atoi(temp) * 3600 * 8;
	temp = strstr(temp, ":") + 1;
	frame += atoi(temp) * 60 * 8;
	temp = strstr(temp, ":") + 1;
	frame += atoi(temp) * 8;
	pclose(bin);
	sprintf(line, "ffmpeg -i %s -y -vf fps=8 -vframes %d record%%d.jpeg 2>ffmpeg_info", filename, frame);
	bin = popen(line, "r");
	pclose(bin);
}

void remove_file(){
	std::this_thread::sleep_for(std::chrono::seconds(10));
	char filename[1024];

	int i = 1;
	while(i <= frame){
		std::this_thread::sleep_for(std::chrono::milliseconds(125));
		sprintf(filename, "record%d.jpeg", i);
		remove(filename);
		i++;
	}
}

int main(int argc, char** argv){
	struct winsize t;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &t);

	unsigned char* pixel = NULL;
	int w, h, n;
	char filename[1024];

	std::thread timerThread(counting);
	std::thread loadThread(init_video, argv[1]);
	std::thread clearThread(remove_file);
	printf("loading...\n");
	std::this_thread::sleep_for(std::chrono::seconds(3));

	while(true){
		sprintf(filename, "record%d.jpeg", flag);
		while(true){
			if((pixel = stbi_load(filename, &w, &h, &n, 0)) == NULL){
				std::this_thread::sleep_for(std::chrono::seconds(3));
				printf("\033[1;1Hloading...");
			}
			else
				break;
		}
		printf("\033[1;1H");
		int offsetRow = h / (t.ws_row - 1);
		int offsetCol = w / t.ws_col;
		int startCol;
		for(int i = 0; i < t.ws_row - 1; i++){
			for(int j = 0; j < t.ws_col; j++){
				printf("\033[48;5;%dm\033[38;5;%dm*\033[0m", 16 + 36 * (pixel[i * offsetRow * w * 3 + j * offsetCol * 3] / 43) + 6 * (pixel[i * offsetRow * w * 3 + j * offsetCol * 3 + 1] / 43) + (pixel[i * offsetRow * w * 3 + j * offsetCol * 3 + 2] / 43), 16 + 36 * (pixel[i * offsetRow * w * 3 + j * offsetCol * 3 + offsetCol / 2 * 3] / 43) + 6 * (pixel[i * offsetRow * w * 3 + j * offsetCol * 3 + 1 + offsetCol / 2 * 3] / 43) + (pixel[i * offsetRow * w * 3 + j * offsetCol * 3 + 2 + offsetCol / 2 * 3] / 43));
			}
		}
		stbi_image_free(pixel);
		printf("flag %d %s", flag ,filename);

		if(flag > frame)
			break;
	}
	stopTimer = true;
	timerThread.join();
	loadThread.join();
	printf("\nClearing data...\n");
	clearThread.join();
	return 0;
}
