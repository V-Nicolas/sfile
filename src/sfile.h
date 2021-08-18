/*
 *  sfile
 *  src/sfile.h
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

#ifndef SFILE_H
#define SFILE_H

#include  <getopt.h>
#include  <stdint.h>
#include  <sys/types.h>

#define EMPTY_STRING "\0"

#ifndef PATH_LEN
# define PATH_LEN      1024
# define PATH_LEN_USE  1023
#else
# define PATH_LEN_USE (PATH_LEN-1)
#endif /* PATH_LEN */

#ifndef LINE_BUFSIZE
# define LINE_BUFSIZE 4096
#endif /* !LINE_BUFSIZE */

#ifndef ENV_VAR_PATH
# define ENV_VAR_PATH "PATH"
#endif /* !ENV_VAR_PATH */

/* color list */
#define COLOR_NULL          "\033[00m"
#define COLOR_DIR           "\033[31m"
#define COLOR_REG_FILE      "\033[34m"
#define COLOR_BACKUP        "\033[33m"
#define COLOR_ARCHIVE       "\033[35m"

/* no short program argument list */
enum long_program_arguments {
    OPT_ACK_LIKE = 1, /* combination of options: -VlPrci */
    OPT_IGN_CASE_IN_FILE = 2,
    OPT_IGN_CASE_FILE_NAME = 3,
    OPT_WIF_COUNT = 4,
};

#define STR_OPT_INDEX    "hvawrDFBAPcIlLpVCx:Q:u:o:e:i:N:n:G:"

/* enumeration of all x->optq value */
enum sfile_options_values {
    /* default mode, just list file in current directory */
    O_LS_MODE = 0x00000001,

    /* list all object in directory */
    O_ALL = 0x00000002,

    /* search in dir to the $PATH variable */
    O_ENV_PATH = 0x00000004,

    /* set recursive mode */
    O_RECURSIVE = 0x00000008,

    /* ignore all directory */
    O_IGN_DIR = 0x00000010,

    /* ignore regular file */
    O_IGN_FILE = 0x00000020,

    /* ignore backup (file ending by ~) */
    O_IGN_BACKUP = 0x00000040,

    /* ignore archive */
    O_IGN_ARCHIVE = 0x00000080,

    /* print object full path */
    O_FULL_PATH = 0x00000100,

    /* color output */
    O_COLOR = 0x00000200,

    /* print inode to find object */
    O_PUT_INODE = 0x00000400,

    /* print number line to find word */
    O_NUM_LINE = 0x00000800,

    /* print file informations */
    O_FILE_INFOS = 0x00001000,

    /* print first line to find word */
    O_PRINT = 0x00002000,

    /* print all line to find word */
    O_ALL_PRINT = 0x00004000,

    /* Ignore case distinctions in file word */
    O_IGN_CASE_IN_FILE = 0x00008000,

    /* Ignore case distinctions in file name */
    O_IGN_CASE_FILE_NAME = 0x00010000,

    /* Count number result for word in file options */
    O_WIF_COUNT = 0x00020000
};

/* append new chunk to stack */
#define APPENDTOSTACK(stack, new)       \
  do {                                  \
      new->next = NULL;                 \
      if (stack->chunk)                 \
          stack->tail->next = new;      \
      else                              \
          stack->chunk = new;           \
      stack->tail = new;                \
    }  while (0)


#define NEED_CUSTOM_OUTPUT(x) ((x->opts & O_FULL_PATH) &&          \
                                  (x->opts & O_COLOR) &&           \
                                  x->line.chunk &&                 \
                                  ((x->opts & O_PRINT) ||          \
                                   (x->opts & O_ALL_PRINT)))

enum file_type_e {
    TF_REG,
    TF_DIR,
    TF_BACKUP,
    TF_ARCHIVE,
    TF_OTHER,
    TF_ERROR,
};

struct line_s {
    long n;
    char *line;
#define LINE_S(y)    ((struct line_s *) y->un.data)->line
#define LINE_N(y)    ((struct line_s *) y->un.data)->n
};

struct stack_chunk_s {
    union {
        void *data;
        char *str;
    } un;
    struct stack_chunk_s *next;
};

struct stack_s {
    struct stack_chunk_s *tail;
    struct stack_chunk_s *chunk;
};

struct opt_s {
    int n_exit;
    int byuid;
    int byino;
    uint32_t opts;
    unsigned long n_wif_result;
    char *ext;
    char *wif;   /* Word In File */
    char *win;   /* Word In Name */
    char *wnf;   /* Word Name File */
    char *ign;
    char **ign_ext;
    char *p_current_path;
    char *(*searchstring_wif)(const char *, const char *);
    char *(*searchstring_win)(const char *, const char *);
    int (*cmpstring_wnf)(const char *, const char *);
    struct stack_s line;
};

struct finfo_s {
    char fi_path[PATH_LEN];
    const char *fi_name;
    enum file_type_e fi_type;
    struct stat fi_stat;
};

static struct option const opt_index[] =
     {
          {"help",               no_argument,       NULL, 'h'},
          {"version",            no_argument,       NULL, 'v'},
          {"all",                no_argument,       NULL, 'a'},
          {"which",              no_argument,       NULL, 'w'},
          {"recursive",          no_argument,       NULL, 'r'},
          {"ign-dir",            no_argument,       NULL, 'D'},
          {"ign-file",           no_argument,       NULL, 'F'},
          {"ign-backup",         no_argument,       NULL, 'B'},
          {"ign-archive",        no_argument,       NULL, 'A'},
          {"ign-ext",            required_argument, NULL, 'G'},
          {"full-path",          no_argument,       NULL, 'P'},
          {"color",              no_argument,       NULL, 'c'},
          {"put-inode",          no_argument,       NULL, 'I'},
          {"line",               no_argument,       NULL, 'l'},
          {"info",               no_argument,       NULL, 'L'},
          {"print",              no_argument,       NULL, 'p'},
          {"print-all",          no_argument,       NULL, 'V'},
          {"ign-case",           no_argument,       NULL, 'C'},
          {"ign-case-in-file",   no_argument,       NULL, OPT_IGN_CASE_IN_FILE},
          {"ign-case-file-name", no_argument,       NULL, OPT_IGN_CASE_FILE_NAME},
          {"count",              no_argument,       NULL, OPT_WIF_COUNT},
          {"exit",               required_argument, NULL, 'x'},
          {"no-scan",            required_argument, NULL, 'o'},
          {"extension",          required_argument, NULL, 'e'},
          {"in-file",            required_argument, NULL, 'i'},
          {"name",               required_argument, NULL, 'N'},
          {"in-name",            required_argument, NULL, 'n'},
          {"uid",                required_argument, NULL, 'u'},
          {"inode",              required_argument, NULL, 'Q'},
          {"ack",                required_argument, NULL, OPT_ACK_LIKE},
          {NULL,                 0,                 NULL, 0}
     };

void sfile_init(struct opt_s *x);
void sfile_free(struct opt_s *x);
void set_program_name(const char *arg0);
void decode_program_param(int argc, char **argv, struct opt_s *x);
void scan_arg_object(int argc, char **argv, struct opt_s *x);
void scan_path_environ(struct opt_s *x);
void set_object_path(char *name, uint32_t full);
int get_current_dir(char *current_path);
enum file_type_e get_file_type(struct finfo_s *fi);
int object_is_archive(const char *name);
void list_dir_object(struct opt_s *x, const char *path);
void check_object(struct opt_s *x, struct finfo_s *finfo);
int ign_file_extension(const char *name, char **ext);
int cmp_file_extension(const char *name, const char *ext);
int word_in_file(struct opt_s *x, const char *path_file);
void push_dir_stack(struct stack_s *stack, const char *path);
void push_line_stack(struct stack_s *stack, uint32_t print,
                     char *line, long n);
void sfile_print_object(struct opt_s *x, struct finfo_s *fi);
void print_perm_object(mode_t mode);
unsigned char object_have_suid_bit(mode_t mode, unsigned int flag, unsigned int flag_x);
void print_user_object(uid_t uid);
void print_object_name(struct finfo_s *fi, struct opt_s *x);
void print_line_object(struct stack_chunk_s *chunk, struct opt_s *x);
void *xmalloc(size_t size);
char *xstrdup(const char *str);
void xfree(void *ptr);
void free_str_array(char **array);
void out_memory(const char *func_name) __attribute__((noreturn));
int xstrtol_fatal(const char *str, const char *err_msg);
char *xstrcasestr(const char *str, const char *substr);
char **parse_str_array(const char *arg);
void usage(void) __attribute__((noreturn));
void version(void) __attribute__((noreturn));

/*  list archive extension */
const char *tab_archive[] =
     {".gz", ".bz2", ".zip", ".rar", ".7z", NULL};

#endif /* not have SFILE_H */
