/*
 *  sfile
 *  src/sfile.c
 *
 *  Author: Vilmain Nicolas
 *  Contact: nicolas.vilmain@gmail.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include  <pwd.h>
#include  <errno.h>
#include  <stdio.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <string.h>
#include  <strings.h>
#include  <dirent.h>
#include  <unistd.h>
#include  <sys/stat.h>
#include  "sfile.h"

const char *program_name;

int
main(int argc, char **argv)
{
    struct opt_s x;

    set_program_name(argv[0]);
    sfile_init(&x);
    decode_program_param(argc, argv, &x);
    scan_arg_object(argc, argv, &x);
    sfile_free(&x);
    return EXIT_SUCCESS;
}

void
sfile_init(struct opt_s *x)
{
    memset(x, 0, sizeof(struct opt_s));
    x->byino = -1;
    x->byuid = -1;
    x->n_exit = -1;
}

void
sfile_free(struct opt_s *x)
{
    xfree(x->ign);
    xfree(x->ext);
    xfree(x->wif);
    xfree(x->win);
    xfree(x->wnf);
    free_str_array(x->ign_ext);
}

void
set_program_name(const char *arg0)
{
    char *sep = NULL;

    program_name = arg0;
    sep = strrchr(arg0, '/');
    if (sep && *(sep + 1))
        program_name = (sep + 1);
}

void
decode_program_param(int argc, char **argv, struct opt_s *x)
{
    int current_arg;

    if (argc == 1) {
        x->opts = O_LS_MODE;
        return;
    }
    do {
        current_arg = getopt_long(argc, argv, STR_OPT_INDEX, opt_index, NULL);
        switch (current_arg) {
        case 'h':
            usage();
            break;
        case 'v':
            version();
            break;
        case 'a':
            x->opts |= O_ALL;
            break;
        case 'w':
            x->opts |= (O_ENV_PATH | O_FULL_PATH);
            break;
        case 'r':
            x->opts |= O_RECURSIVE;
            break;
        case 'D':
            x->opts |= O_IGN_DIR;
            break;
        case 'F':
            x->opts |= O_IGN_FILE;
            break;
        case 'B':
            x->opts |= O_IGN_BACKUP;
            break;
        case 'A':
            x->opts |= O_IGN_ARCHIVE;
            break;
        case 'P':
            x->opts |= O_FULL_PATH;
            break;
        case 'c':
            x->opts |= O_COLOR;
            break;
        case 'I':
            x->opts |= O_PUT_INODE;
            break;
        case 'l':
            x->opts |= O_NUM_LINE;
            break;
        case 'L':
            x->opts |= O_FILE_INFOS;
            break;
        case 'p':
            x->opts |= O_PRINT;
            break;
        case 'V':
            x->opts |= O_ALL_PRINT;
            break;
        case 'x':
            x->n_exit = xstrtol_fatal(optarg, "invalid argument -x, --exit");
            break;
        case 'Q':
            x->byino = xstrtol_fatal(optarg, "invalid argument -Q, --inode");
            break;
        case 'u':
            x->byuid = xstrtol_fatal(optarg, "invalid argument -u, --uid");
            break;
        case 'o':
            xfree(x->ign);
            x->ign = xstrdup(optarg);
            break;
        case 'e':
            xfree(x->ext);
            x->ext = xstrdup(optarg);
            break;
        case 'i':
            xfree(x->wif);
            x->wif = xstrdup(optarg);
            break;
        case 'N':
            xfree(x->wnf);
            x->wnf = xstrdup(optarg);
            break;
        case 'n':
            xfree(x->win);
            x->win = xstrdup(optarg);
            break;
        case OPT_IGN_CASE_FILE_NAME:
            x->opts |= O_IGN_CASE_FILE_NAME;
            break;
        case OPT_IGN_CASE_IN_FILE:
            x->opts |= O_IGN_CASE_IN_FILE;
            break;
        case OPT_WIF_COUNT:
            x->opts |= O_WIF_COUNT;
            break;
        case 'C':
            x->opts |= (O_IGN_CASE_FILE_NAME | O_IGN_CASE_IN_FILE);
            break;
        case 'G':
            free_str_array(x->ign_ext);
            x->ign_ext = parse_str_array(optarg);
            break;
        case OPT_ACK_LIKE:
            x->opts |= O_ALL_PRINT | O_PRINT | O_FULL_PATH | O_RECURSIVE |
                       O_NUM_LINE | O_COLOR;
            x->wif = xstrdup(optarg);
            break;
        default:
            /* for waring */
            break;
        }
    } while (current_arg != -1);
    if (!x->wif && !x->win && !x->wnf && !x->ext &&
        x->byuid == -1 && x->byino == -1) {
        x->opts |= O_LS_MODE;
    }

    /* Set pointer on string search/cmp functions by options. */
    x->cmpstring_wnf = strcmp;
    x->searchstring_win = strstr;
    if ((x->opts & O_IGN_CASE_FILE_NAME)) {
        x->cmpstring_wnf = strcasecmp;
        x->searchstring_win = xstrcasestr;
    }

    x->searchstring_wif = strstr;
    if ((x->opts & O_IGN_CASE_IN_FILE)) {
        x->searchstring_wif = xstrcasestr;
    }
}

void
scan_arg_object(int argc, char **argv, struct opt_s *x)
{
    struct finfo_s finfo;

    if ((x->opts & O_ENV_PATH))
        scan_path_environ(x);
    if (x->n_exit) {
        do {
            memset(&finfo, 0, sizeof(struct finfo_s));
            if ((argc - optind))
                strncpy(finfo.fi_path, argv[optind++], PATH_LEN_USE);
            set_object_path(finfo.fi_path, (x->opts & O_FULL_PATH));
            finfo.fi_type = get_file_type(&finfo);
            if (finfo.fi_type != TF_ERROR) {
                if (finfo.fi_type == TF_DIR)
                    list_dir_object(x, finfo.fi_path);
                else {
                    finfo.fi_name = strrchr(finfo.fi_path, '/');
                    if (!finfo.fi_name)
                        finfo.fi_name = finfo.fi_path;
                    else
                        finfo.fi_name++;
                    check_object(x, &finfo);
                }
            }
        } while ((optind - argc) && x->n_exit);
    }
}

void
scan_path_environ(struct opt_s *x)
{
    char *envpath = NULL;
    char *buf = NULL;

    envpath = getenv("PATH");
    if (!envpath || !envpath[0]) {
#ifndef NDEBUG
        fprintf(stderr, "Environement variable `%s' not set or is empty.",
                ENV_VAR_PATH);
#endif /* NDEBUG */
        return;
    }

    buf = strtok(envpath, ":");
    while (buf && x->n_exit) {
        list_dir_object(x, buf);
        buf = strtok(NULL, ":");
    }
}

void
set_object_path(char *name, uint32_t full)
{
#define PATH_LEN_TRUNCATED (PATH_LEN - 2) /* ./ gcc warning */
    char buf[PATH_LEN_TRUNCATED];

    /* if name begin by '/', it is already full path */
    if (name[0] == '/')
        return;
    strncpy(buf, name, (PATH_LEN_TRUNCATED - 1));
    /* set full path */
    if (full) {
        if (get_current_dir(name))
            strncpy(name, buf, PATH_LEN);
        strncat(name, buf, (PATH_LEN - strlen(name)));
    }
    else {
        /* need add ./ */
        if (name[0] != '.' && name[1] != '/')
            snprintf(name, PATH_LEN, "./%s", buf);
    }
}

int
get_current_dir(char *current_path)
{
    char *ret = NULL;
    size_t len;

    ret = getcwd(current_path, PATH_LEN_USE);
    if (!ret) {
        fprintf(stderr, "%s:getcwd: get current path fails\n",
                program_name);
        return -1;
    }
    len = strlen(current_path);
    if (current_path[len] != '/')
        current_path[len] = '/';
    return 0;
}

enum file_type_e
get_file_type(struct finfo_s *fi)
{
    if (lstat(fi->fi_path, &fi->fi_stat) == -1) {
        fprintf(stderr, "%s:lstat:path `%s': %s\n", program_name,
                fi->fi_path, strerror(errno));
        return TF_ERROR;
    }
    if (S_ISDIR(fi->fi_stat.st_mode))
        return TF_DIR;
    else if (*(fi->fi_path
               + (strlen(fi->fi_path) - 1)) == '~')
        return TF_BACKUP;
    else if (!object_is_archive(fi->fi_path))
        return TF_ARCHIVE;
    else if (S_ISREG(fi->fi_stat.st_mode))
        return TF_REG;
    return TF_OTHER;
}

int
object_is_archive(const char *name)
{
    const char **p_tab_archive = NULL;
    char *ext = NULL;

    ext = strrchr(name, '.');
    if (ext) {
        p_tab_archive = tab_archive;
        do {
            if (!strcmp(*p_tab_archive, ext))
                return 0;
        } while (*++p_tab_archive);
    }
    return -1;
}

void
list_dir_object(struct opt_s *x, const char *path)
{
    DIR *dir = NULL;
    size_t len;
    struct dirent *ent = NULL;
    struct finfo_s fi;
    struct stack_s dlist;
    struct stack_chunk_s *p_next = NULL;

    dlist.chunk = NULL;
    dlist.tail = NULL;
    push_dir_stack(&dlist, path);
    do {
        dir = opendir(dlist.chunk->un.str);
        if (!dir) {
            fprintf(stderr, "%s:opendir: path: `%s': %s\n", program_name,
                    dlist.chunk->un.str, strerror(errno));
        }
        else {
            x->p_current_path = dlist.chunk->un.str;
            do {
                ent = readdir(dir);
                if (!ent)
                    break;
                if ((ent->d_name[0] != '.' ||
                     (ent->d_name[0] == '.' && (x->opts & O_ALL) &&
                      strcmp(ent->d_name, ".") &&
                      strcmp(ent->d_name, ".."))) &&
                    (!x->ign || (x->ign && !strstr(ent->d_name, x->ign)))) {
                    memset(&fi, 0, sizeof(struct finfo_s));
                    strncpy(fi.fi_path, dlist.chunk->un.str, PATH_LEN_USE);
                    len = strlen(dlist.chunk->un.str);
                    if (*(dlist.chunk->un.str + (len - 1)) != '/') {
                        fi.fi_path[len] = '/';
                    }
                    strncat(fi.fi_path, ent->d_name, (PATH_LEN_USE - len));
                    fi.fi_name = ent->d_name;
                    check_object(x, &fi);
                    if (fi.fi_type == TF_DIR && (x->opts & O_RECURSIVE))
                        push_dir_stack(&dlist, fi.fi_path);
                }
            } while (x->n_exit);
            closedir(dir);
        }
        if (dlist.chunk) {
            p_next = dlist.chunk->next;
            xfree(dlist.chunk->un.str);
            xfree(dlist.chunk);
            dlist.chunk = p_next;
        }
    } while (x->n_exit && dlist.chunk);
}

void
check_object(struct opt_s *x, struct finfo_s *fi)
{
    fi->fi_type = get_file_type(fi);
    if (fi->fi_type == TF_ERROR)
        return;

    /* check filter */
    if ( /* check ignore file type */
         (fi->fi_type == TF_BACKUP && (x->opts & O_IGN_BACKUP)) ||
         (fi->fi_type == TF_DIR && (x->opts & O_IGN_DIR)) ||
         (fi->fi_type == TF_REG && (x->opts & O_IGN_FILE)) ||
         (fi->fi_type == TF_ARCHIVE && (x->opts & O_IGN_ARCHIVE)) ||
         /* check ignore and ignore by extension */
         (x->ign_ext && !ign_file_extension(fi->fi_name, x->ign_ext)))
        return;

    x->n_wif_result = 0;
    if (/* ls mode, list all file by default */
         (x->opts & O_LS_MODE) ||
         /* search by uid */
         (x->byuid == (int) fi->fi_stat.st_uid) ||
         /* is file inode ? */
         (x->byino == (int) fi->fi_stat.st_ino) ||
         /* search by file extension */
         (x->ext && !cmp_file_extension(fi->fi_name, x->ext)) ||
         /* search word in file name */
         (x->win && x->searchstring_win(fi->fi_name, x->win)) ||
         /* compar file name */
         (x->wnf && !x->cmpstring_wnf(x->wnf, fi->fi_name)) ||
         /* search word in file */
         (x->wif && !word_in_file(x, fi->fi_path))) {
        sfile_print_object(x, fi);
        x->n_exit--;
    }
}

int
ign_file_extension(const char *name, char **ext)
{
    char *buf = NULL;
    char **p_ext = NULL;

    buf = strrchr(name, '.');
    if (!buf)
        return -1;

    p_ext = ext;
    while (*p_ext) {
        if (!strcmp(buf, *p_ext))
            return 0;
        p_ext++;
    }
    return -1;
}

int
cmp_file_extension(const char *name, const char *ext)
{
    char *buf = NULL;

    buf = strrchr(name, '.');
    return (buf && !strcmp(buf, ext)) ? 0 : -1;
}

int
word_in_file(struct opt_s *x, const char *path_file)
{
    long n_lines;
    FILE *file = NULL;
    char buf[LINE_BUFSIZE];

    file = fopen(path_file, "r");
    if (!file) {
        fprintf(stderr, "%s:fopen `%s': %s\n",
                program_name, path_file, strerror(errno));
        return -1;
    }

    /* first line */
    n_lines = 1;
    do {
        if (fgets(buf, LINE_BUFSIZE, file)) {
            if (x->searchstring_wif(buf, x->wif)) {
                x->n_wif_result++;
                if ((x->opts & O_PRINT) ||
                    (x->opts & O_ALL_PRINT) ||
                    (x->opts & O_NUM_LINE)) {
                    push_line_stack(&x->line, (x->opts &
                                               (O_PRINT | O_ALL_PRINT)),
                                    buf, n_lines);
                }
                if (!(x->opts & O_ALL_PRINT) && !(x->opts & O_WIF_COUNT)) {
                    fclose(file);
                    return 0;
                }
            }
            if (strchr(buf, '\n'))
                n_lines++;
        }
    } while (!feof(file) && !ferror(file));
    fclose(file);

    if (((x->opts & O_WIF_COUNT) && x->n_wif_result > 0) || x->line.tail)
        return 0;
    return -1;
}


void
push_dir_stack(struct stack_s *stack, const char *path)
{
    struct stack_chunk_s *new = NULL;

    new = xmalloc(sizeof(struct stack_chunk_s));
    if (stack->chunk) {
        new->next = stack->chunk->next;
        stack->chunk->next = new;
    }
    else {
        stack->chunk = new;
        new->next = NULL;
    }
    new->un.str = xstrdup(path);
}

void
push_line_stack(struct stack_s *stack, uint32_t print, char *line, long n)
{
    struct stack_chunk_s *new = NULL;

    new = xmalloc(sizeof(struct stack_chunk_s));
    APPENDTOSTACK(stack, new);
    new->un.data = xmalloc(sizeof(struct line_s));
    LINE_S (new) = NULL;
    if (print) {
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        LINE_S(new) = xstrdup(line);
    }
    LINE_N(new) = n;
}

void
sfile_print_object(struct opt_s *x, struct finfo_s *fi)
{
    if (NEED_CUSTOM_OUTPUT(x) && x->line.chunk)
        printf("\x1b[1;36;44m\x1B[37m"); /* set custom color */

    if ((x->opts & O_FILE_INFOS)) {
        print_perm_object(fi->fi_stat.st_mode);
        print_user_object(fi->fi_stat.st_uid);
#ifdef MACOS
        printf("\r\t\t\t %llu\r\t\t\t\t\t", fi->fi_stat.st_size);
#else
        printf("%ld ", fi->fi_stat.st_size);
#endif /* MACOS */
    }

    if ((x->opts & O_PUT_INODE)) {
#ifdef MACOS
        printf("(ino: %llu) ", fi->fi_stat.st_ino);
#else
        printf("(ino: %lu) ", fi->fi_stat.st_ino);
#endif /* MACOS */
    }

    if ((x->opts & O_WIF_COUNT) && x->n_wif_result) {
        printf("(n_result: %lu) ", x->n_wif_result);
    }

    print_object_name(fi, x);

    if (x->line.tail) {
        print_line_object(x->line.chunk, x);
        x->line.chunk = NULL;
        x->line.tail = NULL;
    }
    else
        putchar('\n');
}

void
print_perm_object(mode_t mode)
{
    unsigned char perm[11];

    perm[10] = 0;
    perm[0] = (S_IFDIR & mode) ? 'd' : '-';
    perm[1] = (S_IRUSR & mode) ? 'r' : '-';
    perm[2] = (S_IWUSR & mode) ? 'w' : '-';
    perm[3] = object_have_suid_bit(mode, S_ISUID, S_IXUSR);
    perm[4] = (S_IRGRP & mode) ? 'r' : '-';
    perm[5] = (S_IWGRP & mode) ? 'w' : '-';
    perm[6] = object_have_suid_bit(mode, S_ISGID, S_IXGRP);
    perm[7] = (S_IROTH & mode) ? 'r' : '-';
    perm[8] = (S_IWOTH & mode) ? 'w' : '-';
    perm[9] = (S_IXOTH & mode) ? 'x' : '-';
    printf("%s  ", perm);
}

unsigned char
object_have_suid_bit(mode_t mode, unsigned int flag, unsigned int flag_x)
{
    if ((flag & mode))
        return 's';
    return (flag_x & mode) ? 'x' : '-';
}

void
print_user_object(uid_t uid)
{
    struct passwd *pwd = NULL;

    pwd = getpwuid(uid);
    if (!pwd)
        return;
    printf("%s ", pwd->pw_name);
}

void
print_object_name(struct finfo_s *fi, struct opt_s *x)
{
    size_t len;
    const char *color = EMPTY_STRING;
    char buf[PATH_LEN];
    char buf_path[PATH_LEN];

    if ((x->opts & O_FULL_PATH)) {
        /* bug: sfile -cPi string no color output ...
         * but for --ack or -cVi options color is ok
         * need call COLOR_NULL at end.
         */
        printf("%s%s", fi->fi_path, COLOR_NULL);
        return;
    }

    buf_path[0] = 0;
    if (x->p_current_path && x->p_current_path[0]) {
        len = strlen(x->p_current_path);
        if (*(x->p_current_path + (len - 1)) != '/')
            snprintf(buf_path, PATH_LEN_USE, "%s/", x->p_current_path);
        else
            strncpy(buf_path, x->p_current_path, PATH_LEN);
    }

    memset(buf, 0, PATH_LEN);
    if ((x->opts & O_COLOR)) {
        if (fi->fi_type == TF_DIR)
            color = COLOR_DIR;
        else if (fi->fi_type == TF_REG) {
            if (!NEED_CUSTOM_OUTPUT(x) || !x->line.chunk)
                color = COLOR_REG_FILE;
        }
        else if (fi->fi_type == TF_BACKUP)
            color = COLOR_BACKUP;
        else if (fi->fi_type == TF_ARCHIVE)
            color = COLOR_ARCHIVE;

        snprintf(buf, PATH_LEN_USE, "%s%s%s%s",
                 color, buf_path, fi->fi_name, COLOR_NULL);
    }
    else {
        if ((x->opts & O_FULL_PATH))
            strncpy(buf, fi->fi_path, PATH_LEN);
        else {
            snprintf(buf, PATH_LEN_USE, "%s%s",
                     buf_path, fi->fi_name);
        }
    }
    printf("%s ", buf);
}

void
print_line_object(struct stack_chunk_s *chunk, struct opt_s *x)
{
    struct stack_chunk_s *p_next = NULL;

    if (LINE_S(chunk)) {
        putchar('\n');
        do {
            p_next = chunk->next;
            if (!(x->opts & O_NUM_LINE)) {
                printf(" + %s\n", LINE_S(chunk));
            }
            else {
                if (!NEED_CUSTOM_OUTPUT(x))
                    printf(" [%ld] + %s\n", LINE_N(chunk), LINE_S(chunk));
                else {
                    printf(" [\x1b[1;36;44m\x1B[37m%ld%s] + %s\n",
                           LINE_N(chunk), COLOR_NULL, LINE_S(chunk));
                }
            }
            xfree(LINE_S(chunk));
            xfree(chunk->un.data);
            xfree(chunk);
            chunk = p_next;
        } while (chunk);
    }
    else {
        printf(" (line: %ld)\n", LINE_N(chunk));
        xfree(chunk->un.data);
        xfree(chunk);
    }
}

void *
xmalloc(size_t size)
{
    void *ptr = NULL;

    if (!size)
        size++;
    ptr = malloc(size);
    if (!ptr)
        out_memory("malloc");
    return ptr;
}

char *
xstrdup(const char *str)
{
    char *copy = NULL;

    copy = strdup(str);
    if (!copy)
        out_memory("strdup");
    return copy;
}

void
xfree(void *ptr)
{
    if (ptr)
        free(ptr);
}

void
free_str_array(char **array)
{
    int i;

    if (array) {
        for (i = 0; array[i]; i++)
            xfree(array[i]);
        xfree(array);
    }
}

void
out_memory(const char *func_name)
{
    fprintf(stderr, "%s:%s: memory exhausted\n",
            program_name, func_name);
    exit(EXIT_FAILURE);
}

int
xstrtol_fatal(const char *str, const char *err_msg)
{
    int ret;
    char *err = NULL;

    ret = (int) strtol(str, &err, 10);
    if (*err != '\0') {
        fprintf(stderr, "%s:strtol: %s\n", program_name, err_msg);
        exit(EXIT_FAILURE);
    }
    return ret;
}

char *
xstrcasestr(const char *str, const char *substr)
{
    size_t len;

    if (!str || !str[0] || !substr || !substr[0])
        return NULL;
    len = strlen(substr);

    do {
        if (tolower(*str) == tolower(*substr) &&
            !strncasecmp(str, substr, len)) {
/* disable warning -Wcast-qual */
#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif
            return (char *) str;
#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
        }
        str++;
    } while (*str);
    return NULL;
}

char **
parse_str_array(const char *arg)
{
    int i;
    char *copy = NULL;
    char *ptr = NULL;
    char **array = NULL;

    copy = xstrdup(arg);
    i = 2; /* string + final NULL */
    do {
        if (*arg++ == ',')
            i++;
    } while (*arg);

    array = xmalloc(((size_t) i) * sizeof(char *));
    i = 0;
    ptr = copy;
    arg = strtok(ptr, ",");
    while (arg) {
        array[i++] = xstrdup(arg);
        arg = strtok(NULL, ",");
    }
    xfree(copy);
    array[i] = NULL;
    return array;
}

void
usage(void)
{
    printf("%s usage: %s [OPTION_1 OPTION_2 ...]  file_1 file_2 ...\n"
           "  -h, --help                      show usage and exit program\n"
           "  -v, --version                   show program version and exit\n"
           "  -a, --all                       do not ignore entries starting with '.'\n"
           "  -w, --which                     search in directory to the $PATH varaible\n"
           "  -r, --recursive                 list subdirectories recursively\n"
           "  -D, --ign-dir                   do not list entries is dir\n"
           "  -F, --ign-file                  do not list entries is regular file\n"
           "  -B, --ign-backup                do not list implied entries ending by '~'\n"
           "  -A, --ign-archive               do not list implied entries ending by\n"
           "                                  .gz or .bz2 or .zip or .rar or .7z\n"
           "  -G, --ign-ext [.EXT,.EXT2,...]  ignore all file ending by EXT\n"
           "  -P, --full-path                 show full path to the entries\n"
           "  -c, --color                     display file name with color\n"
           "  -L, --info                      print entries informations\n"
           "  -I, --put-inode                 print inode for file finds\n"
           "  -l, --line                      print line to find word in file\n"
           "                                  (just with -i argument)\n"
           "  -p, --print                     print first line to find word\n"
           "  -V, --print-all                 print all line to found word\n"
           "  -C, --ign-case                  ignore case distinctions in file name and word\n"
           "      --ign-case-in-file          ignore case distinctions to search word in file\n"
           "      --ign-case-file-name        ignore case distinctions in file name\n"
           "      --count                     count result for option --in-file\n"
           "  -x, --exit [N]                  exit program after N result finds\n"
           "  -o, --no-scan [STR]             do not list entries with STR in name\n"
           "  -e, --extension [STR]           search file by extension\n"
           "  -i, --in-file [STR]             search string to file\n"
           "  -N, --name [STR]                search file to name exactly with STR\n"
           "  -n, --in-name [STR]             if STR in the file name\n"
           "  -u, --uid [UID]                 search file by UID\n"
           "  -Q, --inode [INODE]             search file by inode numbers\n"
           "      --ack [STR]                 like default ack program. (active options: -VlPrci)\n"
           "                                    -V: Print all line where STRING (see option -i)\n"
           "                                    -i: search word in file\n"
           "                                    -l: Print line number\n"
           "                                    -P: print full path\n"
           "                                    -r: recusive\n"
           "                                    -c: color\n",
           program_name, program_name);
    exit(EXIT_SUCCESS);
}

void
version(void)
{
    puts("sfile version 1.4.0");
    exit(EXIT_SUCCESS);
}
