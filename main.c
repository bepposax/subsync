#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAXLEN 256
#define RED    "\e[0;91m"
#define GREEN  "\e[0;92m"
#define RESET  "\e[0m"

typedef struct {
    int h, m, s, ms;
} timestamp_t;

typedef struct {
    int id;
    timestamp_t* start, * end;
} sub_t;

void addhours(timestamp_t* t, int offset);
void addminutes(timestamp_t* t, int offset);
void addseconds(timestamp_t* t, int offset);
void addmilliseconds(timestamp_t* t, int offset);
static inline void err_exit(const char* msg, int exit_value) {
    fprintf(stderr, "%s\nPress enter to exit\n", msg);
    getchar();
    getchar();
    exit(exit_value);
};

int main(int argc, char* argv[]) {
    FILE* file, * edit;
    float offset = 0;
    int offset_ms;
    char filename[MAXLEN], editname[MAXLEN], line[MAXLEN];
    timestamp_t starttime = { 0 }, endtime = { 0 };
    sub_t sub = {
        .start = &starttime,
        .end = &endtime
    };


    // get filename and offset
    if (argc < 3) {
        int id = 0, choice = 0;
        DIR* dir;
        struct dirent* f;
        char file[MAXLEN][MAXLEN];

        if (!(dir = opendir("."))) err_exit("Unable to open current directory", 1);
        printf("%-3s\tSubtitle Files\n", "ID");
        puts("--\t--------------");

        while ((f = readdir(dir))) {
            if (strstr(f->d_name, ".srt")) {
                printf("%d\t%s\n", id, f->d_name);
                strcpy(file[id++], f->d_name);
            }
        }
        puts("");
        if (id == 0) err_exit("No subtitle files found", 1);
        if (id > 1) {
            printf(">\tFile ID   ");
            while (!scanf("%d", &choice));
            if (choice < 0 || choice >= id)
                err_exit("Invalid choice", 1);
            printf("<\t%s\n\n", file[choice]);
        }
        strcpy(filename, file[choice]);
        printf(">\tOffset (seconds)   ");
        while (!scanf("%f", &offset));

        closedir(dir);
    }
    else {
        strcpy(filename, argv[1]);
        offset = atof(argv[2]);
    }
    if (offset == 0)
        err_exit("No change", 0);
    // file supported check
    if (strstr(filename, ".srt") == NULL)
        err_exit("File not supported\nSupported formats: .srt", 1);
    if (!(file = fopen(filename, "r")))
        err_exit(strcat("Can't open file ", filename), 1);
    // create edited file name
    snprintf(editname, MAXLEN, "%.*s-%.03f.srt", (int)strlen(filename) - 4, filename, offset);
    if (!(edit = fopen(editname, "w")))
        err_exit(strcat("Can't open file ", editname), 1);

    offset_ms = offset * 1000;
    setvbuf(stdout, NULL, _IOFBF, 1 << 10);
    // read input
    while (fgets(line, MAXLEN, file)) {
        if (sscanf(line, "%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d",
            &sub.start->h, &sub.start->m, &sub.start->s, &sub.start->ms,
            &sub.end->h, &sub.end->m, &sub.end->s, &sub.end->ms) != 8
            ) {
            fprintf(edit, "%s", line);
        }

        else {
            sub.id++;
            addmilliseconds(sub.start, offset_ms);
            addmilliseconds(sub.end, offset_ms);
            // write modified times in output file
            fprintf(edit, "%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\n",
                sub.start->h, sub.start->m, sub.start->s, sub.start->ms,
                sub.end->h, sub.end->m, sub.end->s, sub.end->ms);

            // log results
            line[strcspn(line, "\r\n")] = 0;
            printf("%03d\t" RED "%s" RESET " .... " GREEN "%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d" RESET "\n",
                sub.id, line, sub.start->h, sub.start->m, sub.start->s, sub.start->ms,
                sub.end->h, sub.end->m, sub.end->s, sub.end->ms
            );
        }
    }
#ifdef __linux__
#define SLASH "/"
#else
#define SLASH "\\"
#endif
    printf("\nSaved in: ." SLASH "%s\n\nPress enter to exit \n", editname);
    fflush(stdout);
    fclose(file);
    fclose(edit);
    getchar();
    getchar();

    return 0;
}

void addhours(timestamp_t* t, int offset) {
    t->h += offset;
}

void addminutes(timestamp_t* t, int offset) {
    t->m += offset;
    if (t->m >= 60) {
        addhours(t, t->m / 60);
        t->m %= 60;
    }
    while (t->m < 0) {
        addhours(t, -1);
        t->m += 60;
    }
}

void addseconds(timestamp_t* t, int offset) {
    t->s += offset;
    if (t->s >= 60) {
        addminutes(t, t->s / 60);
        t->s %= 60;
    }
    while (t->s < 0) {
        addminutes(t, -1);
        t->s += 60;
    }
}

void addmilliseconds(timestamp_t* t, int offset) {
    if (offset < 0 && (t->s * 1000 + t->ms) < offset * -1) return;
    t->ms += offset;
    if (t->ms >= 1000) {
        addseconds(t, t->ms / 1000);
        t->ms %= 1000;
    }
    while (t->ms < 0) {
        addseconds(t, -1);
        t->ms += 1000;
    }
}
