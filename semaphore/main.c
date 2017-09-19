/*  main.c  - main */

#include <xinu.h>

pid32 producer_id;
pid32 consumer_id;
pid32 timer_id;

int32 consumed_count = 0;
const int32 CONSUMED_MAX = 100;

/* Define your circular buffer structure and semaphore variables here */
sid32 produce, consume, mutex_lock;			//semaphore variables

int32 cirqueue[100];		//creating circular buffer of size 100
int32 head=0, tail=-1;


/* Place your code for entering a critical section here */
void mutex_acquire(sid32 mutex)
{
	/*Implement wait function call */
	wait(mutex);
}

/* Place your code for leaving a critical section here */
void mutex_release(sid32 mutex)
{
signal (mutex);
}

/* Place the code for the buffer producer here */
process producer(void)
{
	int32 i;
	//adds a new node to the tail of the array
	
	for(i=0;i<2000;i++){
			//calls function mutex_acquire to check for the acquire 
			mutex_acquire(consume);
			mutex_acquire(mutex_lock);
			if((head==0 && tail==-1)||(tail!=head-1)){
				cirqueue[++tail]=i;		// adding element to a circular queue cirqueue
				kprintf("Prduced value = %d\n",cirqueue[tail]);
			}
			
			else if(tail==CONSUMED_MAX-1 && head>0){
				tail=0;
				cirqueue[tail]=i;
				kprintf("Prduced value = %d\n",cirqueue[tail]);
			}
			mutex_release(mutex_lock);
			mutex_release(produce);//calls realease to realease the acquired lock
	
	}
	return OK;
}

/* Place the code for the buffer consumer here */
process consumer(void)
{
	
	int32 i;
	/* Every time your consumer consumes another buffer element,
	 * make sure to include the statement:
	 *   consumed_count += 1;
	 * this will allow the timing function to record performance */
	/* */
	
	for(i=0;i<=2000;i++){
		mutex_acquire(produce);
		mutex_acquire(mutex_lock);
		if(head==tail){
			kprintf("Consumed Value = %d\n",cirqueue[head]);
			consumed_count+=1;
			head=0;
			tail=-1;
		}
		else if(head==CONSUMED_MAX-1){
		kprintf("Consumed Value= %d\n",cirqueue[head]);
		consumed_count+=1;
		}
		else{ 
		kprintf("Consumed Value= %d \n",cirqueue[head++]);
		consumed_count+=1;
		}
		mutex_release(mutex_lock);
		mutex_release(consume);
	}
	

	return OK;
}


/* Timing utility function - please ignore */
void time_and_end(void)
{
	int32 times[5];
	int32 i;

	for (i = 0; i < 5; ++i)
	{
		times[i] = clktime_ms;
		yield();
		while (consumed_count < CONSUMED_MAX * (i+1)){
			yield();
		}
		times[i] = clktime_ms - times[i];
		
		consumed_count = 0;
	}

	kill(producer_id);
	kill(consumer_id);

	for (i = 0; i < 5; ++i)
	{
		kprintf("TIME ELAPSED (%i): %i\n", (i+1) * CONSUMED_MAX, times[i]);
	}
}

process	main(void)
{
	recvclr();
	produce=semcreate(0);			//initial access to cs
	consume=semcreate(99);			//initially waits for the produce to signal access
	mutex_lock=semcreate(1);


	/* Create the shared circular buffer and semaphores here */
	/* */

	producer_id = create(producer, 4096, 50, "producer", 0);
	consumer_id = create(consumer, 4096, 50, "consumer", 0);
	timer_id = create(time_and_end, 4096, 50, "timer", 0);
	resched_cntl(DEFER_START);
	resume(producer_id);
	resume(consumer_id);
	/* Uncomment the following line for part 3 to see timing results */
	resume(timer_id);
	resched_cntl(DEFER_STOP);

	return OK;
}
