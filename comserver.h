/*--------------------------------------------------------------------------------------------------
  This is a server-side module which creates a file socket for communication with multiple
  clients, typically Apache scripts responding to an HTTP request.  A simple printf/getc style
  interface is presented to the server which merges all client communication into one input and
  one output stream.  The point is to allow console apps to serve as the back-end engines to
  Apache without the hassle of managing network connections, multiple threads, or 'select' loops.
  Communication between server and client is done one command at a time.  Commands are '\0'
  terminated.  The first command from a client must be it's sessionId.  After that, any
  zero-terminated string is a valid message.

  The server reacts to client messages with:

      char *coStartResponse(void);
      int coPrintf(char *format, ...);
      coCompleteResponse(void);
      char coGetc(void):

  The point of these functions is to act like the functions used in a typical console app:
  printf and getchar.  At the top level of a command interpreter loop, you should call
  coStartResponse, which returns the sessionId of the next message from a client.  Then call your
  command processing code, which calls coGetc until it determines it's read enough input.  '\0'
  will be the actual termination of the messsage from the client, but you may only read to the
  first '\n' if that's how your console app works.  Instead of calling printf in response, call
  coPrintf.  When control returns back to the command processing loop, call coCompleteResponse to
  cause the response message to be sent.  Note that empty responses will result in an empty
  message, so clients should expect a response to every message.

  The server and initialize/closesthings with:

      int coStartServer(char *fileSocketPath);
      void coStopServer(void);

--------------------------------------------------------------------------------------------------*/

void coStartServer(char *fileSocketPath);
void coStopServer(void);
char *coStartResponse(void);
int coPrintf(char *format, ...);
void coCompleteResponse(void);
int coGetc(void):
