#include "fifoq.h"
#include <stdio.h>

int main(int argc, const char **argv)
{
	/* Create a new threadsafe FIFO queue */
	fifoq int_queue		= fifoq_init(int, fifoq_threadsafe);
	/* Create a new FIFO queue where data will not be copied */
	fifoq intp_queue	= fifoq_init(int *, fifoq_linkdata);

	int intval				= 1337;
	int node				= 0;
	/* Grab some memory to put in queue */
	int *intpval			= malloc(sizeof(* intpval));
	int *pnode				= NULL;
	
	/* Push 1337 onto queue */
	fifoq_push(int_queue, &intval);
	/* Push 1330 onto queue without var */
	fifoq_pushrval(int_queue, 1330);
	/* Push 1880 onto queue without var */
	fifoq_pushrval(int_queue, 1880);
	
	/* Push pointer onto queue */
	fifoq_push(intp_queue, &intpval);
	
	/* Set value afterwards to 1880 */
	*intpval = 1880;
	
	/*
		int_queue    intp_queue
		FRONT +------+     +-------+
		      | 1337 |     | pnode |
		      +------+     +-------+
			  | 1330 |
			  +------+
		      | 1880 |
		REAR  +------+
		
		POP     1337         pnode
		POP     1330         FALSE (nothing to pop off)
	*/
	
	/* Get rear element */
	fifoq_rear(int_queue, &node);
	fprintf(stderr, "int_queue(%zu)  2: %d\n", fifoq_count(int_queue), node);
	/* Get first element */
	fifoq_front(int_queue, &node);
	fprintf(stderr, "int_queue(%zu)  0: %d\n", fifoq_count(int_queue), node);
	fifoq_pop(int_queue);
	/* Get second element */
	fifoq_front(int_queue, &node);
	fprintf(stderr, "int_queue(%zu)  1: %d\n", fifoq_count(int_queue), node);
	fifoq_pop(int_queue);
	
	/* Get first element */
	fifoq_front(intp_queue, &pnode);
	fprintf(stderr, "intp_queue(%zu) 0: %d\n", fifoq_count(intp_queue), *pnode);
	fifoq_pop(intp_queue);
	
	/* Destroy queues */
	fifoq_destroy(int_queue);
	fifoq_destroy(intp_queue);
	
	/* Free memory manually */
	free(pnode);
	
	if (fifoq_test() == false) return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
