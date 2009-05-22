#define INPUT_MAXLEN 100000000

void enableDebug(char *logFile);
void print(char *format, ...);
void unencode(char *string);
char *readCookie(char *varName);
char *generateSessionId(void);
char *readInput(void);
char *readInputVar(char *input, char *varName);
char *readSessionId(void);
