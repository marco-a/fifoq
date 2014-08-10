/*!
 *	@file			fifoq.h
 *	@copyright		< Pro Fusion , 2014 >
 *	@date			10.08.14
 *	@discussion
 *	Simple FIFO queue.
 *
 *	Copyright 2014 ProFusion. All rights reserved.
 */

#if !defined(_fifoq_h)
	#define _fifoq_h 1

	#include <stdlib.h>		// size_t
	#include <stdbool.h>	// bool

	// Public definition
	typedef struct _fifoq		*fifoq;

	/*!
	 *	@enum	fifoq_opts
	 *	@brief
	 *	Options for fifoq_init
	 */
	enum fifoq_opts {
		/*!
		 *	@field	fifoq_threadsafe
		 *	@brief
		 *	Make this FIFO queue threadsafe.
		 */
		fifoq_threadsafe	= 0x01,
		
		/*!
		 *	@field	fifoq_linkdata
		 *	@brief
		 *	Do NOT copy data.
		 */
		fifoq_linkdata		= 0x02
	};

	/*!
	 *	@function	fifoq_init
	 *	@brief
	 *	Initialize a new FIFO queue.
	 *
	 *	@param		dsize
	 *	Size of data hold in nodes.
	 *	@param		opts
	 *	Options for this queue.
	 *
	 *	@return
	 *	Initialized FIFO queue or NULL on failure.
	 */
	fifoq _fifoq_init(const size_t dsize, const unsigned char opts);

	#define fifoq_init(_type, _args...)	_fifoq_init(sizeof(_type), (0, ##_args))

	/*!
	 *	@function	fifoq_push
	 *	@brief
	 *	Push a lvalue onto queue.
	 *
	 *	@function	fifoq_pushrval
	 *	@brief
	 *	Push a rvalue onto queue.
	 *
	 *	@param		q
	 *	FIFO queue object.
	 *	@param		data
	 *	Pointer to data object.
	 *
	 *	@return		bool
	 *	`true` on success, otherwise `false`.
	 */
	bool fifoq_push(const restrict fifoq q, const void *const restrict data);
	#define fifoq_pushrval(_q, _data...)						\
		({														\
			typeof(_data) _d	= (typeof(_data))(_data);		\
			bool _ret			= fifoq_push(_q, &_d);			\
		_ret;})


	/*!
	 *	@function	fifoq_count
	 *	@brief
	 *	Count number of nodes in queue.
	 *
	 *	@param		q
	 *	FIFO queue object.
	 *
	 *	@return		size_t
	 *	Number of nodes in `q`.
	 */
	size_t fifoq_count(const fifoq q);

	/*!
	 *	@function	fifoq_pop
	 *	@brief
	 *	Pop a node off queue.
	 *
	 *	@param		q
	 *	FIFO queue object.
	 *
	 *	@return		bool
	 *	`true` on success, otherwise `false`.
	 */
	bool fifoq_pop(const fifoq c);

	/*!
	 *	@function	fifoq_empty
	 *	@brief
	 *	Check if queue is empty.
	 *
	 *	@param		q
	 *	FIFO queue object.
	 *
	 *	@return		bool
	 *	`true` if `q` is empty else `false`.
	 */
	bool fifoq_empty(const fifoq q);

	/*!
	 *	@function	fifoq_front
	 *	@brief
	 *	Get front node of queue.
	 *
	 *	@param		q
	 *	FIFO queue object.
	 *
	 *	@param		data
	 *	Pointer to data to store data in.
	 *
	 *	@return		bool
	 *	`true` on success, otherwise `false`.
	 */
	bool fifoq_front(const restrict fifoq q, void *const restrict data);

	/*!
	 *	@function	fifoq_rear
	 *	@brief
	 *	Get rear node of queue.
	 *
	 *	@param		q
	 *	FIFO queue object.
	 *
	 *	@param		data
	 *	Pointer to data to store data in.
	 *
	 *	@return		bool
	 *	`true` on success, otherwise `false`.
	 */
	bool fifoq_rear(const restrict fifoq q, void *const restrict data);

	/*!
	 *	@function	fifoq_destroy
	 *	@brief
	 *	Delete FIFO a queue.
	 *
	 *	@param		q
	 *	FIFO queue object.
	 */
	void fifoq_destroy(const fifoq q);

	/*!
	 *	@function	fifoq_test
	 *	@brief
	 *	Test queue.
	 *
	 *	@return		bool
	 *	`true` on success, otherwise `false`.
	 */
	bool fifoq_test(void);

#endif // !defined(_fifoq_h)
