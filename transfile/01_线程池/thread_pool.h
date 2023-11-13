#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <errno.h>
#include <pthread.h>

#define MAX_WAITING_TASKS	1000
#define MAX_ACTIVE_THREADS	20

struct task
{
	void *(*task)(void *arg);	//函数指针，指向一个函数
	void *arg;					//上面函数指针指向函数所需的参数

	struct task *next;		//链表下一个节点地址
};

typedef struct thread_pool
{
	pthread_mutex_t lock;	//互斥锁，保护任务链表
	pthread_cond_t  cond;	//条件变量，判断任务链表是否有任务可取
	struct task *task_list;	//任务链表头节点

	pthread_t *tids;		//顺序表，存储活跃线程的tid

	unsigned waiting_tasks;	//等待的任务数量
	unsigned active_threads;	//活跃线程的数量

	bool shutdown;		//线程池的开关
}thread_pool;	//struct thread_pool 的类型别名


bool
init_pool(thread_pool *pool,
          unsigned int threads_number);

bool
add_task(thread_pool *pool,
         void *(*task)(void *arg),
         void *arg);

int 
add_thread(thread_pool *pool,
           unsigned int additional_threads_number);

int 
remove_thread(thread_pool *pool, 
			unsigned int removing_threads);		   

bool destroy_pool(thread_pool *pool);
void *routine(void *arg);

#endif
