#include <stdlib.h>
#include <assert.h>

#include "command.h"

static size_t cmdReadArg(char *dest, char *src, size_t len);

int cmdFromString(struct command *cmd, char *str, size_t len) {
	char *strp = str;
	cmd->argc = 0;

	while (strp - str < len && cmd->argc < CMD_MAX_ARGS) {
		strp += cmdReadArg(cmd->argv[cmd->argc++], strp, MAX_ARG_LEN);
	}
	return 0;
}

static int isspace(char c)
{
	return c == ' ' || c == '\n' || c == '\t';
}

static size_t cmdReadStringArg(char *dest, char *src, size_t len)
{
	char *out = dest;

	*out++ = *src++;

	while (out - dest < len) {
		*out++ = *src++;
		if (*src == '"')
			break;
	}
	*out++ = '"';
	*out   = '\0';

	return out - dest;
}

static size_t cmdReadDelimArg(char *dest, char *src, size_t len, char delim)
{
	char *out = dest;

	while (out - dest < len) {
		*out++ = *src++;
		if (*src == delim)
			break;
	}
	*out++ = delim;
	*out   = '\0';

	return out - dest;
}

static size_t cmdReadWordArg(char *dest, char *src, size_t len)
{
	char *out = dest;

	while (out - dest < len) {
		*out++ = *src++;
		if (*src == ' ')
			break;
	}
	*out = '\0';

	return out - dest;
}

static size_t cmdReadArg(char *dest, char *src, size_t len)
{
	size_t n = 0;

	assert(dest);
	assert(src);
	assert(len > 0);

	switch (src[0]) {
		case '"': n = cmdReadStringArg(dest, src, len);     break;
		case '{': n = cmdReadDelimArg(dest, src, len, '}'); break;
		case '(': n = cmdReadDelimArg(dest, src, len, ')'); break;
		case '[': n = cmdReadDelimArg(dest, src, len, ']'); break;
		default:  n = cmdReadWordArg(dest, src, len);       break;
	}
	src += n;

	while (isspace(*src++)) // skip trailing whitespace
		n++;

	return n;
}
