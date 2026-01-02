#pragma once

#include <stdbool.h>
#include <string.h>
#include "parse.h"

typedef enum
{
	ADD,
	EDIT,
	DELETE,
	UNCHANGED
} edit_t;

typedef enum
{
	REGION_INSERT,
	REGION_DELETE,
	REGION_UNCHANGED,
	REGION_REPLACE
} region_edit_t;


typedef struct
{
	size_t base_index;
	char *line;
	edit_t edit_type;

} line_edit_t;

typedef struct
{
	line_edit_t *data;
	size_t       size;
	size_t       capacity;
} vec_line_edit_t;

typedef struct
{
	size_t base_index;          // Start relative to base
	size_t base_lines_consumed;	// Number of base lines consumed by this region in the base file (how far to move forward relative to the base file)
	edit_t edit_type;
	vec_line_edit_t edits;
} region_t;

typedef struct
{
	region_t *data;
	size_t       size;
	size_t       capacity;
} vec_region_t;

typedef struct
{
	line_edit_t local_line;
	line_edit_t remote_line;
} merge_slot_t;


void
longest_continuous_sequence (file_contents_t *base, file_contents_t *other, vec_line_edit_t *out_vec);

merge_slot_t *
fill_merge_slot_table (vec_line_edit_t *local_edits, vec_line_edit_t *remote_edits);

void
build_edit_groups(const vec_line_edit_t *edits,
                  vec_region_t *out_groups);

void
vec_line_edit_init(vec_line_edit_t *v);

void
vec_line_edit_push(vec_line_edit_t *v, line_edit_t e);

line_edit_t *
vec_line_edit_find(vec_line_edit_t *v, int base_index);

void
vec_line_edit_free(vec_line_edit_t *v);

void
vec_line_edit_reverse(vec_line_edit_t *v);

void
vec_region_init(vec_region_t *v);

void
vec_region_push(vec_region_t *v, region_t e);

region_t *
vec_region_find(vec_region_t *v, int base_index);

void
vec_region_free(vec_region_t *v);

void
vec_region_reverse(vec_region_t *v);
