#include "parse.h"
#include "diff.h"

static const char *edit_name(edit_t e)
{
    switch (e) {
        case ADD:       return "ADD";
        case EDIT:      return "EDIT";
        case DELETE:    return "DELETE";
        case UNCHANGED: return "UNCHANGED";
        default:        return "?";
    }
}

void print_edits(const vec_line_edit_t *v)
{
    for (size_t i = 0; i < v->size; i++) {
        const line_edit_t *e = &v->data[i];
        printf("[%s] base=%zu", edit_name(e->edit_type), e->base_index);
        if (e->line)
            printf(" line=\"%s\"", e->line);
        printf("\n");
    }
}

void
print_regions(vec_region_t *region_vec)
{
    for (size_t i = 0; i < region_vec->size; i++)
    {
        region_t *region = &region_vec->data[i];

        printf("===== REGION %zu =====\n", i);
        print_edits(&region->edits);
        printf("====================\n\n");
    }
}

int
main (int argc, char **argv)
{
    if (argc != 5)
    {
        printf ("Usage: ./main BASE.txt LOCAL.txt REMOTE.txt MERGED.txt!\n");
        return 0;
    }
    char *base_path = argv[1];
    char *local_path = argv[2];
    char *remote_path = argv[3];
    char *out_path = argv[4];

    file_contents_t base_file_contents =
    {
        .buf = NULL,
        .lines = NULL,
        .line_count = 0
    };

    file_contents_t local_file_contents =
    {
        .buf = NULL,
        .lines = NULL,
        .line_count = 0
    };

    file_contents_t remote_file_contents =
    {
        .buf = NULL,
        .lines = NULL,
        .line_count = 0
    };

    /* Parse files */
    parse_file (&base_file_contents, base_path);
    parse_file (&local_file_contents, local_path);
    parse_file (&remote_file_contents, remote_path);

    printf ("=== BASE CONTENTS ===\n");
    print_file_contents (&base_file_contents);
    printf ("=== LOCAL CONTENTS ===\n");
    print_file_contents (&local_file_contents);
//    printf ("=== REMOTE CONTENTS ===\n");
//    print_file_contents (&remote_file_contents);
//    printf ("===   ===\n");

    /* Diff files */
    vec_line_edit_t edits;
    vec_line_edit_init (&edits);

    longest_continuous_sequence (&base_file_contents, &local_file_contents, &edits);

    vec_region_t regions;
    vec_region_init (&regions);

    build_edit_groups (&edits, &regions);
    print_regions (&regions);

    /* Free resources */
    free_file_contents (&base_file_contents);
    free_file_contents (&local_file_contents);
    free_file_contents (&remote_file_contents);

    vec_line_edit_free(&edits);

    return 0;
}
