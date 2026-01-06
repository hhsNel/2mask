#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ALLOC_GRAN 32

void handle_frame(FILE *frame, char *frameignore, char *stdinignore);

int main(int argc, char **argv) {
	int start, i, end;
	char *format;
	unsigned int fps = 30;
	char *frameignore = " ";
	char *stdinignore = "\t\n";
	char *filename;
	unsigned int length;
	FILE *frame;

	if(argc < 4) {
		printf("usage: %s <start> <end> <frame format string> [fps] [frameignore] [stdinigore]\n"
				"default:\n"
				"fps        \t=\t30\n"
				"frameignore\t=\t\" \"\n"
				"stdinignore\t=\t\"\\t\\n\"\n", argv[0]);
		exit(0);
	}

	start = atoi(argv[1]);
	end = atoi(argv[2]);
	format = argv[3];
	if(argc >= 5) fps = atoi(argv[4]);
	if(argc >= 6) frameignore = argv[5];
	if(argc >= 7) frameignore = argv[6];

	length = snprintf(NULL, 0, format, end);
	filename = malloc(length + 1);

	for(i = start; i <= end; ++i) {
		sprintf(filename, format, i);
		frame = fopen(filename, "rb");
		if(!frame) {
			printf("error opening %s\n", filename);
			exit(1);
		}
		handle_frame(frame, frameignore, stdinignore);
		fclose(frame);
		usleep(1000000 / fps);
		printf("\e[2J\e[H");
	}
}

void
handle_frame(FILE *frame, char *frameignore, char *stdinignore)
{
	unsigned int xcap, ycap, width, height, i, j, k;
	char **framedump;
	int c;

	xcap = width = i = 0;
	ycap = height = j = 0;
	framedump = NULL;
	while((c = fgetc(frame)) != EOF) {
		if(height == ycap) {
			ycap += ALLOC_GRAN;
			framedump = realloc(framedump, sizeof(char *) * ycap);
			for(k = height; k < ycap; ++k) {
				framedump[k] = NULL;
			}
		}
		if(width == xcap) {
			xcap += ALLOC_GRAN;
			for(k = 0; k < ycap; ++k) {
				framedump[k] = realloc(framedump[k], xcap);
			}
		}
		if(c == '\n') {
			i = 0;
			++j;
			++height;
		} else {
			framedump[j][i] = c;

			++i;
			if(i > width) width = i;
		}
	}

	for(j = 0; j < height; ++j) {
		for(i = 0; i < width; ++i) {
			if(strchr(frameignore, framedump[j][i]) == NULL) {
				do {
					c = getchar();
				} while (c != EOF && strchr(stdinignore, c));
				if(c == EOF) exit(0);

				putchar(c);
			} else {
				putchar(' ');
			}
		}
		free(framedump[j]);
		putchar('\n');
	}
	free(framedump);
}

