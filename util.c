//
// util.c
// utility functions
//
#include <GL/glew.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void fatalf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	abort();
}

const char *readfile(const char *path)
{
	FILE   *fp;
	char   *buffer;
	size_t  size;

	if (! (fp = fopen(path, "rb"))) {
		fatalf("error reading '%s'\n", path);
	}

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	rewind(fp);

	buffer = malloc(size + 1);

	fread(buffer, size, 1, fp);
	fclose(fp);

	buffer[size] = '\0';

	return buffer;
}

int freadstr(char **strp, FILE *fp)
{
	int len = fgetc(fp);

	if (len > 0) {
		*strp = malloc(len + 1);
		fread(*strp, len, 1, fp);
	}
	(*strp)[len] = '\0';

	return len;
}
