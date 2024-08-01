#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MAX_LINE_LENGTH 526
#define MAX_LINES 10000

#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

/*
    MODIFY CAREFULLY
*/

int hasname = 0;
char file_name[1024];
int cline = 0;
char lines[MAX_LINES][MAX_LINE_LENGTH];
char paste[MAX_LINE_LENGTH];
int has_changes = 0;

int process_command(const char *command);
void insert_line(int line, const char *text);

int getsize()
{
    int size = 0;
    for (int line = 0; line < MAX_LINES; line++)
    {
        if (strlen(lines[line]) == 0)
        {
            continue;
        }
        size += strlen(lines[line]) + 1;
    }
    return size;
}

char *displaySize(int size)
{
    double convertedSize;
    const char *unit;

    if (size >= 1024 * 1024 * 1024)
    {
        convertedSize = (double)size / (1024 * 1024 * 1024);
        unit = "GB";
    }
    else if (size >= 1024 * 1024)
    {
        convertedSize = (double)size / (1024 * 1024);
        unit = "MB";
    }
    else if (size >= 1024)
    {
        convertedSize = (double)size / 1024;
        unit = "KB";
    }
    else
    {
        convertedSize = (double)size;
        unit = "bytes";
    }
    char text[MAX_LINE_LENGTH];
    snprintf(text, MAX_LINE_LENGTH, "Size: %.2f %s", convertedSize, unit);
    return text;
}

void append_line(const char *text)
{
    if (cline < MAX_LINES)
    {
        strncpy(lines[cline++], text, MAX_LINE_LENGTH);
    }
    else
    {
        printf(RED "Error: Maximum number of lines reached.\n" RESET);
    }
}

void into_a()
{
    char text[MAX_LINE_LENGTH];
    printf("Type '.' to finish.\n");
    while (1)
    {
        printf("%04d: ", cline + 1);
        fgets(text, sizeof(text), stdin);
        if (strcmp(text, ".\n") == 0)
        {
            break;
        }
        text[strlen(text) - 1] = '\0';
        strcpy(lines[cline++], text);
    }
}

void into_i(int line)
{
    char text[MAX_LINE_LENGTH];
    printf("Type '.' to finish.\n");
    while (1)
    {
        printf("%04d: ", line);
        fgets(text, sizeof(text), stdin);
        if (strcmp(text, ".\n") == 0)
        {
            break;
        }
        text[strlen(text) - 1] = '\0';
        insert_line(line++, text);
    }
}

void print_lines()
{
    for (int p = 0; p < cline; p++)
    {
        printf("%04d: %s\n", p + 1, lines[p]);
    }
}

void print_lines_raw()
{
    for (int p = 0; p < cline; p++)
    {
        printf("%s\n", lines[p]);
    }
}

void delete_line(int line)
{
    if (line <= cline && line > 0)
    {
        strcpy(paste, lines[line - 1]);
        for (int p = line; p < cline; p++)
        {
            strncpy(lines[p - 1], lines[p], MAX_LINE_LENGTH);
        }
        cline--;
    }
    else
    {
        printf(RED "Error: Line number out of range.\n" RESET);
    }
}

void rdelete_lines(int start, int end)
{
    if (start < 1 || start > cline || end < start)
    {
        printf(RED "Error: Invalid line range.\n" RESET);
        return;
    }

    if (end > cline)
    {
        end = cline;
    }

    int lines_to_delete = end - start + 1;
    for (int i = start - 1; i + lines_to_delete < cline; i++)
    {
        strncpy(lines[i], lines[i + lines_to_delete], MAX_LINE_LENGTH);
    }

    cline -= lines_to_delete;
}

void delete_lines()
{
    for (int p = 0; p < MAX_LINES; p++)
        memset(lines[p], 0, MAX_LINE_LENGTH);
    cline = 0;
}

char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf(RED "Error opening file\n" RESET);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL)
    {
        printf(RED "Error allocating memory" RESET);
        fclose(file);
        return NULL;
    }
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);
    return buffer;
}

void load_file(const char *filename)
{
    delete_lines();

    char *file_contents = read_file(filename);
    if (file_contents == NULL)
    {
        return;
    }

    char *line = strtok(file_contents, "\n");
    while (line != NULL && cline < 1000)
    {
        strncpy(lines[cline++], line, MAX_LINE_LENGTH);
        line = strtok(NULL, "\n");
    }

    printf(GREEN "Loaded file!\n%s\n" RESET, displaySize(getsize()));

    free(file_contents);
}

int run_file(const char *filename)
{
    char *file_contents = read_file(filename);
    if (file_contents == NULL)
    {
        return 1;
    }

    char *line = strtok(file_contents, "\n");
    char test[MAX_LINE_LENGTH];
    strcpy(test, "run ");
    strcat(test, filename);
    while (line != NULL)
    {
        if (strcmp(test, line) == 0)
        {
            printf(RED "Cannot recurse!\n" RESET);
            free(file_contents);
            return 1;
        }
        int res = process_command(line);
        if (res == 1)
        {
            free(file_contents);
            return 1;
        }
        else if (res == 2)
        {
            break;
        }
        line = strtok(NULL, "\n");
    }

    free(file_contents);
    return 0;
}

int save_file(const char *filename)
{
    FILE *file = fopen(filename, "w");
    int size = 0;
    if (file == NULL)
    {
        perror(RED "Error opening file for writing" RESET);
        return 1;
    }

    for (int p = 0; p < cline; p++)
    {
        fprintf(file, "%s\n", lines[p]);
        size += strlen(lines[p]) + 1;
    }

    fclose(file);
    printf(GREEN "File saved successfully.\n%s\n" RESET, displaySize(getsize()));
    return 0;
}

void insert_line(int line, const char *text)
{
    if (line > 0 && line <= cline + 1 && cline < MAX_LINES)
    {
        for (int p = cline; p >= line; p--)
        {
            strncpy(lines[p], lines[p - 1], MAX_LINE_LENGTH);
        }
        strncpy(lines[line - 1], text, MAX_LINE_LENGTH);
        cline++;
    }
    else
    {
        printf("Error: Line number out of range or maximum lines reached.\n");
    }
}

void myprint()
{
    int r = 0;
    for (int p = 0; p < cline; p++)
    {
        r = 0;
        printf("%04d: %s\n", p + 1, lines[p]);
        while (p + 1 < cline && strcmp(lines[p + 1], lines[p]) == 0)
        {
            p++;
            r++;
        }
        if (r)
        {
            printf("Repeats %i times...\n", r);
        }
    }
}

int process_command(const char *command)
{
    if (strncmp(command, "@", 1) == 0)
    {
        return 0;
    }
    else if (strncmp(command, "out ", 4) == 0)
    {
        printf("%s\n", command + 4);
    }
    else if (strncmp(command, "repeat ", 7) == 0)
    {
        int iter;
        int text[MAX_LINE_LENGTH];
        if (sscanf(command, "repeat %d %[^\n]", &iter, text) == 2)
        {
            for (int p = 0; p < iter; p++)
                if (process_command(text))
                    break;
        }
    }
    else if (strncmp(command, "h", 1) == 0 && strlen(command) == 1)
    {
        printf(
            GREEN
            "EdIm (Ed Improved) v0.1  - by Darren Chase Papa\n"
            "a [text]                 - Append text\n"
            "a.                       - Insert an empty line.\n"
            "a                        - Traditional ed 'a' command.\n"
            "sl [scr] [dest]          - Switch the two lines.\n"
            "p [line]                 - Print all lines.\n"
            "P [line]                 - Like `p`. Removes similar ones.\n"
            "p                        - Print the whole buffer.\n"
            "praw                     - Like `p` but has no line numbers.\n"
            "pr [start] [end]         - Print the lines start to end.\n"
            "pr                       - Print the last 10 lines.\n"
            "r [line] [text]          - Replace line with text.\n"
            "rc [line] [index] [char] - Replace a character.\n"
            "rs [line] [index] [text] - Replace a section of a line.\n"
            "i [line] [text]          - Insert a line.\n"
            "ii [line]                - Inserts multiple lines.\n"
            "d [line]                 - Remove a line.\n"
            "d                        - Delete the entire buffer.\n"
            "l [file]                 - Load a file.\n"
            "s [file]                 - Save to the file.\n"
            "s                        - Save to a set name.\n"
            "ssn [file]               - Set save name.\n"
            "rsn                      - Remove save name.\n"
            "%% [command]              - Runs command in shell.\n"
            "lines                    - Print current line.\n"
            "name                     - Print current save name.\n"
            "ps                       - Paste string.\n"
            "ps [line]                - Paste into the line.\n"
            "cp [line]                - Copy line.\n"
            "cps                      - Clear the paste buffer.\n"
            "psi [line]               - Insert the paste buffer to `line`\n"
            "sps [text]               - Set the paste buffer.\n"
            "ps-buffer                - Print the paste buffer.\n"
            "q                        - Exit.\n"
            "== EdIm Script Stuff (.ed files) ==\n"
            "run [file]               - Runs the file as commands.\n"
            "@                        - Comment.\n"
            "out [text]               - Prints text.\n"
            "\"info\" for more info.\n"
            "\"ed-info\" for the current edim session.\n" RESET);
    }
    else if (strncmp(command, "info", 4) == 0 && strlen(command) == 4)
    {
        printf(
            GREEN
            "EdIm (Ed Improved) v0.1  - by Darren Chase Papa\n\n"
            "EdIm is like Vim is for Vi. But we wont replace ed soon.\n"
            "EdIm is just a little better than ed.\n"
            "It has more (maybe) concise commands.\n"
            "Ed is like ed this \"a\" type \".\" and thats it.\n"
            "Ed is a fast way to write code,\n"
            "EdIm is for longer use and not only for a few key strokes.\n"
            "\nHappy EdIm-ing!\n" RESET);
    }
    else if (strncmp(command, "ed-info", 7) == 0 && strlen(command) == 7)
    {
        printf(
            GREEN
            "          Version: v0.1\n\n"
            "        Max lines: %04d KB - %06d Lines\n"
            "  Max line length: %04d KB - %06d Chars\n"
            "Paste buffer size: %04d KB - %06d Chars\n"
            "      Buffer size: %.2f MB\n"
            "     Total memory: %.2f~ MB\n" RESET,
            MAX_LINES / 1024, MAX_LINES,
            MAX_LINE_LENGTH / 1024, MAX_LINE_LENGTH,
            MAX_LINE_LENGTH / 1024, MAX_LINE_LENGTH,
            ((float)(MAX_LINES * MAX_LINE_LENGTH) / 1024 / 1024),
            (float)(MAX_LINE_LENGTH / 1024, MAX_LINE_LENGTH + (MAX_LINES * MAX_LINE_LENGTH)) / 1024 / 1024);
    }
    else if (strncmp(command, "a ", 2) == 0)
    {
        append_line(command + 2);
    }
    else if (strncmp(command, "a.", 2) == 0 && strlen(command) == 2)
    {
        cline++;
    }
    else if (strncmp(command, "r ", 2) == 0)
    {
        int line;
        char text[MAX_LINE_LENGTH];
        int argc = sscanf(command, "r %d :%[^\n]", &line, text);
        if (!(line > 0 && line <= cline))
        {
            printf(RED "Error: Invalid line number or text.\n" RESET);
            return 1;
        }
        if (argc == 2)
        {
            strncpy(lines[line - 1], text, MAX_LINE_LENGTH);
        }
        else if (argc == 1)
        {
            strncpy(lines[line - 1], "", MAX_LINE_LENGTH);
        }
        else
        {
            printf(RED "Error: Invalid line number or text.\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "rc ", 2) == 0)
    {
        int line, index;
        char text;
        if (sscanf(command, "rc %d %d %c", &line, &index, &text) == 3 && line > 0 && line <= cline && index >= 0 && index - 1 < strlen(lines[line - 1]))
        {
            lines[line - 1][index] = text;
        }
        else
        {
            printf(RED "Error: Invalid line number or index or text.\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "rs ", 3) == 0)
    {
        int line, index;
        char text[MAX_LINE_LENGTH];
        if (sscanf(command, "rs %d %d :%s", &line, &index, &text) == 3 && line > 0 && line <= cline && index >= 0 && index < MAX_LINE_LENGTH)
        {
            for (int p = 0; p < strlen(lines[line - 1]) && p < strlen(text); p++)
            {
                lines[line - 1][index++] = text[p];
            }
        }
        else
        {
            printf(RED "Error: Invalid line number or index or text.\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "i ", 2) == 0)
    {
        int line;
        char text[MAX_LINE_LENGTH];
        int argc = sscanf(command, "i %d :%[^\n]", &line, text);
        if (argc == 2)
        {
            insert_line(line, text);
        }
        else if (argc == 1)
        {
            insert_line(line, "");
        }
        else
        {
            printf(RED "Error: Invalid line number or text.\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "ii ", 3) == 0)
    {
        int line;
        if (sscanf(command, "ii %d", &line) == 1)
        {
            into_i(line);
        }
        else
        {
            printf(RED "Error: Invalid line number or text.\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "pr ", 3) == 0)
    {
        int start, end;
        if (sscanf(command, "pr %d %d", &start, &end) == 2)
        {
            start--;
            for (; start < end && start < MAX_LINES; start++)
            {
                printf("%04d: %s\n", start + 1, lines[start]);
            }
        }
        else
        {
            printf(RED "Error: Invalid line number or text.\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "sl ", 3) == 0)
    {
        int scr, dest;
        if (sscanf(command, "sl %d %d", &scr, &dest) == 2)
        {
            char temp[MAX_LINE_LENGTH];
            strcpy(temp, lines[scr - 1]);
            strcpy(lines[scr - 1], lines[dest - 1]);
            strcpy(lines[dest - 1], temp);
        }
        else
        {
            printf(RED "Error: Invalid line number.\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "pr", 2) == 0 && strlen(command) == 2)
    {
        if (cline - 10 > 0)
        {
            for (int start = cline - 11; start < cline && start < MAX_LINES; start++)
            {
                printf("%04d: %s\n", start + 1, lines[start]);
            }
        }
        else
        {
            int st = 10;
            while (cline - st < 0 && st < MAX_LINES)
                st--;
            for (int start = cline - st; start < cline && start < MAX_LINES; start++)
            {
                printf("%04d: %s\n", start + 1, lines[start]);
            }
        }
    }
    else if (strncmp(command, "p ", 2) == 0)
    {
        int line = atoi(command + 2);
        if (line > 0 && line - 1 <= cline)
        {
            printf("%04d: %s\n", line, lines[line - 1]);
        }
        else
        {
            printf(RED "Error: Line number out of range.\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "pi ", 3) == 0)
    {
        int line = atoi(command + 3);
        if (line > 0 && line - 1 <= cline)
        {
            for (int index = 0; index < strlen(lines[line - 1]); index++)
            {
                printf("%04d: %c\n", index, lines[line - 1][index]);
            }
        }
        else
        {
            printf(RED "Error: Line number out of range.\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "p", 1) == 0 && strlen(command) == 1)
    {
        print_lines();
    }
    else if (strncmp(command, "praw", 4) == 0 && strlen(command) == 4)
    {
        print_lines_raw();
    }
    else if (strncmp(command, "P", 1) == 0 && strlen(command) == 1)
    {
        myprint();
    }
    else if (strncmp(command, "d ", 2) == 0)
    {
        int line = atoi(command + 2);
        delete_line(line);
    }
    else if (strncmp(command, "dr ", 3) == 0)
    {
        int start, end;
        if (sscanf(command, "dr %d %d", &start, &end) == 2)
        {
            rdelete_lines(start, end);
        }
        else
        {
            printf(RED "oh no: %s\n" RESET, command);
        }
    }
    else if (strncmp(command, "d", 1) == 0 && strlen(command) == 1)
    {
        delete_lines();
    }
    else if (strncmp(command, "b", 1) == 0 && strlen(command) == 1)
    {
        delete_line(cline);
    }
    else if (strncmp(command, "l ", 2) == 0)
    {
        load_file(command + 2);
    }
    else if (strncmp(command, "ssn ", 4) == 0)
    {
        strcpy(file_name, command + 4);
        hasname = 1;
    }
    else if (strncmp(command, "s", 1) == 0 && strlen(command) == 1)
    {
        if (hasname)
        {
            if (!save_file(file_name))
            {
                has_changes = 0;
                return 0;
            }
        }
        else
        {
            printf(RED "No file set.\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "s ", 2) == 0)
    {
        if (!save_file(command + 2))
        {
            has_changes = 0;
            return 0;
        }
    }
    else if (strncmp(command, "q", 1) == 0 && strlen(command) == 1)
    {
        if (!has_changes)
        {
            return 2;
        }
        printf("Quit without saving? [y/n]\n");
        char key;
        while ((key = _getch()) != 'y')
        {
            switch (key)
            {
            case 'x':
                if (hasname)
                {
                    if (save_file(file_name))
                        return 1;
                    return 2;
                }
                else
                {
                    printf("File (* to cancel): ");
                    fgets(file_name, sizeof(file_name), stdin);
                    file_name[strcspn(file_name, "\n")] = '\0';
                    if (!strcmp(file_name, "*") == 0)
                    {
                        if (save_file(file_name))
                            return 1;
                        return 2;
                    }
                }
                break;
            case 'c':
                return 0;
            case 'n':
                return 0;
            default:
                printf(
                    "x - Save and quit.\n"
                    "c or n - Cancel.\n"
                    "y - Dont save.\n");
                break;
            }
        }
        return 2;
    }
    else if (strncmp(command, "% ", 2) == 0)
    {
        system(command + 2);
    }
    else if (strncmp(command, "lines", 5) == 0 && strlen(command) == 5)
    {
        printf("Current line: %d\n", cline);
    }
    else if (strncmp(command, "ps", 2) == 0 && strlen(command) == 2)
    {
        if (strlen(paste))
        {
            append_line(paste);
        }
        else
        {
            printf(RED "Paste buffer is empty!\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "ps ", 3) == 0)
    {
        int line;
        if (!sscanf(command, "ps %d", &line) == 1)
        {
            printf(RED "Invalid ps command!\n" RESET);
            return 1;
        }
        if (strlen(paste))
        {
            strcpy(lines[line - 1], paste);
        }
        else
        {
            printf(RED "Paste buffer is empty!\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "psi ", 3) == 0)
    {
        int line;
        if (!sscanf(command, "psi %d", &line) == 1)
        {
            printf(RED "Invalid psi command!\n" RESET);
            return 1;
        }
        if (strlen(paste))
        {
            insert_line(line, paste);
        }
        else
        {
            printf(RED "Paste buffer is empty!\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "cp ", 3) == 0)
    {
        int line;
        if (sscanf(command, "cp %d", &line) == 1)
        {
            strcpy(paste, lines[line - 1]);
        }
        else
        {
            printf(RED "Invalid cp command!\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "cp", 2) == 0 && strlen(command) == 2)
    {
        if (!cline > 0)
        {
            printf(RED "The buffer is empty!\n" RESET);
            return 1;
        }
        strcpy(paste, lines[cline - 1]);
    }
    else if (strncmp(command, "cps", 3) == 0 && strlen(command) == 3)
    {
        strcpy(paste, "");
    }
    else if (strncmp(command, "sps ", 4) == 0)
    {
        strcpy(paste, command + 4);
    }
    else if (strncmp(command, "ps-buffer", 9) == 0 && strlen(command) == 9)
    {
        if (strlen(paste))
        {
            printf("Paste buffer: %s\n", paste);
        }
        else
        {
            printf("Paste buffer is empty!\n");
        }
    }
    else if (strncmp(command, "name", 4) == 0 && strlen(command) == 4)
    {
        if (hasname)
        {
            printf("Current file: %s\n", file_name);
        }
        else
        {
            printf(RED "No file set!\n" RESET);
            return 1;
        }
    }
    else if (strncmp(command, "rsn", 3) == 0 && strlen(command) == 3)
    {
        hasname = 0;
    }
    else if (strncmp(command, "run ", 4) == 0)
    {
        if (run_file(command + 4))
            printf(RED "Error running file: %s\n" RESET, command + 4);
    }
    /* OLD ED COMMANDS */
    else if (strncmp(command, "a", 1) == 0 && strlen(command) == 1)
    {
        into_a();
    }
    else
    {
        if (strlen(command))
        {
            printf(RED "Error Unknown command\n" RESET);
            return 1;
        }
    }
    has_changes = 1;
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        load_file(argv[1]);
        strcpy(file_name, argv[1]);
        hasname = 1;
    };
    char command[MAX_LINE_LENGTH + 5];
    printf("EdIm v0.1 (type 'q' to quit, type 'h' for help)\n");
    while (1)
    {
        strcpy(command, "");
        printf("> ");
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            break;
        }
        command[strcspn(command, "\n")] = '\0'; // Remove trailing newline
        if (process_command(command) == 2)
            break;
    }
    return 0;
}
