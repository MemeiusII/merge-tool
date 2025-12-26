#pragma once

#include <string.h>
#include "parse.h"

typedef enum
{
	ADD,
	EDIT,
	DELETE
} edit_t;

typedef struct
{
	size_t base_index;
	char *line;
	edit_t edit_type;

} line_edit_t;

typedef struct
{
	line_edit_t local_line;
	line_edit_t remote_line;
} merge_slot_t;

typedef struct
{
	line_edit_t *data;
	size_t       size;
	size_t       capacity;
} vec_line_edit_t;

void
longest_continuous_sequence (file_contents_t *base, file_contents_t *other, vec_line_edit_t *out_vec);

merge_slot_t *
fill_merge_slot_table (vec_line_edit_t *local_edits, vec_line_edit_t *remote_edits);

void
vec_line_edit_init(vec_line_edit_t *v);

void
vec_line_edit_push(vec_line_edit_t *v, line_edit_t e);

line_edit_t *
vec_line_edit_find(vec_line_edit_t *v, int base_index);

void
vec_line_edit_free(vec_line_edit_t *v);
