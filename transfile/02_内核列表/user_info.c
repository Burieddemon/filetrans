#include "user_info.h"


// 初始化一条内核链表
struct user_list *list_init(void)
{
	struct user_list *head = calloc(1, sizeof(struct user_list));
	if(head != NULL)
	{
		INIT_LIST_HEAD(&head->list);
	}
	return head;
}

// 检查data中的用户ip是否已在用户列表中
bool user_exist(struct user_info *data, struct user_list *head)
{
	struct user_list *pos = NULL;	
	list_for_each_entry(pos, &head->list, list)
	{
		if( strcmp(inet_ntoa(pos->data.addr.sin_addr), 
				inet_ntoa(data->addr.sin_addr)) == 0)
			return true;
	}
	return false;
}

// 检查data中的用户ip是否已在用户列表中
struct user_list * find_user(struct sockaddr_in *addr, struct user_list *head)
{
	struct user_list *pos = NULL;	
	list_for_each_entry(pos, &head->list, list)
	{
		if( strcmp(inet_ntoa(pos->data.addr.sin_addr), 
				inet_ntoa(addr->sin_addr)) == 0)
			return pos;
	}
	return NULL;
}


// 检查data中的用户ip是否已在用户列表中
struct user_list * find_user_u(struct user *u, struct user_list *head)
{
	struct user_list *pos = NULL;	
	list_for_each_entry(pos, &head->list, list)
	{
		if( strcmp(pos->data.u.name, 
				u->name) == 0)
			return pos;
	}
	return NULL;
}

// 添加用户
struct user_list * add_user(struct user_info *data, struct user_list *head)
{
	// 过滤掉已经在用户列表中的用户
	// if( user_exist(data, head) )
		// return ;
	
	// 创建一个新节点
	struct user_list *new = calloc(1, sizeof(struct user_list));
	if(new == NULL)
	{
		perror("calloc new");
		return NULL;
	}
	
	// 初始化节点的指针，初始化节点数据
	INIT_LIST_HEAD(&new->list);
	new->data = *data;
	
	// 把新节点添加到链表尾部
	list_add_tail(&new->list, &head->list);
	
	return new;
}


void read_file_stu(struct user_list *head)
{
	// 打开学生信息的文件
	FILE *fp = fopen("student_info.txt", "r");	//只读打开文件
	if(fp == NULL)
	{
		perror("fopen() fail");
		exit(errno);
	}
	struct user_info data;	//临时存储每次读出的一个学生的信息
	int ret = -1;
	
	char s[20] = {0};
	// 从文件中读取学生的信息，每次读一条
	while(1)
	{
		ret = fscanf(fp," %s\t%s\t%s\t%d\t%s\n", 
					data.u.name,
					data.u.pass,
					data.u.dir_name,
					&data.u.s,
					s);
					if(strcmp(s, "MAN")){
						data.u.sex = WOMAN;
					}else
					{
						data.u.sex = MAN;
					}
		if( ret == EOF )	//文件读取失败或者读完
		{
			if( feof(fp) )	//读取结束 end of file
				break;
			if( ferror(fp) )	//读取发生了错误
			{
				perror("fscanf() fail");
				exit(errno);
			}
		}
		add_user(&data, head);
	}
	
	// 关闭文件
	fclose(fp);
}




// 删除用户
void del_user(struct user_info *data, struct user_list *head)
{

	if(list_empty(&head->list) != 0)
	{
		printf("链表为空\n");
		return ;
	}	

	// 找到该用户
	struct user_list *pos = NULL;	
	list_for_each_entry(pos, &head->list, list)
	{
		if( strcmp(pos->data.u.name, 
				data->u.name) == 0)
			break;
	}

	// 删除节点
	list_del(&pos->list);
	free(pos);
}


// 遍历用户列表
void dsipaly_list(struct user_list *head)
{
	FILE *fp = fopen("student_info.txt", "w+");	//以清空创建打开
	if(fp == NULL)
	{
		perror("fopen() fail");
		exit(errno);
	}
	struct user_list *pos = NULL;	
	char line[100];
	memset(line, '-', 100);
	printf("%.25s\n", line);
	// printf("%*s\n", 20, line);
	list_for_each_entry(pos, &head->list, list)
	{
		printf("confd:%d [%s:%hu], %s\t%s\t%s\t%d\t%s\n", 
					pos->data.confd,
					inet_ntoa(pos->data.addr.sin_addr),
					ntohs(pos->data.addr.sin_port),
					pos->data.u.name,
					pos->data.u.pass,
					pos->data.u.dir_name,
					pos->data.u.s,
					pos->data.u.sex == MAN ? "MAN":"WOMAN");
		
		fprintf(fp," %s\t%s\t%s\t%d\t%s\n", 
					pos->data.u.name,
					pos->data.u.pass,
					pos->data.u.dir_name,
					pos->data.u.s,
					pos->data.u.sex == MAN ? "MAN":"WOMAN");
					
	}
	printf("%.25s\n", line);
	fclose(fp);
}

