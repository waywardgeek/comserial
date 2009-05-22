/* Just to keep responses sane, we limit POST input size. */
#define CGI_INPUT_MAXLEN 100000000
/* Max length of message created with cgiPrintf */
#define CGI_MAX_STRING_LENGTH 4096

/* Calling this causes all text printed by cgiPrintf to be logged to the log file */
void cgiEnableDebug(char *logFile);
/* Call this rather than printf simply to enable optional debug logging */
int cgiPrintf(char *format, ...);
/* Encode a string to replace non-alpha-numeric characters with % escapes, and spaces with +.
   The returned string is allocated with calloc, so the caller will need to free it. */
void cgiEncode(char *string);
/* Unencodes encoded strings in-place, overwriting the original string */
void cgiUnencode(char *string);
/* The following two cookie functions must be called while writing the header of the CGI response */
/* Read a cookie from the client */
char *cgiReadCookie(char *varName);
/* Set a cookie from in client */
void cgiSetCookie(char *varName, char *value);
/* This must also be called while writing the header of the response, so it can set the session ID
   cookie if it has not yet been set */
char *cgiReadSessionId(void);
/* Call this to obtain the values submitted by a POST reqest.  It is used later by cgiReadInputVar */
char *cgiReadInput(void);
/* Find a particular input variable value from the input string */
char *cgiReadInputVar(char *input, char *varName);
