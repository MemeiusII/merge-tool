#pragma once

#include <stdio.h>
#include <stdlib.h>

typedef struct 
{
	char *buf;
	char **lines;
	size_t line_count;
} file_contents_t;

/* Get parsed file contents */
void
parse_file (file_contents_t *file_contents, char *path);

void
free_file_contents (file_contents_t *file_contents);

void
print_file_contents (file_contents_t *file_contents);
