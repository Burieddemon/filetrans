#include "thread_pool.h"

void handler(void *arg)
{
	pthread_mutex_unlock((pthread_mutex_t *)arg);
}


//设计一个通用的线程函数
void *routine(void *arg)
{
	thread_pool *pool = (thread_pool *)arg;	//定义线程池结构体指针，拿到线程池的指针
	struct task *p;	//定义任务结构体指针，用来表示拿到的任务

	//不断的拿任务，执行
	while(1)
	{

		pthread_cleanup_push(handler, (void *)&pool->lock);	//防止死锁
		pthread_mutex_lock(&pool->lock);	//加锁

		//如果任务队列为空并且线程池没有被销毁，线程进入条件变量等待队列
		while(pool->waiting_tasks == 0 && !pool->shutdown)
		{
			pthread_cond_wait(&pool->cond, &pool->lock);	//条件变量等待队列
		}

		//如果任务为空，线程池被销毁，但是被换醒了，直接解锁退出线程
		if(pool->waiting_tasks == 0 && pool->shutdown)
		{
			pthread_mutex_unlock(&pool->lock);	//解锁
			pthread_exit(NULL);
		}

		//取走一个任务
		p = pool->task_list->next;	//把链表头pool->task_list的下一个节点地址给到p
		pool->task_list->next = p->next;
		pool->waiting_tasks--;	//任务数量减1

		//任务取走，解锁
		pthread_mutex_unlock(&pool->lock);
		pthread_cleanup_pop(0);

		//执行任务期间拒绝取消请求
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		printf("[%lu]-->", pthread_self());
		(p->task)(p->arg);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		free(p);	//释放资源
	}

	pthread_exit(NULL);
}


//初始化线程池
// pool：指向线程池结构体的指针
// threads_number：准备初始化的活跃线程数量
bool init_pool(thread_pool *pool, unsigned int threads_number)
{
	pthread_mutex_init(&pool->lock, NULL);	//初始化互斥锁
	pthread_cond_init(&pool->cond, NULL);	//初始化条件变量

	pool->shutdown = false;	//初始化线程池销毁标记
	pool->task_list = malloc(sizeof(struct task));	//初始化任务链表
	pool->tids = malloc(sizeof(pthread_t) * MAX_ACTIVE_THREADS);	//初始化存放线程ID的数组
	// 检测malloc分配内存是否成功
	if(pool->task_list == NULL || pool->tids == NULL)
	{
		perror("allocate memory error");
		return false;
	}

	pool->task_list->next = NULL;	//链表的下一个节点为空

	pool->waiting_tasks = 0;	//初始化任务队列中等待的任务个数
	pool->active_threads = threads_number;	//初始化活跃线程个数

	int i;
	for(i=0; i<pool->active_threads; i++)
	{
		if( pthread_create(&((pool->tids)[i]), NULL,routine, (void *)pool) != 0)
		{
			perror("create threads error");
			return false;
		}
	}

	return true;
}

bool add_task(thread_pool *pool,
			void *(*task)(void *arg), void *arg)
{
	struct task *new_task = malloc(sizeof(struct task));
	if(new_task == NULL)
	{
		perror("allocate memory error");
		return false;
	}
	//初始化任务节点
	new_task->task = task;
	new_task->arg = arg;
	new_task->next = NULL;

	//加锁
	//超过最大任务
	pthread_mutex_lock(&pool->lock);
	if(pool->waiting_tasks >= MAX_WAITING_TASKS)
	{
		pthread_mutex_unlock(&pool->lock);

		fprintf(stderr, "too many tasks.\n");
		free(new_task);

		return false;
	}
	
	struct task *tmp = pool->task_list;
	while(tmp->next != NULL)	//找到任务链表的末尾节点
		tmp = tmp->next;

	tmp->next = new_task;	//把新任务添加到链表最后一个节点
	pool->waiting_tasks++;	//等待任务数量加1


	pthread_mutex_unlock(&pool->lock);
	pthread_cond_signal(&pool->cond);

	return true;
}

int add_thread(thread_pool *pool, unsigned additional_threads)
{
	if(additional_threads == 0)
		return 0;

	unsigned total_threads =
		     pool->active_threads + additional_threads;
	
	// if(total_threads > MAX_ACTIVE_THREADS)
		// return additional_threads - (total_threads - MAX_ACTIVE_THREADS);

	int i, actual_increment = 0;	//成功创建线程的个数
	for(i = pool->active_threads;
	    i < total_threads && i < MAX_ACTIVE_THREADS;
	    i++)
	{
		if(pthread_create(&((pool->tids)[i]),
				NULL, routine, (void *)pool) != 0)
		{
			perror("add threads error");

			if(actual_increment == 0)
				return -1;

			break;
		}
		actual_increment++; 
	}

	//更新线程池中的活跃线程个数
	pool->active_threads += actual_increment;
	return actual_increment;
}


int remove_thread(thread_pool *pool, unsigned int removing_threads)
{
	if(removing_threads == 0)
		return pool->active_threads;

	int remain_threads = pool->active_threads - removing_threads;
	remain_threads = remain_threads>0 ? remain_threads:1;

	int i;
	for(i=pool->active_threads-1; i>remain_threads-1; i--)
	{
		errno = pthread_cancel(pool->tids[i]);
		if(errno != 0)
			break;
	}

	if(i == pool->active_threads-1)
		return -1;
	else
	{
		pool->active_threads = i+1;
		return i+1;
	}
}


bool destroy_pool(thread_pool *pool)
{

	pool->shutdown = true;	//线程池的销毁标记
	pthread_cond_broadcast(&pool->cond);	//换醒所有线程

	int i;
	for(i=0; i<pool->active_threads; i++)	//等待所有线程退出
	{
		errno = pthread_join(pool->tids[i], NULL);
		if(errno != 0)
		{
			printf("join tids[%d] error: %s\n",
					i, strerror(errno));
		}
		else
			printf("[%u] is joined\n", (unsigned)pool->tids[i]);
		
	}

	//释放资源
	//清空任务队列
	//释放任务队列
	free(pool->task_list);
	free(pool->tids);
	free(pool);

	return true;
}
