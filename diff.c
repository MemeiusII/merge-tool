#include "diff.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

void
longest_continuous_sequence (file_contents_t *base, file_contents_t *other, vec_line_edit_t *out_vec)
{
    int dp_table[base->line_count + 1][other->line_count + 1];
    memset(dp_table, 0, sizeof dp_table);

    /* Build Dynamic Programming Table */
    for (size_t i = 1; i <= base->line_count; i++)
    {
        for (size_t j = 1; j <= other->line_count; j++)
        {
            if (strcmp (base->lines[i - 1], other->lines[j - 1]) == 0)
                dp_table[i][j] = dp_table[i - 1][j - 1] + 1; 
            else
                dp_table[i][j] = MAX (dp_table[i - 1][j], dp_table[i][j - 1]);
        }
    }

    /* Back track */
    size_t i = base->line_count;
    size_t j = other->line_count;

    while (i > 0 && j > 0) {


        if (strcmp(base->lines[i - 1], other->lines[j - 1]) == 0) {
//            printf("UNCHANGED: %s\n", base->lines[i - 1]);
            line_edit_t edit = {
                .base_index = i - 1,
                .line = base->lines[i - 1],
                .edit_type = UNCHANGED
            };
            vec_line_edit_push (out_vec, edit);
            i--;
            j--;

        } else if (dp_table[i - 1][j] >= dp_table[i][j - 1]) {
//            printf("DELETE: %s\n", base->lines[i - 1]);
            line_edit_t edit = {
                .base_index = i - 1,
                .line = base->lines[i - 1],
                .edit_type = DELETE
            };
            vec_line_edit_push (out_vec, edit);
            i--;

        } else {
//            printf("ADD: %s\n", other->lines[j - 1]);
            line_edit_t edit = {
                .base_index = i,
                .line = other->lines[j - 1],
                .edit_type = ADD
            };
            vec_line_edit_push (out_vec, edit);
            j--;
        }
    }

    /* Remaining deletions */
    while (i > 0) {
//        printf("DELETE: %s\n", base->lines[i - 1]);
        line_edit_t edit = {
            .base_index = i - 1,
            .line = base->lines[i - 1],
            .edit_type = DELETE
        };
        vec_line_edit_push (out_vec, edit);
        i--;
    }

    /* Remaining insertions */
    while (j > 0) {
//        printf("ADD: %s\n", other->lines[j - 1]);
        line_edit_t edit = {
            .base_index = i,
            .line = other->lines[j - 1],
            .edit_type = ADD
        };
        vec_line_edit_push (out_vec, edit);
        j--;
    }

    // Reverse out vector
    vec_line_edit_reverse (out_vec);
}

static bool
edit_consumes_base(edit_t type)
{
    return type == UNCHANGED || type == DELETE;
}

static bool
can_extend_group(const region_t *r, const line_edit_t *e)
{
    size_t expected_base = r->base_index + r->base_lines_consumed;

    /* Base contiguity check */
    if (edit_consumes_base(e->edit_type))
    {
        if (e->base_index != expected_base)
            return false;
    }
    else
    {
        if (e->base_index != expected_base)
            return false;
    }

    /* Semantic compatibility */
    switch (r->edit_type)
    {
        case ADD:
            return e->edit_type == ADD;

        case DELETE:
            return e->edit_type == DELETE ||
                   e->edit_type == ADD; /* replacement */

        case UNCHANGED:
            return e->edit_type == UNCHANGED;

        default:
            return false;
    }
}

void
build_edit_groups(const vec_line_edit_t *edits,
                  vec_region_t *out_groups)
{
    if (out_groups == NULL)
    {
        fprintf (stderr, "build_edit_groups: out_groups vector must be initialized!\n");
        return;
    }

    region_t current = {0};
    bool has_current = false;

    for (size_t i = 0; i < edits->size; i++)
    {
        const line_edit_t *e = &edits->data[i];

        if (!has_current)
        {
            /* Start first group */
            current.base_index          = e->base_index;
            current.base_lines_consumed = 0;
            current.edit_type           = e->edit_type;
            vec_line_edit_init(&current.edits);
            has_current = true;
        }
        else if (!can_extend_group(&current, e))
        {
            /* Close current group */
            vec_region_push(out_groups, current);

            /* Start new group */
            current.base_index          = e->base_index;
            current.base_lines_consumed = 0;
            current.edit_type           = e->edit_type;
            vec_line_edit_init(&current.edits);
        }

        vec_line_edit_push(&current.edits, *e);

        if (edit_consumes_base(e->edit_type))
            current.base_lines_consumed++;
    }

    if (has_current)
        vec_region_push(out_groups, current);
}

/* Vec line edit */
void 
vec_line_edit_init(vec_line_edit_t *v)
{
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

static void 
vec_line_edit_grow(vec_line_edit_t *v)
{
    size_t new_cap = (v->capacity == 0) ? 8 : v->capacity * 2;

    line_edit_t *new_data =
        realloc(v->data, new_cap * sizeof(line_edit_t));

    if (!new_data) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }

    v->data = new_data;
    v->capacity = new_cap;
}

void
vec_line_edit_push(vec_line_edit_t *v, line_edit_t e)
{
    if (v->size == v->capacity)
        vec_line_edit_grow(v);

    v->data[v->size++] = e;
}

line_edit_t *
vec_line_edit_find(vec_line_edit_t *v, int base_index)
{
    for (size_t i = 0; i < v->size; i++) {
        if (v->data[i].base_index == base_index)
            return &v->data[i];
    }
    return NULL;
}

void 
vec_line_edit_free(vec_line_edit_t *v)
{
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

void
vec_line_edit_reverse(vec_line_edit_t *v)
{
    if (!v || v->size < 2)
        return;

    size_t i = 0;
    size_t j = v->size - 1;

    while (i < j) {
        line_edit_t tmp = v->data[i];
        v->data[i] = v->data[j];
        v->data[j] = tmp;
        i++;
        j--;
    }
}

/* Vec region */
void 
vec_region_init(vec_region_t *v)
{
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

static void 
vec_region_grow(vec_region_t *v)
{
    size_t new_cap = (v->capacity == 0) ? 8 : v->capacity * 2;

    region_t *new_data =
        realloc(v->data, new_cap * sizeof(region_t));

    if (!new_data) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }

    v->data = new_data;
    v->capacity = new_cap;
}

void
vec_region_push(vec_region_t *v, region_t e)
{
    if (v->size == v->capacity)
        vec_region_grow(v);

    v->data[v->size++] = e;
}

region_t *
vec_region_find(vec_region_t *v, int base_index)
{
    for (size_t i = 0; i < v->size; i++) {
        if (v->data[i].base_index == base_index)
            return &v->data[i];
    }
    return NULL;
}

void
vec_region_free(vec_region_t *v)
{
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

void
vec_region_reverse(vec_region_t *v)
{
    if (!v || v->size < 2)
        return;

    size_t i = 0;
    size_t j = v->size - 1;

    while (i < j) {
        region_t tmp = v->data[i];
        v->data[i] = v->data[j];
        v->data[j] = tmp;
        i++;
        j--;
    }
}
