/*=================================================================================================
  This is an example of a back-end engine that processes commands from a file.
=================================================================================================*/
#include <stdio.h>

FILE *file;

static void executeFileCommands(void)
{

}

int main(
    int argc,
    char **argv)
{
    if(argc != 0) {
	printf("Usage: %s commandFile\n", argv[0]);
	return 1;
    }
    file = fopen(argv[1], "r");
    if(file == NULL) {
	printf("Unable to open %s for reading\n", argv[1]);
	return 1;
    }
    executeFileCommands();
    fclose(file);
    return 0;
}
