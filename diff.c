#include "diff.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

void
longest_continuous_sequence (file_contents_t *base, file_contents_t *other, vec_line_edit_t *out_vec)
{
    int dp_table[base->line_count + 1][other->line_count + 1];
    memset(dp_table, 0, sizeof dp_table);

    /* Build Dynamic Programming Table */
    for (size_t i = 1; i < base->line_count; i++)
    {
        for (size_t j = 1; j < base->line_count; j++)
        {
            if (strcmp (base->lines[i - 1], other->lines[j - 1]) == 0)
                dp_table[i][j] += dp_table[i - 1][j - 1] + 1; 
            else
                dp_table[i][j] = MAX (dp_table[i - 1][j], dp_table[i][j - 1]);
        }
    }

    /* Back track */
    size_t i = base->line_count;
    size_t j = other->line_count;

    while (i > 0 && j > 0) {


        if (strcmp(base->lines[i - 1], other->lines[j - 1]) == 0) {
            printf("UNCHANGED: %s\n", base->lines[i - 1]);
            i--;
            j--;

        } else if (dp_table[i - 1][j] >= dp_table[i][j - 1]) {
            printf("DELETE: %s\n", base->lines[i - 1]);
            line_edit_t edit = {
                .base_index = i - 1,
                .line = base->lines[i - 1],
                .edit_type = DELETE
            };
            vec_line_edit_push (out_vec, edit);
            i--;

        } else {
            printf("ADD: %s\n", other->lines[j - 1]);
            line_edit_t edit = {
                .base_index = i,
                .line = base->lines[j - 1],
                .edit_type = ADD
            };
            vec_line_edit_push (out_vec, edit);
            j--;
        }
    }

    /* Remaining deletions */
    while (i > 0) {
        printf("DELETE: %s\n", base->lines[i - 1]);
        line_edit_t edit = {
            .base_index = i,
            .line = base->lines[j - 1],
            .edit_type = DELETE
        };
        vec_line_edit_push (out_vec, edit);
        i--;
    }

    /* Remaining insertions */
    while (j > 0) {
        printf("ADD: %s\n", other->lines[j - 1]);
        line_edit_t edit = {
            .base_index = i,
            .line = base->lines[j - 1],
            .edit_type = ADD
        };
        vec_line_edit_push (out_vec, edit);
        j--;
    }

}

void
detect_conflicts (file_contents_t *base, vec_line_edit_t *local_edits, vec_line_edit_t *remote_edits)
{
    merge_slot_t ms_table[base->line_count];
    memset (ms_table, 0, sizeof (ms_table));

    /* Fill merge table with edits from LOCAL and REMOTE matched to the base index */
    for (size_t i = 0; i < local_edits->size - 1; i++)
    {
        line_edit_t edit = local_edits->data[i];
        ms_table[edit.base_index].local_line = edit;
    }
    for (size_t i = 0; i < remote_edits->size - 1; i++)
    {
        line_edit_t edit = remote_edits->data[i];
        ms_table[edit.base_index].remote_line = edit;
    }

    /* Compare merge slots and detect conflicts between LOCAL and REMOTE edits */

}

void vec_line_edit_init(vec_line_edit_t *v)
{
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

static void vec_line_edit_grow(vec_line_edit_t *v)
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

void vec_line_edit_push(vec_line_edit_t *v, line_edit_t e)
{
    if (v->size == v->capacity)
        vec_line_edit_grow(v);

    v->data[v->size++] = e;
}

line_edit_t *vec_line_edit_find(vec_line_edit_t *v, int base_index)
{
    for (size_t i = 0; i < v->size; i++) {
        if (v->data[i].base_index == base_index)
            return &v->data[i];
    }
    return NULL;
}

void vec_line_edit_free(vec_line_edit_t *v)
{
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}
