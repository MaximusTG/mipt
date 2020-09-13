const int DEBUG_MODE = 1;

int           DEBUG_NUMBER = 1;
unsigned char DEBUG_LETTER = 'a';

#define DEBUG_NUMBER_PRINT() printf("[deb] %d [deb]\n", DEBUG_NUMBER++);
#define DEBUG_LETTER_PRINT() printf("[deb] %c [deb]\n", DEBUG_LETTER++);

#define DEBUG_NUMBER_INCR() DEBUG_NUMBER++;
#define DEBUG_LETTER_INCR() DEBUG_LETTER++;

#define DEBUG if(DEBUG_MODE)


enum CURRENT_PROJECT_SETTINGS { // special for Onegin
    STROFA_SIZE = 14,
    RYTHM_DEPTH = 4
};

enum ERRORS {
    ERROR_FILE_NOT_FOUND = -10,
    ERROR_BIG_FILE,
    ERROR_MALLOC_FAIL
};

struct Line {
    unsigned char *string;
    size_t len;
    unsigned char ending[RYTHM_DEPTH + 1]; // special for Onegin
};

typedef struct Line Line_t;

struct File {
    char *name;
    int file_ptr;
    struct stat info;
    unsigned char *text;
    int lines_cnt;
    Line_t **lines;
};

typedef struct File File_t;

void qqh_sort(const void *arr, const size_t elem_cnt, const size_t elem_size, int (*comp)(void *elem1, void *elem2));

int compare_lines(const void **elem1, const void **elem2);
int reverse_compare_lines(const void **elem1, const void **elem2);
void free_memory_file(const File_t *file);
int read_file(File_t *file, const char *name);
int read_lines(File_t *file);
void print_file(const File_t *file, const char *fout);

char is_russian_letter(const unsigned char c) {
    return (c >= (unsigned char) '�' && c <= (unsigned char) '�') || (c >= (unsigned char) '�' && c <= (unsigned char) '�');
}

char is_letter(const unsigned char c) {
    return isalpha(c) || is_russian_letter(c);
}

void swap_ptrs(void **first, void **second) {
    void *tmp = *second;
    *second = *first;
    *first = tmp;
}

void qqh_sort(const void *arr, const size_t elem_cnt, const size_t elem_size, int (*comp)(void *elem1, void *elem2)) {
    assert(arr);

    for (int i = 0; i < elem_cnt; ++i) {
        for (int j = 0; j < elem_cnt - 1; ++j) {
            void *first = arr + j * elem_size;
            void *second = arr + (j + 1) * elem_size;
            if (comp(first, second) > 0) {
                swap_ptrs(first, second);
            }
        }
    }
}

int compare_lines_letters(const void **elem1, const void **elem2) {
    const unsigned char* str1 = ((Line_t*)(*elem1))->string;
    const unsigned char* str2 = ((Line_t*)(*elem2))->string;

    int i = 0;
    int j = 0;
    while (str1[i] && str2[j]) {
        while (!is_letter(str1[i]) && str1[i]) {
            ++i;
        }
        while (!is_letter(str2[j]) && str2[j]) {
            ++j;
        }

        if (str1[i] != str2[i] || str1[i] * str2[i] == 0) {
            return str1[i] - str2[j];
        }

        ++i;
        ++j;
    }
    return str1[i] - str2[j];
}

int reverse_compare_lines_letters(const void **elem1, const void **elem2) {
    return -compare_lines_letters(elem1, elem2);
}

void free_memory_file(const File_t *file) {
    assert(file);

    Line_t **lines_ptr = file->lines;
    for (int i = 0; i < file->lines_cnt; ++i) {
        free(*lines_ptr);
        ++lines_ptr;
    }
    free(file->lines);
    free(file->text);
}

int read_file(File_t *file, const char *name) {
    assert(file);
    assert(name);

    file->name = name;
    stat(name, &(file->info));

    file->text = calloc(file->info.st_size + 1, sizeof(char));
    if (!file->text) {
        return ERROR_MALLOC_FAIL;
    }
    return file->lines_cnt = read_lines(file);
}

int read_lines(File_t *file) {
    assert(file);

    DEBUG {printf("Working with [%s] file\n", file->name)};

    file->file_ptr = open(file->name, O_BINARY);
    if (!file->file_ptr) {
        return ERROR_FILE_NOT_FOUND;
    }

    read(file->file_ptr, file->text, file->info.st_size);
    int lines_cnt = 0;
    for (unsigned char *c = file->text; *c; ++c) {
        lines_cnt += *c == '\n';
    }
    file->lines = calloc(lines_cnt, sizeof(Line_t*));
    if (!file->lines) {
        return ERROR_MALLOC_FAIL;
    }

    unsigned char *c = file->text;
    lines_cnt = -1;
    int line_len = 0;
    while (*c) {
        ++lines_cnt;

        file->lines[lines_cnt] = calloc(1, sizeof(Line_t));
        if (!file->lines[lines_cnt]) {
            return ERROR_MALLOC_FAIL;
        }
        Line_t *line_ptr = file->lines[lines_cnt];
        line_ptr->string = c;

        while(*c != '\n') {
            ++line_len;
            ++c;
            if (*c == '\r') {
                *c = '\0';
            }
        }
        *c = '\0';
        ++c;

        line_ptr->len = line_len;
        line_len = 0;
    }
    file->lines_cnt = lines_cnt;

    return lines_cnt + 1;
}

void print_file(const File_t *file, const char *fout_name) {
    assert(file);
    assert(fout_name);

    FILE *fout = fopen(fout_name, "w");
    for (int i = 0; i < file->lines_cnt; ++i) {
        fprintf(fout, "%s\n", file->lines[i]->string);
    }

    fclose(fout);
}
