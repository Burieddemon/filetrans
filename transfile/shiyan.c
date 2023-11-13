#include "./02_内核列表/user_info.h"
#include "dir.h"

typedef void (*Smp)(int,char *);

// 用于注册获取基本信息
void get_info(struct user *pu)
{
	char sex;
	printf("请输入用户名：\n");
	scanf("%s", pu->name);
	printf("请输入密码：");
	scanf("%s", pu->pass);
	pu->passSize = strlen(pu->pass);
	while(getchar() != '\n');	//清空缓冲区
	printf("请输入性别：m 或者 w\n");
	scanf("%c", &sex);
	pu->sex = sex=='m' ? MAN:WOMAN;
	while(getchar() != '\n');	//清空缓冲区
	sprintf(pu->dir_name,"./dir_yh/%s_dir",pu->name);
	pu->s = 1;
}


//用于登录获取消息
void get_login(struct user *pu)
{
	char sex;
	printf("请输入用户名：\n");
	scanf("%s", pu->name);
	printf("请输入密码：");
	scanf("%s", pu->pass);
	pu->s = 0;
}

//用于管理员登录获取消息
void get_manage_login(struct user *pu)
{
	char sex;
	printf("请输入账号：\n");
	scanf("%s", pu->name);
	printf("请输入密码：");
	scanf("%s", pu->pass);
	pu->s = 2;
}

// 客户端连接初始化
int tcp_init(void)
{
	// 1. 创建TCP套接字
	int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	
	// 2. 准备服务端的ip+port
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr("192.168.1.148"); // 自动获取本机IP
	saddr.sin_port = htons(5000); // 端口号PORT

	// 连接服务器
	connect(tcp_fd, (struct sockaddr *)&saddr, sizeof(saddr));
	printf("已连接到服务器\n");
	
	return tcp_fd;
}

//下载到共享还是当地
bool share_or_local(int tcp_fd,struct msg m)
{
	int path_choice = -1;
	printf("请选择下载到：1.当地,2.个人文件夹\n");
	while(1)
	{
		scanf("%d",&path_choice);
		if(path_choice == 1)
		{
			m.msg_or = LOCAL;
			write(tcp_fd, &m, sizeof(m));	
			return true;
		}
		else if(path_choice == 2)
		{
			m.msg_or = PERSON;
			write(tcp_fd, &m, sizeof(m));
			return false;
		}
		else
			printf("请重新输入\n");
	}
}

//上传文件
void file_upload_exist(int tcp_fd,struct msg m)
{
	char dir_name[1024] = {0},file_name[1024] = {0},dir_file_name[1024] = {0};
	printf("请输入上传的文件是在那个文件夹下\n");
	scanf("%s",dir_name);
    printf("请输入上传的文件名\n");
	scanf("%s",file_name);
	sprintf(dir_file_name,"%s/%s",dir_name,file_name);
	printf("%s\n",dir_file_name);
	//判断文件是否存在
	if(dir_exist(dir_file_name))
	{
		int a = 0;char buf[10] = {0};
		m.msg_whether = FILE_OK;
		write(tcp_fd,&m,sizeof(m));
		read(tcp_fd,buf,10);
		write(tcp_fd,file_name,strlen(file_name)+1);
		read(tcp_fd,buf,10);
		file_send(tcp_fd,dir_file_name);
		printf("上传成功\n");
	}
	else
	{
		m.msg_whether = FILE_NO;
		write(tcp_fd,&m,sizeof(m));
		printf("没有找到这个文件\n");
	}
}


//下载文件到个人
void my_file_download(int tcp_fd,char *file_name)
{
	char buf[10] = {0};
		read(tcp_fd,buf,10);
	    if(strstr(buf,"success"))
		{
			printf("下载成功\n");
		}
}


//下载文件到当地
void file_download(int tcp_fd,char *file_name)
{
	char dir_name[1024] = {0},dir_file_name[1024] = {0};
	while(1)
	{
		printf("请输入下载到哪里的文件夹\n");
		scanf("%s",dir_name);
		sprintf(dir_file_name,"%s/%s",dir_name,file_name);
		printf("%s\n",dir_file_name);
		if(dir_exist(dir_name))
		{
		
			write(tcp_fd,"success",8);
			file_accept(tcp_fd,dir_file_name);
		break;
		}
		else
		{
			bzero(dir_name,1024);
			bzero(dir_file_name,1024);
		}
	}
}


//文件读取
void file_read(int tcp_fd,char *file_name)
{
	char buf[1024] = {0};
	printf("%s :\n",file_name);
	while(1)
	{
		read(tcp_fd, buf, sizeof(buf));
		if(strstr(buf,"exit") == NULL)
		{
			printf("%s\n",buf);
		}
		else
		break;
	}
}


//文件删除判断
void file_rm(int tcp_fd,char *file_name)
{
	char buf[1024] = {0};
	read(tcp_fd,buf,1024);
	if(strstr(buf,"success") !=NULL )
	{
		printf("%s删除成功\n",file_name);
	}
	else
		printf("%s删除失败\n",file_name);
}


// 打开文件显示文件内容
void file_cat(char *file_name,int tcp_fd,struct msg m,Smp smp)
{	
	//输入要查看的文件名字
	printf("请输入文件的名字\n");
	bzero(file_name,1024);
	scanf("%s",file_name);
	write(tcp_fd,file_name,strlen(file_name)+1);
	
	//读取文件是否存在的信号
	read(tcp_fd, &m, sizeof(m));
	
	//文件存在
	if(m.msg_whether == FILE_OK)
	{
		write(tcp_fd,"ok",3);
		smp(tcp_fd,file_name);
	}
	else if(m.msg_whether == FILE_NO)
	{
		printf("没有找到这个文件\n");
	}
	
}


//用户修改信息
void user_modify(int tcp_fd,char *user_name)
{
		struct user u;
		char buf[10] = {0};
		get_info(&u);
		write(tcp_fd, &u, sizeof(u));
		read(tcp_fd,buf,sizeof(buf));
		if(strcmp(buf,"success")==0)
		{
			printf("修改成功\n");
		}
		else
		{
			printf("修改失败\n");
		}
}

//用户账号的查看
void user_cat(int tcp_fd,struct msg m,Smp smp)
{	
	//输入要查看的文件名字
	char user_name[20] = {0};
	printf("请输入用户的名字\n");
	bzero(user_name,20);
	scanf("%s",user_name);
	write(tcp_fd,user_name,strlen(user_name)+1);
	
	//读取文件是否存在的信号
	read(tcp_fd, &m, sizeof(m));
	
	//文件存在
	if(m.msg_whether == FILE_OK)
	{
		smp(tcp_fd,user_name);
	}
	else if(m.msg_whether == FILE_NO)
	{
		printf("没有找到这个用户\n");
	}
	
}


//用户账号管理界面
void user_manage(int tcp_fd,struct msg m)
{
	
	file_read(tcp_fd,"用户账号");
	
	while(1)
	{
		int choice = -1;
		printf("1.删除用户2.修改用户信息0.退出\n");
		scanf("%d",&choice);
		if(choice == 1)
		{
			m.msg_file = CHOICE_A;
			write(tcp_fd, &m, sizeof(m));
			user_cat(tcp_fd,m,file_rm);
		}
		else if(choice == 2)
		{
			m.msg_file = CHOICE_B;
			write(tcp_fd, &m, sizeof(m));
			user_cat(tcp_fd,m,user_modify);
		}
		else if(choice == 0)
		{
			m.msg_file = CHOICE_O;
			write(tcp_fd, &m, sizeof(m));
			break;
		}
		else
			printf("请从新输入\n");
	
	}
}


//个人文件夹的操作
void my_folder(int tcp_fd,struct msg m)
{
	char file_name[1024]={0};
	m.msg_type = MESSAGE;
	write(tcp_fd, &m, sizeof(m));
	
	//传输文件名
	while(1)
	{
		read(tcp_fd,file_name,sizeof(file_name));
		if(strcmp("exit",file_name))
			printf("file_name:%s\n",file_name);
		else
		{
			break;
		}
	}
	
	//对个人文件的操作
	while(1)
	{
		printf("1.查看的文件内容,2.删除文件3.上传文件,4.下载文件,0.退出\n");
		int file_choice=-1;
		scanf("%d",&file_choice);
		
		// 打开文件显示文件内容
		if(file_choice == 1)
		{
			//给服务端发送一个打开文件的信号
			m.msg_file = CHOICE_A;
			write(tcp_fd, &m, sizeof(m));
			file_cat(file_name,tcp_fd,m,file_read);
		}
		
		// 删除文件
		else if(file_choice == 2)
		{
			m.msg_file = CHOICE_B;
			write(tcp_fd, &m, sizeof(m));
			file_cat(file_name,tcp_fd,m,file_rm);
		}
		
		// 上传文件
		else if(file_choice == 3)
		{
			m.msg_file = CHOICE_C;
			write(tcp_fd, &m, sizeof(m));
			file_upload_exist(tcp_fd,m);
			
		}
		
		// 下载文件
		else if(file_choice == 4)
		{
			m.msg_file = CHOICE_D;
			write(tcp_fd, &m, sizeof(m));
			file_cat(file_name,tcp_fd,m,file_download);
		}
		
		// 退出
		else if(file_choice == 0)
		{
			m.msg_file = CHOICE_O;
			write(tcp_fd, &m, sizeof(m));
			break;
		}
		else
			printf("从新输入\n");
	}
}

//共享文件夹的操作
void public_folder(int tcp_fd,struct msg m)
{
	char file_name[1024]={0};
	m.msg_type = MSG_FILE;
	write(tcp_fd, &m, sizeof(m));	
	
	
	//传输文件名
	while(1)
	{
		read(tcp_fd,file_name,sizeof(file_name));
		if(strcmp("exit",file_name))
			printf("file_name:%s\n",file_name);
		else
		{
			break;
		}
	}
	
	
	//对共享文件的操作
	while(1)
	{
		printf("1.查看,2.下载文件,0.退出\n");
		int file_choice=-1;
		scanf("%d",&file_choice);
		
		// 打开文件显示文件内容
		if(file_choice == 1)
		{
			//给服务端发送一个打开文件的信号
			m.msg_file = CHOICE_A;
			write(tcp_fd, &m, sizeof(m));
			file_cat(file_name,tcp_fd,m,file_read);
		}
		
		// 下载文件
		else if(file_choice == 2)
		{
			m.msg_file = CHOICE_B;
			write(tcp_fd, &m, sizeof(m));
			if(share_or_local(tcp_fd,m))
			{
				file_cat(file_name,tcp_fd,m,file_download);		
			}
			else
				file_cat(file_name,tcp_fd,m,my_file_download);

		}
		
		// 退出
		else if(file_choice == 0)
		{
			m.msg_file = CHOICE_O;
			write(tcp_fd, &m, sizeof(m));
			break;
		}
		else
			printf("从新输入\n");
	}
}

//管理共享文件夹的操作
void manage_share(int tcp_fd,struct msg m)
{
	char file_name[1024]={0};
	m.msg_type = MSG_FILE;
	write(tcp_fd, &m, sizeof(m));	
	
	
	//传输文件名
	while(1)
	{
		read(tcp_fd,file_name,sizeof(file_name));
		if(strcmp("exit",file_name))
			printf("file_name:%s\n",file_name);
		else
		{
			break;
		}
	}
	
	
	
	//对共享文件的操作
	while(1)
	{
		printf("1.查看,2.下载文件,3.上传文件,4.删除文件,0.退出\n");
		int file_choice=-1;
		scanf("%d",&file_choice);
		
		// 打开文件显示文件内容
		if(file_choice == 1)
		{
			//给服务端发送一个打开文件的信号
			m.msg_file = CHOICE_A;
			write(tcp_fd, &m, sizeof(m));
			file_cat(file_name,tcp_fd,m,file_read);
		}
		
		// 下载文件
		else if(file_choice == 2)
		{
			m.msg_file = CHOICE_B;
			write(tcp_fd, &m, sizeof(m));
			file_cat(file_name,tcp_fd,m,file_download);

		}
		
		// 上传文件
		else if(file_choice == 3)
		{
			//给服务端发送一个打开文件的信号
			m.msg_file = CHOICE_C;
			write(tcp_fd, &m, sizeof(m));
			file_upload_exist(tcp_fd,m);
		}
		
		// 删除文件
		else if(file_choice == 4)
		{
			//给服务端发送一个打开文件的信号
			m.msg_file = CHOICE_D;
			write(tcp_fd, &m, sizeof(m));
			file_cat(file_name,tcp_fd,m,file_rm);
		}
		// 退出
		else if(file_choice == 0)
		{
			m.msg_file = CHOICE_O;
			write(tcp_fd, &m, sizeof(m));
			break;
		}
		else
			printf("重新输入\n");
	}
}

//用户登录成功后界面
void login_success(int tcp_fd,struct msg m)
{
	printf("登录成功\n");
	while(1)
	{
		int dir_choice=-1;
		printf("1.个人文件夹,2.公共文件夹,0.退出\n");
		scanf("%d",&dir_choice);
		
		// 个人文件夹
		if(dir_choice == 1)
		{
			my_folder(tcp_fd,m);
		}
		
		// 公共文件夹
		else if(dir_choice == 2)
		{
			public_folder(tcp_fd,m);
		}
		
		// 退出
		else if(dir_choice == 0)
		{
			m.msg_type = TUI_CHU;
			write(tcp_fd, &m, sizeof(m));
			break;
		}
		else
			printf("重新输入\n");
	}
}


//管理员登录成功后界面
void manage_login_success(int tcp_fd,struct msg m)
{
	printf("欢迎！\n");
	while(1)
	{
		int choice=-1;
		printf("1.用户账号管理,2.公共文件夹管理,0.退出\n");
		scanf("%d",&choice);
		
		// 用户账号管理
		if(choice == 1)
		{
			m.msg_type = MESSAGE;
			write(tcp_fd, &m, sizeof(m));
			user_manage(tcp_fd,m);
		}
		
		// 公共文件夹管理
		else if(choice == 2)
		{
			manage_share(tcp_fd,m);
		}
		
		// 退出
		else if(choice == 0)
		{
			m.msg_type = TUI_CHU;
			write(tcp_fd, &m, sizeof(m));
			break;
		}
		else
			printf("重新输入\n");
	}
}



//从服务端获取登录或注册消息
void read_msg(int tcp_fd)
{
		struct msg m;	
		read(tcp_fd, &m, sizeof(m));
		
		//用户注册成功
		if(m.s == A)
		{
			printf("用户注册成功\n");
		}
		
		//用户存在
		else if(m.s == B)
		{
			printf("用户存在\n");
		}
		
		//登录成功并进入操作界面
		else if(m.s == C)
		{
			login_success(tcp_fd,m);
		}

		//密码错误
		else if(m.s == D)
		{
			printf("密码错误\n");
		}
		
		// 用户不存在
		else if(m.s == E)
		{
			printf("账号不存在\n");
		}
		else if(m.s == F)
		{
			manage_login_success(tcp_fd,m);
		}
		else
			printf("重新输入\n");
}







int main(int argc, char *argv[])
{
	// 初始化客户端tcp套接字并连接到服务器，并且把自己的信息也发送过去
	int tcp_fd = tcp_init();
	// struct msg m;
	
	
	//用户端界面显示
	while(1)
	{
		int i = -1;
		printf("1*******注册\n2*****用户登录\n3****管理员登录\n0*******退出\n");
		scanf("%d",&i);
		
		
		//注册基本信息
		if(i == 1)
		{
			struct user u;
			get_info(&u);
			write(tcp_fd, &u, sizeof(u));
			read_msg(tcp_fd);
		}		
		//登录账号
		else if(i == 2)
		{
			struct user u;
			get_login(&u);
			write(tcp_fd, &u, sizeof(u));
			read_msg(tcp_fd);
		}
		
		//管理员登录
		else if(i == 3)
		{
			struct user u;
			get_manage_login(&u);
			write(tcp_fd, &u, sizeof(u));
			read_msg(tcp_fd);
		}
		
		//退出
		else if(i == 0)
		{
			break;
		}
		else
			printf("请重新输入\n");
		printf("???\n");
	}
	
	return 0;
}





