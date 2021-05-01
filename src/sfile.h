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

#ifndef __SFILE_H__
#define __SFILE_H__

#include  <getopt.h>
#include  <stdint.h>

typedef uint32_t setopt_t;

#define    PATH_LEN      1024
#define    _PATH_LEN     1023

/* color list */
#define COLOR_NULL          "\033[00m"
#define COLOR_DIR           "\033[31m"
#define COLOR_REG_FILE      "\033[34m"
#define COLOR_BACKUP        "\033[33m"
#define COLOR_ARCHIVE       "\033[35m"

/* no short program argument list */
enum long_program_arguments {
    OPT_ACK_LIKE = 1, /* combination of options: -VlPrci */
};

#define STR_OPT_INDEX    "hvaErDFBAPcIlLpVx:Q:u:o:e:i:N:n:G:"

/* default mode, just list file in current directory */
#define  O_LS_MODE      0x00000001

/* list all object in directory */
#define  O_ALL          0x00000002

/* search in dir to the $PATH variable */
#define  O_ENV_PATH     0x00000004

/* set recursive mode */
#define  O_RECURSIVE    0x00000008

/* ignore all directory */
#define  O_IGN_DIR      0x00000010

/* ignore regular file */
#define  O_IGN_FILE     0x00000020

/* ignore backup (file ending by ~) */
#define  O_IGN_BACKUP   0x00000040

/* ignore archive */
#define  O_IGN_ARCHIVE  0x00000080

/* print object full path */
#define  O_FULL_PATH    0x00000100

/* color output */
#define  O_COLOR        0x00000200

/* print inode to find object */
#define  O_PUT_INODE    0x00000400

/* print number line to find word */
#define  O_NUM_LINE     0x00000800

/* print file informations */
#define  O_FILE_INFOS   0x00001000

/* print first line to find word */
#define  O_PRINT        0x00002000

/* print all line to find word */
#define  O_ALL_PRINT    0x00004000

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
    TF_OTHER
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
    int retline;
    setopt_t opts;
    char *ext;
    char *wif;   /* Word In File */
    char *win;   /* Word In Name */
    char *wnf;   /* Word Name File */
    char *ign;
    char *ign_ext;
    char *p_current_path;
    struct stack_s line;
};

struct finfo_s {
    char fi_path[PATH_LEN];
    const char *fi_name;
    enum file_type_e fi_type;
    struct stat fi_stat;
};

struct option const opt_index[] =
     {
          {"help",        no_argument,       NULL, 'h'},
          {"version",     no_argument,       NULL, 'v'},
          {"all",         no_argument,       NULL, 'a'},
          {"env-path",    no_argument,       NULL, 'E'},
          {"recursive",   no_argument,       NULL, 'r'},
          {"ign-dir",     no_argument,       NULL, 'D'},
          {"ign-file",    no_argument,       NULL, 'F'},
          {"ign-backup",  no_argument,       NULL, 'B'},
          {"ign-archive", no_argument,       NULL, 'A'},
          {"ign-ext",     required_argument, NULL, 'G'},
          {"full-path",   no_argument,       NULL, 'P'},
          {"color",       no_argument,       NULL, 'c'},
          {"put-inode",   no_argument,       NULL, 'I'},
          {"line",        no_argument,       NULL, 'l'},
          {"info",        no_argument,       NULL, 'L'},
          {"print",       no_argument,       NULL, 'p'},
          {"print-all",   no_argument,       NULL, 'V'},
          {"exit",        required_argument, NULL, 'x'},
          {"no-scan",     required_argument, NULL, 'o'},
          {"extension",   required_argument, NULL, 'e'},
          {"in-file",     required_argument, NULL, 'i'},
          {"name",        required_argument, NULL, 'N'},
          {"in-name",     required_argument, NULL, 'n'},
          {"uid",         required_argument, NULL, 'u'},
          {"inode",       required_argument, NULL, 'Q'},
          {"ack",         required_argument, NULL, OPT_ACK_LIKE},
          {NULL,          0,                 NULL, 0}
     };

void set_program_name(const char *arg0);
void decode_program_param(int argc, char **argv, struct opt_s *x);
void init_sfile_options(struct opt_s *w);
void scan_arg_object(int argc, char **argv, struct opt_s *x);
void scan_path_environ(struct opt_s *x);
void set_object_path(char *name, setopt_t full);
int get_current_dir(char *current_path);
enum file_type_e get_file_type(struct finfo_s *fi);
int object_is_archive(const char *name);
void list_dir_object(struct opt_s *x, const char *path);
void check_object(struct opt_s *x, struct finfo_s *finfo);
int cmp_file_extension(const char *name, const char *ext);
int word_in_file(struct opt_s *x, const char *path_file);
void push_dir_stack(struct stack_s *stack, const char *path);
void push_line_stack(struct stack_s *stack, setopt_t print,
                     char *line, long n);
void sfile_print_object(struct opt_s *x, struct finfo_s *fi);
void print_perm_object(mode_t mode);
int object_is_suid(mode_t mode, int flag, int flag_x);
void print_user_object(uid_t uid);
void print_object_name(struct finfo_s *fi, struct opt_s *x);
void print_line_object(struct stack_chunk_s *chunk,  struct opt_s *x);
void *xmalloc(size_t size);
char *xstrdup(const char *str);
void xfree(void *ptr);
void out_memory(const char *func_name);
int xstrtol_fatal(const char *str, const char *err_msg);
void usage(void);
void version(void);

const char *program_name;
const char *tab_archive[] =             /*  list archive extension       */
     {".gz", ".bz2", ".zip", ".rar", ".7z", NULL};

#endif /* not __SFILE_H__ */
