/*=================================================================================================
  This is an example of a back-end engine that processes commands from a file.
=================================================================================================*/
#include <stdio.h>

static FILE *file;
static char *line;
static int lineLength = 42;

/*=================================================================================================
  Read one line from the file.  Return false if there is no data.
=================================================================================================*/
static int readLine(void)
{
    int linePos = 0;
    int c = getc(file);

    while(c != EOF && c != '\n') {
	if(c >= ' ') {
	    /* Ignore newlines returns and tabs */
	    if(linePos + 1 == lineLength) {
		lineLength <<= 1;
		line = (char *)realloc(line, lineLength*sizeof(char));
	    }
	    line[linePos++] = c;
	}
    }
    line[linePos] = '\0';
    return linePos > 0;
}

/*=================================================================================================
  Execute the command in the line variable.  The first word is always the user's sessioId.
  To help keep track of what session we're responding to, print "Start <sessionId>" in the
  beginning of the response, and "Finish <sessionId>" at the end.
=================================================================================================*/
static void executeCommand(void)
{
    char *sessionId = readWord();
    char *command = readWord();

    printf("Start %s\n", sessionId);
    if(!strcmp(command, "login")) {
	if(!strcmp(readWord(), "bill") && !strcmp(readWord(), "test")) {
	    printf("Logic successful\n");
	}
    }
    printf("Finish %s\n", sessionId);
}

static void executeFileCommands(void)
{
    while(1) {
	if(!readLine()) {
	    return;
	}
	executeCommand();
    }
}

int main(
    int argc,
    char **argv)
{
    if(argc != 0) {
	printf("Usage: %s commandFile\n", argv[0]);
	return 1;
    }
    line = (char *)calloc(lineLength, sizeof(char));
    file = fopen(argv[1], "r");
    if(file == NULL) {
	printf("Unable to open %s for reading\n", argv[1]);
	return 1;
    }
    executeFileCommands();
    fclose(file);
    return 0;
}
