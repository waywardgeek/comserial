/*=================================================================================================
  The client communicates with the server using:

      coSendMessage(char *format, ...);
      char *coReadMessage(void);

  Typically, a message will be just like a command that would normally be issued through a console
  interface to the server.  It's typically one line, and newline terminated.  The server will
  respond to each message sent, even if no response is expected.  In this case, the response
  message will be empty.

  The client does so with:

      int coStartClient(char *fileSocketPath, char *sessionId);
      void coStopClient(void);
=================================================================================================*/

int coStartClient(char *fileSocketPath, char *sessionId);
void coStopClient(void);
coSendMessage(char *format, ...);
char *coReadMessage(void);
