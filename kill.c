//alter

#include "apue.h"
#include <fcntl.h>
#include <dirent.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/resource.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>//lack this header file
#include "def.h"

mode_t perm;
int flags;
struct passwd *pwd;
struct group *grp;
//#define  FILE_TYPE （S_IFREG |S_IFBLK| S_IFCHR |S_IFIFO| S_IFLNK| S_IFSOCK| S_IFDIR）
#define NGROUPS_MAX 128
#define SG_SIZE (NGROUPS_MAX+1)

typedef struct ITEMS {
        int fd;
        char* pathname;
        int type;
        struct stat* statbuf;
        const char* ownertype;
        uid_t uid;
        uid_t ruid;
        uid_t euid;
        uid_t suid;
        uid_t fsuid;
        gid_t gid;
        gid_t rgid;
        gid_t egid;
        gid_t sgid;
        gid_t fsgid;
        gid_t suppgroups[SG_SIZE];
        const char* cmdstring;
};

static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;
int main(int argc, char* argv[])
{
        int ret;
        struct ITEMS* items;
        if (argc != 2)
                err_quit("usage: ftw <starting-pathname>");
        ret = myftw(argv[1], myfunc);
        printf("regular files=%71d   fd= %71d  pathname= %71s  ownertype= %71s  cmdstring= &71s ruid= %71d"
                "euid= %71d  suid= %71d  fsuid= %71d  rgid= %71d  egid= %71d  sgid= %71d fsgid= %71d  suppgroups= %71d\n",
                nreg, items->fd, items->pathname, items->ownertype, items->cmdstring,
                items->ruid, items->euid, items->suid, items->fsuid, items->rgid, items->egid, items->sgid,
                items->fsgid, items->suppgroups);
        printf("block special=%71d   fd= %71d  pathname= %71s  ownertype= %71s  cmdstring= &71s ruid= %71d"
                "euid= %71d  suid= %71d  fsuid= %71d  rgid= %71d  egid= %71d  sgid= %71d fsgid= %71d  suppgroups= %71d\n",
                nblk, items->fd, items->pathname, items->ownertype, items->cmdstring,
                items->ruid, items->euid, items->suid, items->fsuid, items->rgid, items->egid, items->sgid,
                items->fsgid, items->suppgroups);
        printf("char special=%71d   fd= %71d  pathname= %71s  ownertype= %71s  cmdstring= &71s ruid= %71d"
                "euid= %71d  suid= %71d  fsuid= %71d  rgid= %71d  egid= %71d  sgid= %71d fsgid= %71d  suppgroups= %71d\n",
                nchr, items->fd, items->pathname, items->ownertype, items->cmdstring,
                items->ruid, items->euid, items->suid, items->fsuid, items->rgid, items->egid, items->sgid,
                items->fsgid, items->suppgroups);
        printf("FIFOS = %71d   fd= %71d  pathname= %71s  ownertype= %71s  cmdstring= &71s ruid= %71d"
                "euid= %71d  suid= %71d  fsuid= %71d  rgid= %71d  egid= %71d  sgid= %71d fsgid= %71d  suppgroups= %71d\n",
                nfifo, items->fd, items->pathname, items->ownertype, items->cmdstring,
                items->ruid, items->euid, items->suid, items->fsuid, items->rgid, items->egid, items->sgid,
                items->fsgid, items->suppgroups);
        printf("symbolic link=%71d   fd= %71d  pathname= %71s  ownertype= %71s  cmdstring= &71s ruid= %71d"
                "euid= %71d  suid= %71d  fsuid= %71d  rgid= %71d  egid= %71d  sgid= %71d fsgid= %71d  suppgroups= %71d\n",
                nslink, items->fd, items->pathname, items->ownertype, items->cmdstring,
                items->ruid, items->euid, items->suid, items->fsuid, items->rgid, items->egid, items->sgid,
                items->fsgid, items->suppgroups);
        printf("sockets = %71d    fd= %71d  pathname= %71s  ownertype= %71s  cmdstring= &71s ruid= %71d"
                "euid= %71d  suid= %71d  fsuid= %71d  rgid= %71d  egid= %71d  sgid= %71d fsgid= %71d  suppgroups= %71d\n",
                nsock, items->fd, items->pathname, items->ownertype, items->cmdstring,
                items->ruid, items->euid, items->suid, items->fsuid, items->rgid, items->egid, items->sgid,
                items->fsgid, items->suppgroups);
        printf("directories=%71d    fd= %71d  pathname= %71s  ownertype= %71s  cmdstring= &71s ruid= %71d"
                "euid= %71d  suid= %71d  fsuid= %71d  rgid= %71d  egid= %71d  sgid= %71d fsgid= %71d  suppgroups= %71d\n",
                ndir, items->fd, items->pathname, items->ownertype, items->cmdstring,
                items->ruid, items->euid, items->suid, items->fsuid, items->rgid, items->egid, items->sgid,
                items->fsgid, items->suppgroups);
        exit(ret);
}

#define FTW_F 1
#define FTW_D 2
#define FTW_DNR 3
#define FTW_NS 4

static  char* fullpath;
static size_t pathlen;

static int
myftw(char* pathname, Myfunc* func)
{
        fullpath = path_alloc(&pathlen);
        if (pathlen <= strlen(pathname)) {
                pathname = strlen(pathname) * 2;
                if ((fullpath = realloc(fullpath, pathlen)) == NULL)
                        err_sys("realloc failed");
        }
        strcpy(fullpath, pathname);
        return(trapath(func));
}

static int
trapath(Myfunc* func)
{

        struct stat statbuf;
        struct dirent* dirp;
        DIR* dp;
        int ret, n;
        if (lstat(fullpath, &statbuf) < 0)
                return (func(fullpath, &statbuf, FTW_NS));
        if (S_ISDIR(statbuf.st_mode) == 0)
                return(func(fullpath, &statbuf, FTW_F));
        if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
                return(ret);
        n = strlen(fullpath);
        if (n + NAME_MAX + 2 > pathlen) {
                pathlen *= 2;
                if ((fullpath = realloc(fullpath, pathlen)) == NULL)
                        err_sys("realloc failed");
        }
        fullpath[n++] = '/';
        fullpath[n] = 0;
        if ((dp = opendir(fullpath)) == NULL)
                return (func(fullpath, &statbuf, FTW_DNR));
        while ((dirp = readdir(dp)) != NULL) {
                if (strcmp(dirp->d_name, ".") == 0 ||
                        strcmp(dirp->d_name, "..") == 0)
                        continue;
                strcpy(&fullpath[n], dirp->d_name);
                if ((ret = trapath(func)) != 0)
                        break;
        }
        fullpath[n - 1] = 0;
        if (closedir(dp) < 0)
                err_ret("can't close directory %s", fullpath);
        return(ret);
}

#define S_IFREG 1
#define S_IFBLK 2
#define S_IFCHR 3
#define S_IFIFO 4
#define S_IFLNK 5
#define S_IFSOCK 6
#define S_IFDIR 7

static int
myfunc(const char* pathname, const struct stat* statptr, int type)
                                                                                                                  
{
        struct ITEMS* items;
        switch (type) {
        case FTW_F:
                for (type = 1; type <= 7; type++)
                {
                        printf("start to check");
                        switch (statptr->st_mode & S_IFMT) {
                        case S_IFREG:
                                type = 1;
                                nreg++;
                                printf("regular files");
                                filestate(items->fd, pathname, type);
                                filepermstr(perm, flags);
                                ciruid(items->uid, items->cmdstring, type);
                                cirgid(items->gid, items->cmdstring, type);
                                break;
                        case S_IFBLK:
                                type = 2;
                                nblk++;
                                printf("block special");
                                filestate(items->fd, pathname,type);
                                filepermstr(perm, flags);
                                //fileowner(pathname, items->statbuf, items->ownertype, type);
                                ciruid(items->uid, items->cmdstring, type);
                                cirgid(items->gid, items->cmdstring, type);
                                break;
                        case S_IFCHR:
                                type = 3;
                                nchr++;
                                printf("char special");
                                filestate(items->fd, pathname, type);
                                filepermstr(perm, flags);
                                //fileowner(pathname, items->statbuf, items->ownertype, type);
                                ciruid(items->uid, items->cmdstring, type);
                                cirgid(items->gid, items->cmdstring, type);
                                break;
                        case S_IFIFO:
                                type = 4;
                                nfifo++;
                                printf("fifos");
                                filestate(items->fd, pathname, type);
                                filepermstr(perm, flags);
                                //fileowner(pathname, items->statbuf, items->ownertype, type);
                                ciruid(items->uid, items->cmdstring, type);
                                cirgid(items->gid, items->cmdstring, type);
                                break;
                        case S_IFLNK:
                                type = 5;
                                nslink++;
                                printf("symbolic links");
                                filestate(items->fd, pathname, type);
                                filepermstr(perm, flags);
                                //fileowner(pathname, items->statbuf, items->ownertype, type);
                                ciruid(items->uid, items->cmdstring, type);
                                cirgid(items->gid, items->cmdstring, type);
                                break;
                        case S_IFSOCK:
                                type = 6;
                                nsock++;
                                printf("sockets");
                                filestate(items->fd, pathname, type);   
                                filepermstr(perm, flags);
                                //fileowner(pathname, items->statbuf, items->ownertype, type);
                                ciruid(items->uid, items->cmdstring, type);
                                cirgid(items->gid, items->cmdstring, type);
                                break;
                        case S_IFDIR:
                                type = 7;
                                ndir++;
                                printf("directories");
                                filestate(items->fd, pathname, type);
                                filepermstr(perm, flags);
                                //fileowner(pathname, items->statbuf, items->ownertype, type);
                                ciruid(items->uid, items->cmdstring, type);
                                cirgid(items->gid, items->cmdstring, type);
                                break;
                                err_dump("for S_IFDIR for %s", pathname);
                        }
                }
                break;
        case FTW_D:
                printf("directories");
                break;
        case FTW_DNR:
                err_ret("can't read directory %s", pathname);
        case FTW_NS:
                err_ret("stat error for %s", pathname);
                break;
        default:
                err_dump("unknown type %d for pathname %s", type, pathname);
        }
        return(0);
}



int filestate(int fd, const char* pathname, int type)// struct stat* statbuf
{
        int val;
        if ((val = fcntl(fd, F_GETFL, 0)) < 0)
                err_sys("fcntl F_GETFL error for fd %d and %s", fd, pathname);
        switch (val & O_ACCMODE) {
        case O_RDONLY:
                printf("read only");
                break;
        case O_WRONLY:
                printf("write only");
                break;
        case O_RDWR:
                printf("read write");
                break;
        default:
                err_dump("unknown access mode");
        }
}

/*
#define FILE_MODE (S_IRUSR |S_IWUSR |S_IXUSR |S_IRGRP |IWGRP |S_IXGRP |S_IROTH |S_IWOTH |S_IXOTH)

int fileowner(const char* pathname, struct stat buf, const char* ownertype, int type)
{
        int i;
        for (type = 1; type <= 7; type++)
        {
                for (i = 1; i < strlen(pathname); i++) {
                        printf("%s:", pathname[i]);
                        if (lstat(pathname, &buf) < 0) {
                                err_ret("lstat error");
                                continue;
                        }
                        if (S_IRUSR(buf.st_mode))
                                ownertype = "usr read";
                        else if (S_IWUSR(buf.st_mode))
                                ownertype = "usr write";
                        else if (S_IXUSR(buf.st_mode))
                                ownertype = "usr executive";
                        else if (S_IRGRP(buf.st_mode))
                                ownertype = "group read";
                        else if (S_IWGRP(buf.st_mode))
                                ownertype = "group wrtie";
                        else if (S_IXGRP(buf.st_mode))
                                ownertype = "group executive";
                        else if (S_IROTH(buf.st_mode))
                                ownertype = "other read";
                        else if (S_IWOTH(buf.st_mode))
                                ownertype = "other write";
                        else if (S_IXOTH(buf.st_mode))
                                ownertype = "other executive";
                        else
                                ownertype = "unknow mode";
                        printf("%s\n", ownertype);
                }
        }
        exit(0);
}
*/

#define FP_SPECIAL 1
#define STR_SIZE sizeof("rwxrwxrwx")

char *filepermstr(mode_t perm, int flags)
{
        static char str[STR_SIZE];
        snprintf(str, STR_SIZE, "%c%c%c%c%c%c%c%c%c",
                        (perm& S_IRUSR)? 'r':'-', (perm &S_IWUSR)?'w':'-',
                        (perm& S_IXUSR)?
                        (((perm &S_ISUID) &&(flags &FP_SPECIAL))?'s':'x'):
                        (((perm &S_ISUID) &&(flags &FP_SPECIAL))?'S':'-'),
                        (perm &S_IRGRP) ?'r':'-', (perm &S_IWGRP)?'w':'-',
                        (perm &S_IXGRP)?
                                (((perm &S_ISGID)&&(flags &FP_SPECIAL))?'s':'x'):
                                (((perm &S_ISGID) &&(flags &FP_SPECIAL)) ?'S':'-'),
                        (perm &S_IROTH)? 'r':'-', (perm &S_IWOTH)?'w':'-',
                        (perm &S_IXOTH)?
                                (((perm &S_ISVTX) && (flags &FP_SPECIAL))?'t':'x'):
                                (((perm &S_ISVTX) &&(flags &FP_SPECIAL))?'T':'-'));
        return str;
}


/*int cirpid(pid_t pid, const char* cmdstring, const char* filename)//1.cmd or string 2.file
{
        pid_t pid;
        char* fname;
}*/

int ciruid(uid_t uid, const char* cmdstring, int type) //const char* name
{
        uid_t ruid, euid, suid, fsuid;
        for (type = 1; type <= 7; type++)
        {
                if (getresuid(&ruid, &euid, &suid) == -1)
                        err_sys("getresuid error");
                printf("UID:");
                cmdstring = check_usrid(ruid);
                printf("real=%s(%ld);", (cmdstring = NULL) ? "???" : cmdstring, (long)ruid);
                cmdstring = check_usrid(euid);
                printf("eff=%s(%ld);", (cmdstring = NULL) ? "???" : cmdstring, (long)euid);
                cmdstring = check_usrid(suid);
                printf("saved=%s(%ld);", (cmdstring = NULL) ? "???" : cmdstring, (long)suid);
                cmdstring = check_usrid(fsuid);
                printf("fs=%s(%ld);", (cmdstring = NULL) ? "???" : cmdstring, (long)fsuid);
                printf("\n");
        }
}

int cirgid(gid_t gid, const char* cmdstring, int type)// const char* name  1.real,saved,effective
{
        gid_t rgid, egid, sgid, fsgid;
        int numgroups, j;//p149
        gid_t suppgroups[SG_SIZE];
        for (type = 1; type <= 7; type++)
        {
                if (getresgid(&rgid, &egid, &sgid) == -1)
                        err_sys("getresgid error");
                printf("GID:");
                cmdstring = check_grpid(rgid);
                printf("real=%s(%ld);", (cmdstring = NULL) ? "???" : cmdstring, (long)rgid);
                cmdstring = check_grpid(egid);
                printf("eff=%s(%ld);", (cmdstring = NULL) ? "???" : cmdstring, (long)egid);
                cmdstring = check_grpid(sgid);
                printf("saved=%s(%ld);", (cmdstring = NULL) ? "???" : cmdstring, (long)sgid);
                cmdstring = check_grpid(fsgid);
                printf("fs=%s(%ld);", (cmdstring = NULL) ? "???" : cmdstring, (long)fsgid);
                printf("\n");

                numgroups = getgroups(SG_SIZE, suppgroups);
                if (numgroups == -1)
                        err_sys("getgroups");
                printf("supplementary groups(%d):", numgroups);
                for (j = 0; j < numgroups; j++) {
                        cmdstring = check_grpid(suppgroups[j]);
                        printf("%s (%ld)", (cmdstring == NULL ? "???" : cmdstring, (long)suppgroups[j]));
                }
                printf("\n");
                exit(0);
        }
}

char *check_usrid(uid_t uid)
{
        struct passwd *pwd;
        pwd=getpwuid(uid);
        return(pwd==NULL)?NULL :pwd->pw_name;
}

char *check_grpid(gid_t gid)
{
        struct group *grp;
        grp=getgrpid(gid);
        return (grp==NULL)?NULL :grp->gr_name;
}

/*int prc_con_file(int fp, const char*, fpos_t* restrict pos, off_t offset)//1.whence 2.pathname/filename
{
        //空白，因为这是关键一步
}
*/


