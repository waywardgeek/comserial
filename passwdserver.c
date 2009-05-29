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
struct SessionStruct;
typedef struct SessionStruct *Session;

struct UserStruct {
    Session session;
    char loginName[100];
    char password[100];
    User next;
    int loggedIn;
};

static User firstUser;

struct SessionStruct {
    User user;
    char sessionID[100];
    Session next, prev;
};

static Session firstSession;

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
        printf("Unable to open password file %s\n", passwdFileName);
        exit(1);
    }
    while(fscanf(file, "%s %s\n", loginName, password) == 2) {
        user = (User)calloc(1, sizeof(struct UserStruct));
        user->next = firstUser;
        firstUser = user;
        strcpy(user->loginName, loginName);
        strcpy(user->password, password);
    }
    fclose(file);
}

/*--------------------------------------------------------------------------------------------------
  Just read in a line of text.
--------------------------------------------------------------------------------------------------*/
static int readLine(
    char *line)
{
    int xChar = 0;
    int c;

    while(1) {
        c = coGetc();
        if(c == EOF || c == '\n') {
            line[xChar] = '\0';
#ifdef DEBUG
        if(c == EOF) {
            printf("Warning: read EOF\n");
        }
        printf("Read '%s' from client\n", line);
#endif
            return c != EOF || xChar != 0;
        }
        line[xChar++] = c;
    }
    return 0; /* Dummy return */
}

/*--------------------------------------------------------------------------------------------------
  Find the session from the sessionID.
--------------------------------------------------------------------------------------------------*/
static Session findSession(
    char *sessionID)
{
    Session session;

    for(session = firstSession; session != NULL; session = session->next) {
        if(!strcmp(session->sessionID, sessionID)) {
            return session;
        }
    }
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Execute the command in the line variable.  The first word is always the user's sessionID.
  To help keep track of what session we're responding to, print "Start <sessionID>" in the
  beginning of the response, and "Finish <sessionID>" at the end.
--------------------------------------------------------------------------------------------------*/
static int executeCommand(
    Session session,
    char *line)
{
    User user = session->user;
    char loginName[100], password[100];

#ifdef DEBUG
    printf("%s %s, user=%s\n", session->sessionID, line, user == NULL? "none" : user->loginName);
#endif
    if(sscanf(line, "login %s %s", loginName, password) == 2) {
        if(user != NULL && user->loggedIn) {
            coPrintf("Already logged in.\n");
            return 1;
        }
        for(user = firstUser; user != NULL; user = user->next) {
            if(!strcmp(user->loginName, loginName) && !strcmp(user->password, password)) {
                if(user->loggedIn) {
                    coPrintf("This user is already on-line in a different session.\n");
                } else {
                    user->loggedIn = 1;
                    session->user = user;
                    user->session = session;
                    coPrintf("Login successful.  You are a master of the universe!\n");
                }
                return 1;
            }
        }
        coPrintf("Invalid name/password combo.  If you do that again, I'll have to spank you!\n");
    } else if(!strcmp(line, "logout")) {
        if(user ==  NULL) {
            coPrintf("Not logged in.\n");
        } else {
            coPrintf("Logout successful... wimp.\n");
            user->loggedIn = 0;
            user->session->user = NULL;
            user->session = NULL;
        }
    } else if(!strcmp(line, "quit")) {
        coPrintf("goodbye.\n");
        return 0;
    } else {
        coPrintf("Unknown command: %s\n", line);
    }
    return 1;
}

/*--------------------------------------------------------------------------------------------------
  Log out a user when his session ends.
--------------------------------------------------------------------------------------------------*/
static void endSession(
    char *sessionID)
{
    Session session = findSession(sessionID);
    User user;
    Session prev, next;

    if(session != NULL) {
        user = session->user;
        if(user != NULL) {
            user->loggedIn = 0;
            user->session = NULL;
        }
        prev = session->prev;
        next = session->next;
        if(prev != NULL) {
            prev->next = next;
        } else {
            firstSession = next;
        }
        if(next != NULL) {
            next->prev = prev;
        }
        free(session);
    }
}

/*--------------------------------------------------------------------------------------------------
  Create a new session object.
--------------------------------------------------------------------------------------------------*/
static Session createSession(
    char *sessionID)
{
    Session session = (Session)calloc(1, sizeof(struct SessionStruct));

    strcpy(session->sessionID, sessionID);
    session->next = firstSession;
    if(firstSession != NULL) {
        firstSession->prev = session;
    }
    firstSession = session;
    return session;
}

/*--------------------------------------------------------------------------------------------------
  This is the read/execute loop.
--------------------------------------------------------------------------------------------------*/
static void executeFileCommands(void)
{
    char line[256];
    char *sessionID;
    int passed;
    Session session;

    do {
        sessionID = coStartResponse();
        session = findSession(sessionID);
        if(session == NULL) {
            createSession(sessionID);
            coPrintf("Legal commands are: login userName password, logout, and quit.\n");
        } else {
            if(!readLine(line)) {
                return;
            }
            passed = executeCommand(session, line);
        }
        coPrintf("> ");
        coCompleteResponse();
    } while(passed);
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
    coSetEndSessionCallback(endSession);
    executeFileCommands();
    if(argc == 2) {
        coStopServer();
    }
    return 0;
}
