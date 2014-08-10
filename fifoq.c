/*!
 *	@file			fifoq.c
 *	@copyright		< Pro Fusion , 2014 >
 *	@date			10.08.14
 *
 *	Copyright 2014 ProFusion. All rights reserved.
 */

#include "fifoq.h"		// fifoq
#include <string.h>		// memcpy
#include <pthread.h>	// pthread_*
#include <stdarg.h>		// va_list

struct _fifoqn_ts_impl {
	pthread_mutex_t		mtx;
	pthread_cond_t		cnd;
};

struct _fifoqn {
	struct _fifoqn			*restrict next;
	void					*restrict data;
};

struct _fifoq {
	unsigned char			opts;
	size_t					dsize;
	struct _fifoqn			*front;
	struct _fifoqn			*rear;
	size_t					num_nodes;
	
	struct _fifoqn_ts_impl	*restrict ts_impl;
};

/*!
 *	Create new FIFO queue.
 */
fifoq _fifoq_init(const size_t dsize, const unsigned char opts)
{
	if (dsize == 0 || opts > 3) return NULL;
	
	{
		bool q_threadsafe	= opts & 1;
		struct _fifoq *q	= malloc(sizeof(* q) + (sizeof(struct _fifoqn_ts_impl) * q_threadsafe));
		
		if (q == NULL) return NULL;
		
		if (q_threadsafe) {
			q->ts_impl = (void *)((unsigned char *)q) + sizeof(struct _fifoq);
			
			if (pthread_mutex_init(&q->ts_impl->mtx, NULL) != 0) {
				free(q);
				
				return NULL;
			} else if (pthread_cond_init(&q->ts_impl->cnd, NULL) != 0) {
				(void)pthread_mutex_destroy(&q->ts_impl->mtx);
				free(q);
				
				return NULL;
			}
		}
		
		q->opts			= opts;
		q->dsize		= dsize;
		q->front		= q->rear = NULL;
		q->num_nodes	= 0;
		
		return q;
	}
}

/*!
 *	Push `data` onto `q`.
 */
bool fifoq_push(const restrict fifoq q, const void *const restrict data)
{
	/* Can't push NULL values onto queue */
	if (q == NULL || data == NULL) return false;
	
	{
		bool q_linkdata			= q->opts & fifoq_linkdata;
		size_t q_dsize			= q->dsize;
		struct _fifoqn *node	= malloc(sizeof(* node) + ((q_dsize * !q_linkdata)));
		bool q_threadsafe		= q->opts & fifoq_threadsafe;
		
		if (node == NULL) return false;
		
		if (q_linkdata) {
			memcpy((void *)&node->data, data, sizeof(data));
		} else {
			node->data = (void *)(((unsigned char *)node) + sizeof(struct _fifoqn));
			
			memcpy(node->data, data, q_dsize);
		}
		
		/* If we are threadsafe, lock mutex now */
		if (q_threadsafe) {
			/* Aquire mutex */
			if (pthread_mutex_lock(&q->ts_impl->mtx) != 0) {
				free(node);
				
				return false;
			}
		}
		
		/* The next node is to determine */
		node->next = NULL;
		
		if (q->front == NULL) {
			q->rear = q->front = node;
		} else {
			q->rear->next = node;
			q->rear = node;
		}
		
		/* Increment number of nodes in queue */
		++q->num_nodes;
		
		/* If we are threadsafe, signal condition and unlock mutex */
		if (q_threadsafe) {
			/* Signal to other threads */
			(void)pthread_cond_signal(&q->ts_impl->cnd);
			
			/* Unlock mutex, can't fail */
			(void)pthread_mutex_unlock(&q->ts_impl->mtx);
		}
		
		return true;
	}
}

/*!
 *	Get number of nodes in `q`.
 */
size_t fifoq_count(const fifoq q)
{
	if (q == NULL) return 0;
	
	{
		bool q_threadsafe	= q->opts & fifoq_threadsafe;
		size_t q_num_nodes;
		
		/* If we are threadsafe, lock mutex now */
		if (q_threadsafe) {
			/* Aquire mutex */
			if (pthread_mutex_lock(&q->ts_impl->mtx) != 0) {
				return 0;
			}
		}
		
		q_num_nodes = q->num_nodes;
		
		/* If we are threadsafe, unlock mutex */
		if (q_threadsafe) {
			/* Unlock mutex, can't fail */
			(void)pthread_mutex_unlock(&q->ts_impl->mtx);
		}
		
		return q_num_nodes;
	}
}

/*!
 *	Pop node off `q`.
 */
static struct _fifoqn *_fifoq_pop(const fifoq q, const bool popoff, ...)
{
	if (q == NULL) return NULL;
	
	{
		bool q_threadsafe	= q->opts & fifoq_threadsafe;
		struct _fifoqn *front;
		
		/* If we are threadsafe, lock mutex now */
		if (q_threadsafe) {
			/* Aquire mutex */
			if (pthread_mutex_lock(&q->ts_impl->mtx) != 0) {
				return NULL;
			}
			
			/* Wait on a certain condition */
			while (q->front == NULL) {
				(void)pthread_cond_wait(&q->ts_impl->cnd, &q->ts_impl->mtx);
			}
		} else {
			/* Queue is empty */
			if (q->front == NULL) {
				return NULL;
			}
		}
		
		// Move forward
		front = q->front;
		
		if (popoff) {
			q->front = front->next;
			--q->num_nodes;
			
			if (q->front == NULL) q->rear = NULL;
			
			free(front);
		} else {
			va_list ap;
			void *data;
			
			va_start(ap, popoff);
			
			data = va_arg(ap, void *);
			
			va_end(ap);
			
			/* For reasons inside mutex... */
			if (q->opts & fifoq_linkdata) {
				memcpy(data, (void *)&front->data, sizeof(data));
			} else {
				memcpy(data, front->data, q->dsize);
			}
		}
		
		/* If we are threadsafe, unlock mutex */
		if (q_threadsafe) {
			/* Unlock mutex, can't fail */
			(void)pthread_mutex_unlock(&q->ts_impl->mtx);
		}
			
		return front;
	}
}

bool fifoq_pop(const fifoq q)
{
	return (_fifoq_pop(q, true) != NULL);
}

/*!
 *	Check if `q` is empty.
 */
bool fifoq_empty(const fifoq q)
{
	return (fifoq_count(q) == 0);
}

/*!
 *	Get front of `q`.
 */
bool fifoq_front(const restrict fifoq q, void *const restrict data)
{
	if (data == NULL) return false;
	
	{
		return (_fifoq_pop(q, false, data) != NULL);
	}
}

/*!
 *	Get back of `q`.
 */
bool fifoq_rear(const restrict fifoq q, void *const restrict data)
{
	if (q == NULL || data == NULL) return false;
	
	{
		bool q_threadsafe	= q->opts & fifoq_threadsafe;
		bool ret			= false;
		
		/* If we are threadsafe, lock mutex now */
		if (q_threadsafe) {
			/* Aquire mutex */
			if (pthread_mutex_lock(&q->ts_impl->mtx) != 0) {
				return false;
			}
		}
		
		if (q->rear != NULL) {
			/* For reasons inside mutex... */
			if (q->opts & fifoq_linkdata) {
				memcpy(data, (void *)&q->rear->data, sizeof(data));
			} else {
				memcpy(data, q->rear->data, q->dsize);
			}
			
			ret = true;
		}
		
		/* If we are threadsafe, unlock mutex */
		if (q_threadsafe) {
			/* Unlock mutex, can't fail */
			(void)pthread_mutex_unlock(&q->ts_impl->mtx);
		}
		
		return ret;
	}
}

/*!
 *	Destroy `q`.
 */
void fifoq_destroy(const restrict fifoq q)
{
	if (q == NULL) return;
	
	{
		/* Iterate through nodes */
		struct _fifoqn *next = q->front;
		
		while (next != NULL) {
			struct _fifoqn *node = next;
			
			next = node->next;
			
			free(node);
		}
		
		if (q->opts & fifoq_threadsafe) {
			(void)pthread_mutex_destroy(&q->ts_impl->mtx);
			(void)pthread_cond_destroy(&q->ts_impl->cnd);
		}
		
		q->ts_impl		= NULL;
		q->front		= q->rear = NULL;
		q->num_nodes	= q->opts = q->dsize = 0;
		
		free(q);
	}
}

/*!
 *	Test.
 */
struct _fifoqt {
	char		*name;
	unsigned	age;
};

bool fifoq_test(void)
{
	fifoq q	= fifoq_init(struct _fifoqt);
	struct _fifoqt val = {"hans", 30};
	
	if (fifoq_pushrval(q, (struct _fifoqt){"max", 100}) == false) goto fail;
	
	if (fifoq_pushrval(q, (struct _fifoqt){"mustermann", 50}) == false) goto fail;
	
	if (fifoq_push(q, &val) == false) goto fail;
	
	struct _fifoqt front, rear, pop;
	
	if (fifoq_front(q, &front) == false) goto fail;
	
	if (fifoq_rear(q, &rear) == false) goto fail;
	
	if (fifoq_pop(q) == false) goto fail;
	
	if (fifoq_front(q, &pop) == false) goto fail;

	if (fifoq_pop(q) == false) goto fail;
	
	if (strcmp("max", front.name) != 0 || front.age != 100) goto fail;
	if (strcmp("hans", rear.name) != 0 || rear.age != 30) goto fail;
	if (strcmp("mustermann", pop.name) != 0 || pop.age != 50) goto fail;
	
	fifoq_front(q, &pop);
	
	if (strcmp("hans", pop.name) != 0 || pop.age != 30) goto fail;

	if (fifoq_count(q) != 1)	goto fail;
	if (fifoq_pop(q) == false)	goto fail;
	if (fifoq_pop(q) == true)	goto fail;
	
	return true;
	
	fail:
		fifoq_destroy(q);
		return false;
}
