// creat set ringBuffer to empty
bool terminated = false;
char *prompt = NULL;
char *terminator = NULL;
int promptState = 0;
int matchPromptPos = 0;
int matchTerminatorPos = 0;
unsigned long start_time = 0;
unsigned long atTimeout = 300; //timeout period for receive
RingBuffer rxBuffer(300); // main receiving buffer for task to process on


