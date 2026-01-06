#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ALLOC_GRAN 32

int handle_frame(FILE *frame, char *frameignore, char *stdinignore);

int main(int argc, char **argv) {
	int start, i, end;
	char *format;
	unsigned int fps = 30;
	char *frameignore = " ";
	char *stdinignore = "\r\t\n";
	char *filename;
	int noclear = 0;
	int displaylast = 0;
	int loop = 0;
	unsigned int length;
	FILE *frame;

	if(argc < 4) {
		printf("usage: %s <start> <end> <frame format string> [OPTIONS...]\n"
				"default:\n"
				"-p \tfps                         \t=\t30\n"
				"-f \tframeignore                 \t=\t\" \"\n"
				"-s \tstdinignore                 \t=\t\"\\r\\t\\n\"\n"
				"-c \tclear                       \t*\n"
				"-C \tno clear (can help with laggy inputs)\n"
				"-d \tdisplay last frame\n"
				"-D \tdon't display the last frame\t*\n"
				"-l \tloop\n"
				"-L \tdon't loop                  \t*\n", argv[0]);
		exit(0);
	}

	start = atoi(argv[1]);
	end = atoi(argv[2]);
	format = argv[3];

	for(i = 4; i < argc; ++i) {
		if(argv[i][0] == '-') {
			switch(argv[i][1]) {
#define REQUIRE_ANOTHER if(i + 1 >= argc) { printf("expected another argument after %s\n", argv[i]); exit(1); }
				case 'p':
					REQUIRE_ANOTHER;
					fps = atoi(argv[i + 1]);
					++i;
					break;
				case 'f':
					REQUIRE_ANOTHER;
					frameignore = argv[i + 1];
					++i;
					break;
				case 's':
					REQUIRE_ANOTHER;
					stdinignore = argv[i + 1];
					++i;
					break;
				case 'c':
					noclear = 0;
					break;
				case 'C':
					noclear = 1;
					break;
				case 'D':
					displaylast = 0;
					break;
				case 'd':
					displaylast = 1;
					break;
				case 'l':
					loop = 1;
					break;
				case 'L':
					loop = 0;
					break;
#undef REQUIRE_ANOTHER
			}
		} else {
			printf("unrecognized option: %s\n", argv[i]);
			exit(1);
		}
	}

	length = snprintf(NULL, 0, format, end);
	filename = malloc(length + 1);

	printf("\e[2J");

	do {
		for(i = start; i <= end; ++i) {
			sprintf(filename, format, i);
			frame = fopen(filename, "rb");
			if(!frame) {
				printf("error opening %s\n", filename);
				exit(1);
			}
			if(handle_frame(frame, frameignore, stdinignore)) {
				if(displaylast) {
					printf("\e[2J\e[HLast frame: %i\n", i);
				}
				exit(0);
			}
			fclose(frame);
			fflush(stdout);
			usleep(1000000 / fps);
			if(! noclear) {
				printf("\e[2J");
			}
			printf("\e[H");
		}
	} while(loop);
}

int
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
				if(c == EOF) return 1;;

				putchar(c);
			} else {
				putchar(' ');
			}
		}
		free(framedump[j]);
		putchar('\n');
	}
	free(framedump);

	return 0;
}

