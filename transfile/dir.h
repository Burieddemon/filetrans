#ifndef _DIR_H_
#define _DIR_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <dirent.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <pwd.h>
#include <grp.h>
#include <linux/input.h>


//读出目录中的文件名
extern void dir_view(char *dir_name,int confd);

//查看文件是否存在
extern bool  dir_exist(char *dir_file_name);

//读取文件的内容
extern void file_send(int confd , char *file_path);

//文件的删除
extern void file_delete(int confd , char *file_path);

//文件的接受
extern void file_accept(int confd,char *file_path);


typedef void (*Cmp)(int, char *);



#endif