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
#include  <stdlib.h>
#include  <string.h>
#include  <dirent.h>
#include  <unistd.h>
#include  <sys/stat.h>
#include  "sfile.h"

int
main(int argc, char **argv)
{
    struct opt_s x;

    set_program_name(argv[0]);
    decode_program_param(argc, argv, &x);
    scan_arg_object(argc, argv, &x);
    return EXIT_SUCCESS;
}

void
set_program_name(const char *arg0)
{
    char *sep = NULL;

    sep = strrchr(arg0, '/');
    if (!sep || !*(sep + 1))
        program_name = arg0;
    else
        program_name = (sep + 1);
}

void
decode_program_param(int argc, char **argv, struct opt_s *x)
{
    char current_arg;

    init_sfile_options(x);
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
        case 'E':
            x->opts |= O_ENV_PATH;
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
            x->n_exit = xstrtol_fatal(optarg, "invalid argument for "
                                              "exit after N object find");
            break;
        case 'Q':
            x->byino = xstrtol_fatal(optarg, "invalid argument for "
                                             "search by inode");
            break;
        case 'u':
            x->byuid = xstrtol_fatal(optarg, "invalid argument for "
                                             "search by uid");
            break;
        case 'o':
            x->ign = optarg;
            break;
        case 'e':
            x->ext = optarg;
            break;
        case 'i':
            x->wif = optarg;
            break;
        case 'N':
            x->wnf = optarg;
            break;
        case 'n':
            x->win = optarg;
            break;
        case 'G':
            x->ign_ext = optarg;
            break;
        case OPT_ACK_LIKE:
            x->opts |= O_ALL_PRINT | O_PRINT | O_FULL_PATH | O_RECURSIVE |
                       O_NUM_LINE | O_COLOR;
            x->wif = argv[optind - 1];
            break;
        }
    } while (current_arg != -1);
    if (!x->wif && !x->win && !x->wnf && !x->ext &&
        x->byuid == -1 && x->byino == -1)
        x->opts |= O_LS_MODE;
}

void
init_sfile_options(struct opt_s *x)
{
    memset(x, 0, sizeof(struct opt_s));
    x->byino = -1;
    x->byuid = -1;
    x->n_exit = -1;
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
                strncpy(finfo.fi_path, argv[optind++], _PATH_LEN);
            set_object_path(finfo.fi_path, (x->opts & O_FULL_PATH));
            finfo.fi_type = get_file_type(&finfo);
            if ((int) finfo.fi_type != -1) {
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
    if (envpath) {
        buf = strtok(envpath, ":");
        while (buf && x->n_exit) {
            list_dir_object(x, buf);
            buf = strtok(NULL, ":");
        }
    }
}

void
set_object_path(char *name, setopt_t full)
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

    ret = getcwd(current_path, _PATH_LEN);
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
        fprintf(stderr, "%s:lstat:path %s: %s\n", program_name,
                fi->fi_path, strerror(errno));
        return -1;
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
        x->retline = 0;
        dir = opendir(dlist.chunk->un.str);
        if (!dir) {
            fprintf(stderr, "%s:opendir: path: <%s>: %s\n", program_name,
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
                      strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))) &&
                    (!x->ign || (x->ign && !strstr(ent->d_name, x->ign)))) {
                    memset(&fi, 0, sizeof(struct finfo_s));
                    strncpy(fi.fi_path, dlist.chunk->un.str, (_PATH_LEN - 1));
                    len = strlen(dlist.chunk->un.str);
                    if (*(dlist.chunk->un.str + (len - 1)) != '/')
                        fi.fi_path[len] = '/';
                    strncat(fi.fi_path, ent->d_name, (_PATH_LEN - len));
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
            if (dlist.chunk && x->retline)
                putchar('\n');
        }
    } while (x->n_exit && dlist.chunk);
}

void
check_object(struct opt_s *x, struct finfo_s *fi)
{
    fi->fi_type = get_file_type(fi);
    if ((int) fi->fi_type == -1)
        return;
    /* check filter */
    if ( /* check ignore file type */
         (fi->fi_type == TF_BACKUP && (x->opts & O_IGN_BACKUP)) ||
         (fi->fi_type == TF_DIR && (x->opts & O_IGN_DIR)) ||
         (fi->fi_type == TF_REG && (x->opts & O_IGN_FILE)) ||
         (fi->fi_type == TF_ARCHIVE && (x->opts & O_IGN_ARCHIVE)) ||
         /* check ignore and ignore by extension */
         (x->ign_ext && !cmp_file_extension(fi->fi_name, x->ign_ext)))
        return;

    if (/* ls mode, list all file by default */
         (x->opts & O_LS_MODE) ||
         /* search by uid */
         (x->byuid == (int) fi->fi_stat.st_uid) ||
         /* is file inode ? */
         (x->byino == (int) fi->fi_stat.st_ino) ||
         /* search by file extension */
         (x->ext && !cmp_file_extension(fi->fi_name, x->ext)) ||
         /* search word in file name */
         (x->win && strstr(fi->fi_name, x->win)) ||
         /* compar file name */
         (x->wnf && !strcmp(x->wnf, fi->fi_name)) ||
         /* search word in file */
         (x->wif && !word_in_file(x, fi->fi_path))) {
        x->retline = 1;
        sfile_print_object(x, fi);
        x->n_exit--;
    }
}

int
cmp_file_extension(const char *name, const char *ext)
{
    char *buf = NULL;

    buf = strrchr(name, '.');
    if (buf && !strcmp(buf, ext))
        return 0;
    return -1;
}

int
word_in_file(struct opt_s *x, const char *path_file)
{
    char buf[2048];
    FILE *file = NULL;
    long n_lines;

    file = fopen(path_file, "r");
    if (!file) {
        fprintf(stderr, "%s:fopen <%s>: %s\n",
                program_name, path_file, strerror(errno));
        return -1;
    }
    /* first line */
    n_lines = 1;
    do {
        if (fgets(buf, 4096, file)) {
            if (strstr(buf, x->wif)) {
                if ((x->opts & O_PRINT) ||
                    (x->opts & O_ALL_PRINT) ||
                    (x->opts & O_NUM_LINE))
                    push_line_stack(&x->line, (x->opts &
                                               (O_PRINT | O_ALL_PRINT)),
                                    buf, n_lines);
                if (!(x->opts & O_ALL_PRINT)) {
                    fclose(file);
                    return 0;
                }
            }
        }
        n_lines++;
    } while (!feof(file) && !ferror(file));
    fclose(file);
    return (x->line.tail) ? 0 : -1;
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
push_line_stack(struct stack_s *stack, setopt_t print, char *line, long n)
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
        printf("\x1b[1;36;44m");
    else if ((x->opts & O_COLOR)) {
        //todo print la color ici
    }

    if ((x->opts & O_FILE_INFOS)) {
        print_perm_object(fi->fi_stat.st_mode);
        print_user_object(fi->fi_stat.st_uid);
        /* maybe just libc version ...
         * So add -D MACOS in makefile if you want
         */
#ifdef MACOS
        printf("\r\t\t\t %llu\r\t\t\t\t\t", fi->fi_stat.st_size);
#else
        printf("%lu ", fi->fi_stat.st_size);
#endif /* MACOS */
    }

    if ((x->opts & O_PUT_INODE)) {
#ifdef MACOS
        printf("(ino: %llu) ", fi->fi_stat.st_ino);
#else
        printf("(ino: %lu) ", fi->fi_stat.st_ino);
#endif /* MACOS */
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
    char perm[11];

    perm[10] = 0;
    perm[0] = (S_IFDIR & mode) ? 'd' : '-';
    perm[1] = (S_IRUSR & mode) ? 'r' : '-';
    perm[2] = (S_IWUSR & mode) ? 'w' : '-';
    perm[3] = object_is_suid(mode, S_ISUID, S_IXUSR);
    perm[4] = (S_IRGRP & mode) ? 'r' : '-';
    perm[5] = (S_IWGRP & mode) ? 'w' : '-';
    perm[6] = object_is_suid(mode, S_ISGID, S_IXGRP);
    perm[7] = (S_IROTH & mode) ? 'r' : '-';
    perm[8] = (S_IWOTH & mode) ? 'w' : '-';
    perm[9] = (S_IXOTH & mode) ? 'x' : '-';
    printf("%s  ", perm);
}

int
object_is_suid(mode_t mode, int flag, int flag_x)
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
    const char *search;
    char buf[PATH_LEN + 64];

    memset(buf, 0, (PATH_LEN + 20));

    if ((x->opts & O_COLOR)) {
        if (fi->fi_type == TF_DIR)
            strcpy(buf, COLOR_DIR);
        else if (fi->fi_type == TF_REG) {
            if (!NEED_CUSTOM_OUTPUT(x) || !x->line.chunk)
                strcpy(buf, COLOR_REG_FILE);
        }
        else if (fi->fi_type == TF_BACKUP)
            strcpy(buf, COLOR_BACKUP);
        else if (fi->fi_type == TF_ARCHIVE)
            strcpy(buf, COLOR_ARCHIVE);

        if ((x->opts & O_FULL_PATH)) {
            printf("%s%s", fi->fi_path, COLOR_NULL);
            return;
        }

        search = strrchr(buf, '/');
        if (search == NULL || !search[1])
            strcat(buf, "/");

        strcat(buf, fi->fi_name);
        strcat(buf, COLOR_NULL);
    }
    else {
        if ((x->opts & O_FULL_PATH))
            strncpy(buf, fi->fi_path, _PATH_LEN);
        else
            strncpy(buf, fi->fi_name, _PATH_LEN);
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
            if ((x->opts & O_NUM_LINE)) {
                if (!NEED_CUSTOM_OUTPUT(x))
                    printf(" [%ld] + %s\n", LINE_N(chunk), LINE_S(chunk));
                else {
                    printf(" [\x1b[1;36;44m%ld%s] + %s\n",
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

    ret = strtol(str, &err, 10);
    if (*err != '\0') {
        fprintf(stderr, "%s:strtol: %s\n",
                program_name, err_msg);
        exit(EXIT_FAILURE);
    }
    return ret;
}

void
usage(void)
{
    printf("%s usage: %s [OPTION_1 OPTION_2 ...]  file_1 file_2 ...\n"
           "  -h, --help               show usage and exit program\n"
           "  -v, --version            show program version and exit\n"
           "  -a, --all                do not ignore entries starting with '.'\n"
           "  -E, --env-path           search in directory to the $PATH varaible\n"
           "  -r, --recursive          list subdirectories recursively\n"
           "  -D, --ign-dir            do not list entries is dir\n"
           "  -F, --ign-file           do not list entries is regular file\n"
           "  -B, --ign-backup         do not list implied entries ending by '~'\n"
           "  -A, --ign-archive        do not list implied entries ending by\n"
           "                           .gz or .bz2 or .zip or .rar or .7z\n"
           "  -G, --ign-ext [EXT]      ignore all file ending by EXT\n"
           "  -P, --full-path          show full path to the entries\n"
           "  -c, --color              display file name with color\n"
           "  -L, --info               print entries informations\n"
           "  -I, --put-inode          print inode for file finds\n"
           "  -l, --line               print line to find word in file\n"
           "                           (just with -i argument)\n"
           "  -p, --print              print first line to find word\n"
           "  -V, --print-all          print all line to found word\n"
           "  -x, --exit [N]           exit program after N result finds\n"
           "  -o, --no-scan [STR]      do not list entries with STR in name\n"
           "  -e, --extension [STR]    search file by extension\n"
           "  -i, --in-file [STR]      search string to file\n"
           "  -N, --name [STR]         search file to name exactly with STR\n"
           "  -n, --in-name [STR]      if STR in the file name\n"
           "  -u, --uid [UID]          search file by UID\n"
           "  -Q, --inode [INODE]      search file by inode numbers\n"
           "      --ack [STR]          like default ack program. (active options: -VlPrci)\n"
           "                             -V: Print all line where STRING (see option -i)\n"
           "                             -i: search word in file\n"
           "                             -l: Print line number\n"
           "                             -P: print full path\n"
           "                             -r: recusive\n"
           "                             -c: color\n",
           program_name, program_name);
    exit(EXIT_SUCCESS);
}

void
version(void)
{
    puts("sfile version 1.3.0");
    exit(EXIT_SUCCESS);
}
