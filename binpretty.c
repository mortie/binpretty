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
		fputs(col, out);
	}
}

static void initline(FILE *out, size_t bytenum) {
	color(out, COL_RESET);
	fprintf(out, "%.8X ", (unsigned int)bytenum);
}

static int pretty(FILE *in, FILE *out, size_t linewidth) {
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

			if (linechar >= linewidth) {
				fprintf(out, "\n");
				initline(out, bytenum);
				linechar = 0;
			}
		}

		if (cnt < sizeof(buf)) {
			color(out, COL_RESET);
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
	int linewidth = 80;
	int use_pager = 0;

	int i;
	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			printf("Usage: %s [options] [files]...\n", argv[0]);
			printf("Options:\n");
			printf("    -h, --help:      Show this help text\n");
			printf("    --linewidth [n]: Line width\n");
			printf("    --pager:         Show in pager\n");
			return 0;
		} else if (strcmp(argv[i], "--linewidth") == 0 && i < argc - 1) {
			linewidth = atoi(argv[++i]);
		} else if (strcmp(argv[i], "--pager") == 0) {
			use_pager = 1;
		} else if (strcmp(argv[i], "--") == 0 || argv[i][0] != '-') {
			break;
		} else {
			fprintf(stderr, "Unknown argument: '%s'\n", argv[i]);
			return 1;
		}
	}

	char **files = argv + i;
	size_t filecount = argc - i ;

	FILE *outfile = stdout;

	// Show pager?
	if (use_pager) {
		const char *pager = getenv("PAGER");
		if (pager == NULL || pager[0] == '\0') {
			pager = "less -R";
		}

		outfile = popen(pager, "w");
		if (outfile == NULL) {
			perror(pager);
			return 1;
		}
	}

	// If we get no arguments, or just one file as an argument,
	// just prettify that one file
	if (filecount <= 1) {
		FILE *infile = filecount == 0 ? stdin : openfile(files[0]);
		if (infile == NULL) return EXIT_FAILURE;
		if (pretty(infile, outfile, linewidth) < 0)
			return EXIT_FAILURE;

	// If we get more than one file,
	// loop trhough them, printing their names before their content
	} else {
		for (size_t i = 0; i < filecount; ++i) {
			if (i == 0)
				printf("%s:\n", files[i]);
			else
				printf("\n%s:\n", files[i]);

			FILE *infile = openfile(files[i]);
			if (infile == NULL) return EXIT_FAILURE;
			if (pretty(infile, outfile, linewidth) < 0)
				return EXIT_FAILURE;
		}
	}

	if (use_pager) {
		pclose(outfile);
	} else {
		fclose(outfile);
	}

	return 0;
}
