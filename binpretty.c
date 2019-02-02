#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define COL_RESET "\x1b[0m"
#define COL_REGULAR "\x1b[32m"
#define COL_WHITESPACE "\x1b[31m"
#define COL_BINARY "\x1b[34m"

static void color(FILE *out, char *col) {
	static char *currcol = NULL;
	if (col != currcol) {
		currcol = col;
		fprintf(out, col);
	}
}

static void initline(FILE *out, size_t bytenum) {
	color(out, COL_RESET);
	fprintf(out, "%.8X ", (unsigned int)bytenum);
}

static int pretty(FILE *in, FILE *out) {
	unsigned char buf[4096];

	size_t bytenum = 0;
	size_t linechar = 0;

	initline(out, 0);
	while (1) {
		size_t cnt = fread(buf, 1, sizeof(buf), in);

		for (size_t i = 0; i < cnt; ++i) {
			bytenum += 1;

			unsigned char c = buf[i];

			if (isalnum(c) || ispunct(c)) {
				color(out, COL_REGULAR);
				fprintf(out, "%c", c);
				linechar += 1;
			} else if (c == ' ') {
				fprintf(out, " ");
				linechar += 1;
			} else if (c == '\n') {
				color(out, COL_WHITESPACE);
				fprintf(out, "\\n\n");
				initline(out, bytenum);
				linechar = 0;
			} else if (c == '\r') {
				color(out, COL_WHITESPACE);
				fprintf(out, "\\r");
				linechar += 2;
			} else if (c == '\t') {
				color(out, COL_WHITESPACE);
				fprintf(out, "\\t");
				linechar += 2;
			} else {
				color(out, COL_BINARY);
				fprintf(out, "x%.2X", c);
				linechar += 3;
			}

			if (linechar >= 80) {
				fprintf(out, "\n");
				initline(out, bytenum);
				linechar = 0;
			}
		}

		if (cnt < sizeof(buf)) {
			fprintf(out, "\n");
			if (ferror(in)) {
				perror("read");
				return -1;
			} else {
				return 0;
			}
		}
	}
}

FILE *openfile(char *str) {
	if (strcmp(str, "-") == 0) {
		return stdin;
	} else {
		FILE *f = fopen(str, "r");
		if (f == NULL) {
			perror(str);
			return NULL;
		} else {
			return f;
		}
	}
}

int main(int argc, char **argv) {
	if (argc == 2 && (
			strcmp(argv[1], "-h") == 0 ||
			strcmp(argv[1], "--help") == 0)) {
		printf("Usage: %s [files]...\n", argv[0]);
		return EXIT_SUCCESS;
	}

	FILE *outfile = stdout;

	// If we get no arguments, or just one file as an argument,
	// just prettify that one file
	if (argc <= 2) {
		FILE *infile = argc == 1 ? stdin : openfile(argv[1]);
		if (infile == NULL) return EXIT_FAILURE;
		if (pretty(infile, outfile) < 0)
			return EXIT_FAILURE;

	// If we get more than one file,
	// loop trhough them, printing their names before their content
	} else {
		for (int i = 1; i < argc; ++i) {
			if (i == 0)
				printf("%s:\n", argv[i]);
			else
				printf("\n%s:\n", argv[i]);

			FILE *infile = openfile(argv[i]);
			if (infile == NULL) return EXIT_FAILURE;
			if (pretty(infile, outfile) < 0)
				return EXIT_FAILURE;
		}
	}
}
