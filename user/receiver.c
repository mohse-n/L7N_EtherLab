
#include <stdio.h>
/* For IPC_CREAT */
#include <sys/msg.h>
/* For int32_t */
#include <stdint.h>
/* Header for handling signals (definition of SIGINT) */
#include <signal.h>
/* For pid_t and getpid() */
#include <unistd.h>
#include <sys/types.h>

#define MEASURE_PERF

/* Queue ID */
int qID;

void signal_handler(int sig)
{
	printf("Removing the queue...\n");
	if (!(msgctl(qID, IPC_RMID, NULL)))
	    printf("The queue was successfully removed.\n");
	pid_t pid = getpid();
	kill(pid, SIGKILL);
}

int main(int argc, char **argv)
{

	signal(SIGINT, signal_handler);
	
	/* key is specified by the process which creates the queue (receiver). */
	key_t qKey = 1234;
	
	/* IPC_CREAT: Create a new queue.*/
	int qFlag = IPC_CREAT;
	
        printf("Creating a queue with key = %d\n", qKey);

	if ((qID = msgget(qKey, qFlag)) < 0) 
	{
		printf("Queue creation failed. Terminating the process...\n");
		return -1;
	}
	else 
		printf("Queue creation successful.\n");
	
	
	typedef struct myMsgType 
	{
		/* Mandatory */
		long       mtype;
		/* Data */
		#ifdef MEASURE_PERF
		long       updatePeriod;
		#endif
		int32_t    actPos[2];
		int32_t    targetPos[2];
       	} myMsg;
	
	/* Received message. */
	myMsg recvdMsg;
	
	size_t msgSize;
	
	/* size of data = size of structure - size of mtype */
	msgSize = sizeof(struct myMsgType) - sizeof(long);
	
	/* Pick messages with type = 1. (msg.mtype = 1 in the producer) */
	int msgType = 1;
	
	/* No flag for receiving the message. */
	int msgFlag = 0;
	
	while (1)
	{
	
		/* Removes a message from the queue specified by qID and 
		   places it in the buffer pointed to by recvdMsg. 
		*/
		if (msgrcv(qID, &recvdMsg, msgSize, msgType, msgFlag) < 0) 
		{
			printf("Error picking a message from the queue. Terminating the process...\n");
			return -1;
		}
		
	        /* Print the message's data. */
		printf("Motor 0 actPos: %d\n", recvdMsg.actPos[0]);
		printf("Motor 1 actPos: %d\n", recvdMsg.actPos[1]);
		#ifdef MEASURE_PERF
		printf("Update period: %ld\n", recvdMsg.updatePeriod);
		#endif
	
	}
	
	
}
