#include "parse.h"

/* Read file contents into a text buffer */
char *
read_file (char *path);

/* Split a text buffer into lines */
char **
split_lines (char *buf, size_t *out_line_count);

void
parse_file (file_contents_t *file_contents, char *path)
{
    size_t line_count = 0;
    char *buf = read_file (path);

    char **lines = split_lines (buf, &line_count);

    file_contents->buf = buf;
    file_contents->lines = lines;
    file_contents->line_count = line_count;
}

char *
read_file (char *path)
{
    FILE *stream = fopen (path, "rb");
    if (stream == NULL)
        return NULL;
    fseek (stream, 0, SEEK_END);
    size_t file_size = ftell (stream);
    rewind (stream);

    char *buf = malloc (file_size + 1);
    fread (buf, 1, file_size, stream);
    buf[file_size] = '\0';

    fclose (stream);
    
    return buf;
}

char **split_lines(char *buf, size_t *out_line_count)
{
    size_t count = 0, capacity = 16;
    char **lines = malloc(capacity * sizeof(char *));
    char *p = buf;
    char *start = buf;

    while (*p)
    {
        if (*p == '\n')
        {
            *p = '\0';

            if (count == capacity)
            {
                capacity *= 2;
                lines = realloc(lines, capacity * sizeof(char *));
            }

            lines[count++] = start;   // store even empty lines
            start = p + 1;
        }
        p++;
    }

    /* Add final line (even if empty) */
    if (count == capacity)
    {
        capacity *= 2;
        lines = realloc(lines, capacity * sizeof(char *));
    }
    lines[count++] = start;

    *out_line_count = count;
    return lines;
}

void free_file_contents (file_contents_t *file_contents)
{
    if(file_contents == NULL)
        return;

    free (file_contents->buf);
    free (file_contents->lines);
}

void print_file_contents (file_contents_t *file_contents)
{
    if (file_contents == NULL)
        return;

    for (size_t i = 0; i < file_contents->line_count - 1; i++)
        printf ("%s\n", file_contents->lines[i]);
}






