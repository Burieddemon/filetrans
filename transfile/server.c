#include "./02_内核列表/user_info.h"
#include "dir.h"



//设置链表的头结点
struct user_list *head = NULL;

// tcp初始化
int tcp_init(char *ip, unsigned short port)
{
	// 1. 创建TCP套接字
	int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
	if( tcp_fd < 0 )
	{
		perror("socket()");
		exit(errno);
	}
	int on = 1;
	setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
	// 2. 将套接字与IP和端口绑定
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	if(ip != NULL)
		saddr.sin_addr.s_addr = inet_addr(ip); 
	else
		saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	
	saddr.sin_port = htons(port); // 端口号PORT
	if(bind(tcp_fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
	{
		perror("bind()");
		exit(errno);
	}
	// 3. 将tcp_fd设置为被动监听套接字
	listen(tcp_fd, 3);
	
	return tcp_fd;
}

//上传接受
void file_accept_exist(int confd,struct msg m,const char *dir_name)
{
	char file_name[1024] = {0}, dir_file_name[1024] = {0};
	
	//读取文件是否存在的信号
	read(confd, &m, sizeof(m));
	
	//文件存在
	if(m.msg_whether == FILE_OK)
	{
		write(confd,"ok1",4);
		read(confd,file_name,sizeof(file_name));
		if(dir_name != NULL)
		{
			sprintf(dir_file_name,"%s/%s",dir_name,file_name);
		}
		else
		{
			sprintf(dir_file_name,"dir_gx/%s",file_name);
		}
		write(confd,"ok2",4);
		file_accept(confd,dir_file_name);
		printf("上传成功\n");
	}
	else if(m.msg_whether == FILE_NO)
	{
		printf("没有这个文件\n");
	}
}

//传输文件
void file_set(int confd , char *file_path)
{
	char buf[10] = {0};
	read(confd,buf,sizeof(buf));
	if(strstr(buf,"success"))
	{
		printf("%s\n",buf);
		file_send(confd,file_path);
	}
}

//判定用于下载的位置
bool local_or_share(int confd,struct msg m)
{
	while(1)
	{
		if(read(confd, &m, sizeof(m))==0)
		{
			break;
		}
		if(m.msg_or == LOCAL)
		{
			return true;
		}
		else if(m.msg_or == PERSON)
		{
			return false;
		}
		
	}
}

//用于读取个人或共享文件是否存在
void file_read_rm(int confd,struct user_list *pos,struct msg m,Cmp cmp)
{			
	// 定义一个用来接受文件名，一个用来拼接文件路径
	char file_name[1024]={0},dir_file_name[1024]={0};
	char buf[10] = {0};
	// 读取文件名并拼接文件路径
	read(confd,file_name,sizeof(file_name));
	if(pos !=NULL)
	{
		sprintf(dir_file_name,"%s/%s",pos->data.u.dir_name,file_name);
		printf("%s\n",dir_file_name);
	}
	else
	{
		sprintf(dir_file_name,"./dir_gx/%s",file_name);
		printf("%s\n",dir_file_name);
	}
	// 判断文件是否存在
	if(dir_exist(dir_file_name))
	{
		m.msg_whether = FILE_OK;
		write(confd,&m,sizeof(m));
		read(confd,buf,10);
		// 读取文件内容并发给客户端
		cmp(confd ,dir_file_name);
	}
	else
	{
		m.msg_whether = FILE_NO;
		write(confd,&m,sizeof(m));
		printf("没有找到这个文件\n");
	}
}

//用于共享文件下载到个人文件夹中
void my_file_read(int confd,struct user_list *pos,struct msg m)
{			
	// 定义一个用来接受文件名，一个用来拼接文件路径
	char file_name[1024]={0},dir_file_name[1024]={0},my_dir_file_name[1024]={0};
		// 读取文件名并拼接文件路径
		read(confd,file_name,sizeof(file_name));
		sprintf(dir_file_name,"./dir_gx/%s",file_name);
		printf("%s\n",dir_file_name);
	// 判断文件是否存在
	if(dir_exist(dir_file_name))
	{
		m.msg_whether = FILE_OK;
		write(confd,&m,sizeof(m));
		sprintf(my_dir_file_name,"cp %s %s/",dir_file_name,pos->data.u.dir_name);
		printf("%s\n",my_dir_file_name);
		system(my_dir_file_name);
		write(confd,"success",8);	
	}
	else
	{
		m.msg_whether = FILE_NO;
		write(confd,&m,sizeof(m));
		printf("没有找到这个文件\n");
	}
}

//用于用户的删除
void user_rm(int confd,struct user_list *pos)
{
	char rm_dir[1024] = {0};
		sprintf(rm_dir,"rm -r %s",pos->data.u.dir_name);
		system(rm_dir);
		del_user(&pos->data,head);
		dsipaly_list(head);
		write(confd,"success",8);	
}

//用于用户的修改信息
void user_modify(int confd,struct user_list *pos)
{
	struct user u;
	read(confd, &u, sizeof(u));
	if(strcmp(pos->data.u.name,u.name)==0)
	{
		pos->data.u = u;
	}
	else
	{
		char buf[1024] = {0};
		sprintf(buf,"mv %s %s",pos->data.u.dir_name,u.dir_name);
		system(buf);
		pos->data.u = u;
	}
	dsipaly_list(head);
	write(confd,"success",8);
}


//用于用户的删除或修改查找用户
void user_rm_or_modify(int confd,struct msg m,Amp amp)
{
	char user_name[20]={0};
	read(confd,user_name,sizeof(user_name));
	struct user_info data;
	strcpy(data.u.name,user_name);
	struct user_list *pos = find_user_u(&data.u,head);
	if(pos != NULL)
	{
		m.msg_whether = FILE_OK;
		write(confd,&m,sizeof(m));
		amp(confd,pos);
	}
	else
	{
		m.msg_whether = FILE_NO;
		write(confd,&m,sizeof(m));
		printf("没有找到这个用户\n");
	}
}

//用户账号管理
void user_account(int confd,struct msg m)
{
	file_send(confd ,"./student_info.txt");
	while(1)
	{
		if(read(confd, &m, sizeof(m))==0)
		{
			break;
		}

		
		// 删除用户
		if(m.msg_file == CHOICE_A)
		{
			user_rm_or_modify(confd,m,user_rm);
		}
		
		// 修改用户信息
		else if(m.msg_file == CHOICE_B)
		{
			user_rm_or_modify(confd,m,user_modify);
		}
		
		// 退出
		else if(m.msg_file == CHOICE_O)
		{
			break;
		}
		
	}
}

//个人文件夹
void personal_folder(struct user_list *hui,struct user_list *pos,struct msg m)
{
	
	printf("进入个人文件夹：%s\n",pos->data.u.dir_name);
	dir_view(pos->data.u.dir_name,hui->data.confd);
	while(1)
	{
		if(read(hui->data.confd, &m, sizeof(m))==0)
		{
			break;
		}

		
		// 打开文件显示文件内容
		if(m.msg_file == CHOICE_A)
		{
			file_read_rm(hui->data.confd,pos,m,file_send);
		}
		
		// 删除文件
		else if(m.msg_file == CHOICE_B)
		{
			file_read_rm(hui->data.confd,pos,m,file_delete);
		}
		
		// 上传文件
		else if(m.msg_file == CHOICE_C)
		{
			file_accept_exist(hui->data.confd,m,pos->data.u.dir_name);
		}
		// 下载文件
		else if(m.msg_file == CHOICE_D)
		{
			file_read_rm(hui->data.confd,pos,m,file_set);
		}
		
		// 退出
		else if(m.msg_file == CHOICE_O)
		{
			break;
		}
		
	}
}


//共享文件夹
void share_folder(int confd,struct user_list *pos,struct msg m)
{
	char share_file[10]={"./dir_gx"};
	printf("进入共享文件夹：%s\n",share_file);
	dir_view(share_file,confd);
	
	while(1)
	{
		if(read(confd, &m, sizeof(m))==0)
		{
			break;
		}

		
		// 打开文件显示文件内容
		if(m.msg_file == CHOICE_A)
		{
			file_read_rm(confd,NULL,m,file_send);
		}
		
		// 下载文件
		else if(m.msg_file == CHOICE_B)
		{
			if(local_or_share(confd,m))
			{
				file_read_rm(confd,NULL,m,file_set);
			}
			else
			{
				my_file_read(confd,pos,m);
			}
		}
		
		// 退出
		else if(m.msg_file == CHOICE_O)
		{
			break;
		}
		
	}
	
}


//管理共享文件夹
void manage_share_folder(int confd,struct msg m)
{
	char share_file[10]={"./dir_gx"};
	printf("进入共享文件夹：%s\n",share_file);
	dir_view(share_file,confd);
	
	while(1)
	{
		if(read(confd, &m, sizeof(m))==0)
		{
			break;
		}

		
		// 打开文件显示文件内容
		if(m.msg_file == CHOICE_A)
		{
			file_read_rm(confd,NULL,m,file_send);
		}
		
		// 下载文件
		else if(m.msg_file == CHOICE_B)
		{
			file_read_rm(confd,NULL,m,file_set);
		}
		
		// 上传文件
		else if(m.msg_file == CHOICE_C)
		{
			file_accept_exist(confd,m,NULL);
		}
		
		// 删除文件
		else if(m.msg_file == CHOICE_D)
		{
			file_read_rm(confd,NULL,m,file_delete);
		}
		// 退出
		else if(m.msg_file == CHOICE_O)
		{
			break;
		}
		
	}
	
}




//用户登录界面
void login_interface(struct user_list *hui,struct user_info data,struct msg m)
{
	struct user_list *pos = find_user_u(&data.u, head);
	if(pos != NULL)
	{
		if(strcmp(pos->data.u.pass, data.u.pass)==0)
		{
			m.s=C;
			write(hui->data.confd, &m, sizeof(m));
			printf("登录成功\n");
			while(1)
			{
				if(read(hui->data.confd, &m, sizeof(m))==0)
				{
					break;
				}
				
				//进入个人文件夹
				if(m.msg_type == MESSAGE)
				{
					personal_folder(hui,pos,m);
				}
				
				// 进入公共文件夹
				else if(m.msg_type == MSG_FILE)
				{
					share_folder(hui->data.confd,pos,m);
				}
				
				// 退出
				else if(m.msg_type == TUI_CHU)
				{
					printf("退出：dir\n");
					break;
				}
				
			}
			
		}
		else
		{
			m.s=D;
			write(hui->data.confd, &m, sizeof(m));
			printf("密码错误\n");
		}
	}
	else
	{
		m.s=E;
		write(hui->data.confd, &m, sizeof(m));
		printf("用户不存在\n");
	}
		
}


//管理员登录界面
void manage_login_interface(int confd,struct user_info data,struct msg m)
{
	if(strcmp(data.u.name,"w")==0)
	{
		if(strcmp(data.u.pass, "123")==0)
		{
			m.s=F;
			write(confd, &m, sizeof(m));
			printf("登录成功\n");
			while(1)
			{
				if(read(confd, &m, sizeof(m))==0)
				{
					break;
				}
				
				//用户账号管理
				if(m.msg_type == MESSAGE)
				{
					user_account(confd,m);
				}
				
				// 公共文件夹管理
				else if(m.msg_type == MSG_FILE)
				{
					manage_share_folder(confd,m);
				}
				
				// 退出
				else if(m.msg_type == TUI_CHU)
				{
					printf("退出：dir\n");
					break;
				}
				
			}
		}
		else
		{
			m.s=D;
			write(confd, &m, sizeof(m));
			printf("密码错误\n");
		}
	}
	else
	{
		m.s=E;
		write(confd, &m, sizeof(m));
		printf("账号不存在\n");
	}	
}


//用户的登录注册控制显示界面
void *readlient(void *arg)
{	
	struct user_list *hui = (struct user_list *)arg;
	struct user_info data;
	struct msg m;
	while(1)
	{
		
		//用户端退出
		if(read(hui->data.confd, &data.u, sizeof(data.u)) ==0)
		{
		printf("用户退出\n");
		break;
		}
		
		//用户注册界面
		if(data.u.s == 1)
		{
			struct user_list *pos = find_user_u(&data.u, head);
			if( pos == NULL)
			{
			add_user(&data, head);
			m.s=A;
			write(hui->data.confd, &m, sizeof(m));
			printf("用户注册成功\n");
			mkdir(data.u.dir_name,0777);
			dsipaly_list(head);
			}
			else
			{
			m.s=B;
			write(hui->data.confd, &m, sizeof(m));
			printf("用户存在\n");
			}
		}
		
		//用户登录界面
		else if(data.u.s == 0)
		{
			login_interface(hui,data,m);
		}
			else if(data.u.s == 2)
		{
			manage_login_interface(hui->data.confd,data,m);
		}
	}
	
	
}



int main(int argc, char *argv[])
{
		// 初始化用户列表
	head = list_init();
	read_file_stu(head);
	dsipaly_list(head);
	// 初始化线程池,初始活跃线程数量15条
	thread_pool pool;
	init_pool(&pool, 20);
	
	
	// 获取一个数据报套接字
	int tcp_fd = tcp_init("192.168.1.148", 5000);
	
	//等待客户端的连接
	// 节点数据类型
	struct user_info data;
	socklen_t len = sizeof(data.addr);

//等待新客户端连接：并单独开启一个线程	
while(1)
	{
		printf("等待新客户端连接： \n");
		if( (data.confd = accept(tcp_fd, (struct sockaddr *)&data.addr, &len)) < 0 ){
			perror("accept");
			continue;
		}
		
		printf("[%s:%hu] 连接成功， 得到通信套接字：connfd = %d\n", 
							inet_ntoa(data.addr.sin_addr), 
							ntohs(data.addr.sin_port),
							data.confd);
		struct user_list *new = calloc(1, sizeof(struct user_list));
		new->data=data;
		add_task(&pool, readlient, new);
	}
	return 0;
}


