/*
 *
 * lourland
 * tga.c
 *
 */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "tga.h"

#define TGA_TYPE_UNCOMPRESSED_RGB 2

struct pixel {
	unsigned char r, g, b, a;
};

bool tgaDecode(struct tga *t, const char *path)
{
	FILE *fp = fopen(path, "rb");

	if (!fp)
		return NULL;

	fread(&t->header.idlen, 1, 1, fp);
	fread(&t->header.colormaptype, 1, 1, fp);
	fread(&t->header.imagetype, 1, 1, fp);
	fread(&t->header.colormapoff, 2, 1, fp);
	fread(&t->header.colormaplen, 2, 1, fp);
	fread(&t->header.colormapdepth, 1, 1, fp);
	fread(&t->header.x, 2, 1, fp);
	fread(&t->header.y, 2, 1, fp);
	fread(&t->width, 2, 1, fp);
	fread(&t->height, 2, 1, fp);
	fread(&t->depth, 1, 1, fp);
	fread(&t->header.imagedesc, 1, 1, fp);

	printf("%dx%dx%d, type %d\n", t->width, t->height, t->depth, t->header.imagetype);

	if (t->header.imagetype != TGA_TYPE_UNCOMPRESSED_RGB) {
		return false;
	}

	t->data = malloc(sizeof(struct pixel) * t->width * t->height);
	fread(t->data, t->depth/8, t->width * t->height, fp);
	fclose(fp);

	return true;
}


int tgaEncode(uint32_t *pixels, short w, short h, char depth, const char *path)
{
	FILE *fp = fopen(path, "wb");

	if (!fp)
		return 1;

	short null = 0x0;

	fputc(0, fp); // ID length
	fputc(0, fp); // No color map
	fputc(2, fp); // Uncompressed 32-bit RGBA

	fwrite(&null, 2, 1, fp);  // Color map offset
	fwrite(&null, 2, 1, fp);  // Color map length
	fwrite(&null, 1, 1, fp);  // Color map entry size
	fwrite(&null, 2, 1, fp);  // X
	fwrite(&null, 2, 1, fp);  // Y
	fwrite(&w, 2, 1, fp);     // Width
	fwrite(&h, 2, 1, fp);     // Height
	fwrite(&depth, 1, 1, fp); // Depth
	fwrite(&null, 1, 1, fp);  // Image descriptor

	int bytes = depth / 8;
	char p[4];

	for (int i = 0; i < w * h; i++) {
		p[0] = ((struct pixel *)&pixels[i])->b;
		p[1] = ((struct pixel *)&pixels[i])->g;
		p[2] = ((struct pixel *)&pixels[i])->r;
		p[3] = ((struct pixel *)&pixels[i])->a;

		if (!fwrite(p, bytes, 1, fp)) {
			fprintf(stderr, "error: unexpected EOF at offset %d\n", i);
			exit(-1);
		}
	}
	fclose(fp);

	return 0;
}

void tgaFreeImageData(struct tga *t)
{
	free(t->data);
}

