#include "dir.h"


//读出目录中的文件名
void dir_view(char *dir_name,int confd)
{
	// 1.打开目录文件
	DIR *dp = opendir(dir_name);
	if ( dp == NULL )
	{
		perror("opendir fail");
		exit(errno);
	}
	
	// 2. 读取目录项
	struct dirent *ep = NULL;
	while(1)
	{
		ep = readdir(dp);
		if ( ep == NULL && errno == 0 )	//成功读完所有的目录项
		{
			break;
		}		
		else if( ep == NULL )	//进入到这个判断，说明errno值已被改变，出现某种错误
		{
			perror("readdir fail");
			break;
		}
		if ( ep->d_name[0] == '.' )
			continue;
		printf("%s\t",ep->d_name);
		fflush(stdout);
		write(confd,ep->d_name,sizeof(ep->d_name));
		usleep(10000);
	}
	write(confd,"exit",5);
	return;
}


//查看文件是否存在
bool  dir_exist(char *dir_file_name)
{
	if(access(dir_file_name,F_OK)==0)
	{
		return true;
	}
		return false;
}

//读取文件的内容并发送
void file_send(int confd , char *file_path)
{
	int fp = open(file_path, O_RDONLY);
	if(fp < 0)
	{
		perror("fopen fail");
	}
	char buf[1024] = {0};
	while(1)
	{
		if(read(fp,buf,1024) != 0)
		{
			// printf("buf: %s\n", buf);
			write(confd,buf,strlen(buf));
			bzero(buf,1024);
		}
		else
		break;
	}
	write(confd,"exit",5);
	close(fp);
}


//删除文件
void file_delete(int confd , char *file_path)
{
	char command[1024] = {0};
	sprintf(command,"rm %s",file_path);
	system(command);
	
	//判断是否删除成功
	if(dir_exist(file_path) == 0)	
	write(confd,"success",8);
    else
	write(confd,"exit",5);
}

// 文件接受
void file_accept(int confd,char *file_path)
{
	int fp = open(file_path, O_RDWR|O_CREAT,0777);
	if(fp < 0)
	{
		perror("fopen fail");
	}
	char buf[1024] = {0};
	while(1)
	{
		read(confd, buf, sizeof(buf));
		if(strstr(buf,"exit") == NULL)
		{
			printf("%s\n",buf);
			write(fp,buf,strlen(buf));
			bzero(buf,1024);
		}
		else
		break;
	}
	close(fp);
}














