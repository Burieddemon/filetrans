#ifndef _USER_INFO_H_
#define _USER_INFO_H_

#include "../server.h"
#include "kernel_list.h"
#include "../01_线程池/thread_pool.h"

#define MSGSIZE 1024

struct user{
	char name[20];				// 客户端名字
	char pass[10];				// 密码
	int passSize;				// 密码长度
	enum{MAN, WOMAN}sex;	// 性别
	char dir_name[20];
	int s;
};

// 用户信息结构体
struct user_info{
	int confd;					// 通信套接字
	struct sockaddr_in addr;	// 客户端ip+port
	struct user u;
};

//用户登录信号
struct msg
{
enum sig
{
	A,B,C,D,E,F
}s;
enum {
	MESSAGE, MSG_FILE,TUI_CHU
	}msg_type;
enum{
	CHOICE_A,CHOICE_B,CHOICE_C,CHOICE_D,CHOICE_O
}msg_file;
enum{
	FILE_OK,FILE_NO
}msg_whether;
enum{
	PERSON,LOCAL
}msg_or;
};

struct user_list
{
	struct user_info data;			//数据
	struct list_head list;	//包含内核链表
};

// 初始化一条内核链表
extern struct user_list *list_init(void);
// 添加用户
extern struct user_list *  add_user(struct user_info *data, struct user_list *head);
// 遍历用户列表
extern void dsipaly_list(struct user_list *head);
// 删除用户
extern void del_user(struct user_info *data, struct user_list *head);
// 检查data中的用户ip是否已在用户列表中
extern struct user_list * find_user(struct sockaddr_in *addr, struct user_list *head);
//检查用户名是否存在
extern struct user_list * find_user_u(struct user *u, struct user_list *head);
//从文件中初始化列表
extern void read_file_stu(struct user_list *head);

typedef void (*Amp)(int, struct user_list *);
#endif