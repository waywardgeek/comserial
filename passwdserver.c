/*--------------------------------------------------------------------------------------------------
  This is an example of a back-end engine that processes commands from a file.  It has all kinds
  of buffer-overrun bugs, so use this as a way to understand backend servers, not as a template.
--------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comserver.h"

static int readFromFileSocket = 0;

struct UserStruct;
typedef struct UserStruct *User;

struct UserStruct {
    char sessionId[100];
    char loginName[100];
    char password[100];
    User next;
    int loggedIn;
};

static User firstUser;

/*--------------------------------------------------------------------------------------------------
  Read in the users and their passwords from the passord file.
--------------------------------------------------------------------------------------------------*/
static void readPasswdFile(
    char *passwdFileName)
{
    FILE *file = fopen(passwdFileName, "r");
    User user;
    char loginName[100], password[100];

    if(file == NULL) {
        coPrintf("Unable to open password file %s\n", passwdFileName);
        exit(1);
    }
    while(fscanf(file, "%s %s\n", loginName, password) == 2) {
        user = (User)calloc(1, sizeof(struct UserStruct));
        user->next = firstUser;
        firstUser = user;
        strcpy(user->loginName, loginName);
        strcpy(user->password, password);
        coPrintf("Added user %s with password=%s\n", loginName, password);
    }
    fclose(file);
}

/*--------------------------------------------------------------------------------------------------
  Just read in a line of text.
--------------------------------------------------------------------------------------------------*/
static int readLine(
    char *line)
{
    int c;

    while((c = coGetc()) != EOF && c != '\n') {
        *line++ = c;
    }
    return c != EOF;
}

/*--------------------------------------------------------------------------------------------------
  Execute the command in the line variable.  The first word is always the user's sessionId.
  To help keep track of what session we're responding to, print "Start <sessionId>" in the
  beginning of the response, and "Finish <sessionId>" at the end.
--------------------------------------------------------------------------------------------------*/
static int executeCommand(
    char *line,
    char *sessionId)
{
    User user;
    char loginName[100], password[100];

    if(sscanf(line, "login %s %s", loginName, password) == 2) {
        for(user = firstUser; user != NULL; user = user->next) {
            if(!strcmp(user->loginName, loginName) && !strcmp(user->password, password)) {
                if(user->loggedIn) {
                    if(strcmp(user->sessionId, sessionId)) {
                        coPrintf("This user is already on-line in a different session.\n");
                    } else {
                        coPrintf("Already logged in.\n");
                    }
                } else {
                    user->loggedIn = 1;
                    strcpy(user->sessionId, sessionId);
                    coPrintf("Login successful.  You are a master of the universe!\n");
                }
                return 1;
            }
            coPrintf("Invalid name/password combo.  If you do that again, I'll have to spank you.\n");
        }
    } else if(!strcmp(line, "logout")) {
        for(user = firstUser; user != NULL; user = user->next) {
            if(!strcmp(user->sessionId, sessionId)) {
                if(user->loggedIn) {
                    user->loggedIn = 0;
                    coPrintf("Logout successful... wimp.\n");
                } else {
                    coPrintf("Not logged in.\n");
                }
                return 1;
            }
        }
        coPrintf("Not logged in.\n");
    } else if(!strcmp(line, "quit")) {
        coPrintf("goodbye.\n");
        return 0;
    } else {
        coPrintf("Unknown command: %s\n", line);
    }
    return 1;
}

/*--------------------------------------------------------------------------------------------------
  This is the read/execute loop.
--------------------------------------------------------------------------------------------------*/
static void executeFileCommands(void)
{
    char line[256];
    char *sessionId;
    int done = 0;

    do {
        sessionId = coStartResponse();
        coPrintf("> ");
        if(!readLine(line)) {
            return;
        }
        done = executeCommand(line, sessionId);
        coCompleteResponse();
    } while(!done);
}

int main(
    int argc,
    char **argv)
{
    if(argc != 1 && argc != 2) {
        coPrintf("Usage: %s [file socket]\n", argv[0]);
        return 1;
    }
    if(argc == 2) {
        readFromFileSocket = 1;
        coStartServer(argv[1]);
    }
    readPasswdFile("passwd");
    executeFileCommands();
    if(argc == 2) {
        coStopServer();
    }
    return 0;
}
