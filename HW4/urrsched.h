#define MAX_CALL 100
#define MAX_RESP 10
#define MAX_ARG 25
#define MAX_QUEUES 100
#define MAX_NAME 25
#define MAX_RESP_QUEUE MAX_QUEUES*MAX_NAME
#define GENERR -1
#define URRSCHED_SCHED_UWRR_SUCCESS 0
#define URRSCHED_CALL "sched_uwrr"
#define MAX_URR_PS 1000
#define TENMS ( (10 * HZ ) / 1000)
//#define CALLERCYCLESL ((unsigned long long int) 100000000)
#define CALLERCYCLESL 99999999
char dir_name[] = "urrsched";
char file_name[] = "call";

