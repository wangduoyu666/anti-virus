#ifndef _DEF_H_    //暂定为草稿，反正还会一直修改的
#define _DEF_H_
#define _POSIX_C_SOURCE 200809L
#define PATH_MAX 4096
#define NAME_MAX 255

#include "apue.h"

typedef int Myfunc(const char *, const struct stat *, int);

char *filepermstr(mode_t perm, int flags);

static Myfunc myfunc;

int myftw(char*, Myfunc*);

int trapath(Myfunc*);

int Myfunc(const char*, const struct stat*, int);

int filestate(int,const char* , int);//1.pathname

char *filepermstr(mode_t perm, int flags);

//int fileowner(const char*,struct stat*, const char*, int);//1.name 2.type

int cirpid(pid_t pid, const char*, const char*);//1.cmd or string 2.file

int ciruid(uid_t uid, const char*, int);

int cirgid(gid_t gid, const char*, int);//1.real,saved,effective

char* check_usrid(uid_t uid);

char* check_grpid(gid_t gid);

int prc_con_file(int, const char*, fpos_t* restrict pos, off_t offset);//1.whence 2.pathname/filename


#endif 
