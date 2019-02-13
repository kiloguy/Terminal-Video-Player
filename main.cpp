#include	<stdio.h>
#include	<signal.h>
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

enum COLOR_MODE{
	COLOR256,
	COLOR8,
	COLORgray,
};

/* Global options */
COLOR_MODE mode = COLOR256;
char inputFile[1024];

void play_music(){
	char line[1024];
	sprintf(line, "ffplay -autoexit -nodisp %s 2>/dev/null", inputFile);
	FILE* play = popen(line, "r");
	pclose(play);
}

void counting(){
	std::this_thread::sleep_for(std::chrono::seconds(3));
	std::thread music(play_music);
	while(true){
		std::this_thread::sleep_for(std::chrono::milliseconds(125));
		flag++;
		if(stopTimer)
			break;
	}
	music.join();
}

void init_video(){
	int second = 0;
	FILE* bin = NULL;
	char line[1024];
	char* temp = NULL;
	sprintf(line, "ffmpeg -i %s -hide_banner 2>info", inputFile);
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
	sprintf(line, "ffmpeg -i %s -y -vf fps=8 -vframes %d record%%d.jpeg 2>ffmpeg_info", inputFile, frame);
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

bool parse(char** v, int c){
	for(int i = 1; i < c - 1; i++){
		if(strcmp(v[i], "-256") == 0)
			mode = COLOR256;
		else if(strcmp(v[i], "-8") == 0)
			mode = COLOR8;
		else if(strcmp(v[i], "-gray") == 0)
			mode = COLORgray;
		else{
			printf("Unknown option \"%s\".\n", v[i]);
			return false;
		}
	}
	strcpy(inputFile, v[c - 1]);
	return true;
}

void user_signal(int signum){
	printf("\nInterrupt signal received. Clearing data...\n");
	FILE* rm = popen("rm record*.jpeg 2>/dev/null", "r");
	pclose(rm);
	exit(signum);
}

int main(int argc, char** argv){
	if(!parse(argv, argc))
		return -1;
	FILE* test = fopen(inputFile, "r");
	if(!test){
		printf("File \"%s\" not found.\n", inputFile);
		return -1;
	}
	fclose(test);

	signal(SIGINT, user_signal);

	struct winsize t;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &t);

	unsigned char* pixel = NULL;
	int w, h, n;
	char filename[1024];
	
	int offsetRow = h / (t.ws_row - 1);
	int offsetCol = w / t.ws_col;

	std::thread timerThread(counting);
	std::thread loadThread(init_video);
	std::thread clearThread(remove_file);
	printf("loading...\n");
	std::this_thread::sleep_for(std::chrono::seconds(3));
	
	while(true){
		int index = flag;
		sprintf(filename, "record%d.jpeg", index);
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
		
		for(int i = 0; i < t.ws_row - 1; i++){
			for(int j = 0; j < t.ws_col; j++){
				int backid, foreid;
				unsigned char br = pixel[i * offsetRow * w * 3 + j * offsetCol * 3];
				unsigned char bg = pixel[i * offsetRow * w * 3 + j * offsetCol * 3 + 1];
				unsigned char bb = pixel[i * offsetRow * w * 3 + j * offsetCol * 3 + 2];
				unsigned char fr = pixel[i * offsetRow * w * 3 + j * offsetCol * 3 + offsetCol / 2 * 3];
				unsigned char fg = pixel[i * offsetRow * w * 3 + j * offsetCol * 3 + 1 + offsetCol / 2 * 3];
				unsigned char fb = pixel[i * offsetRow * w * 3 + j * offsetCol * 3 + 2 + offsetCol / 2 * 3];
				switch(mode){
					case COLOR256:
						backid = 16 + 36 * (br / 43) + 6 * (bg / 43) + (bb / 43);
						foreid = 16 + 36 * (fr / 43) + 6 * (fg / 43) + (fb / 43);
						break;
					case COLOR8:
						if(br > 128){
							if(bg > 128 && bb > 128)
								backid = 7;
							else if(bg > 128 && bb < 128)
								backid = 3;
							else if(bg < 128 && bb > 128)
								backid = 5;
							else
								backid = 1;
						}
						else{
							if(bg > 128 && bb > 128)
								backid = 6;
							else if(bg > 128 && bb < 128)
								backid = 2;
							else if(bg < 128 && bb > 128)
								backid = 4;
							else
								backid = 0;
						}
						if(fr > 128){
							if(fg > 128 && fb > 128)
								foreid = 7;
							else if(fg > 128 && fb < 128)
								foreid = 3;
							else if(fg < 128 && fb > 128)
								foreid = 5;
							else
								foreid = 1;
						}
						else{
							if(fg > 128 && fb > 128)
								foreid = 6;
							else if(fg > 128 && fb < 128)
								foreid = 2;
							else if(fg < 128 && fb > 128)
								foreid = 4;
							else
								foreid = 0;
						}
						if(fr + fg + fb > 384)
							foreid += 8;
						break;
					case COLORgray:
						if(br + bg + bb > 384)
							backid = 7;
						else
							backid = 0;
						if(fr + fg + fb > 576)
							foreid = 15;
						else if(fr + fg + fb > 384)
							foreid = 7;
						else if(fr + fg + fb > 192)
							foreid = 8;
						else
							foreid = 0;
				}
				printf("\033[48;5;%dm\033[38;5;%dm*\033[0m", backid, foreid);
			}
		}
		stbi_image_free(pixel);
		printf("[");
		int times = (t.ws_col - 2) * index / frame;
		for(int i = 0; i < times; i++)
			printf("=");
		printf("\033[%dC", t.ws_col - 2 - times);
		printf("]");

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
