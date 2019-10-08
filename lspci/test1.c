#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF 256

int main(int argc, char *argv[])
{
	FILE *fp;
	char cmdline[] = "lspci -v -mm -s ";
	strcat(cmdline, argv[1]);
	if ((fp = popen(cmdline,"r")) == NULL) 
	{
		perror("command exec error");
		exit(EXIT_FAILURE);
	}
	else
	{
		char buf[BUF];
		while (!feof(fp)) 
		{
			if(fgets(buf, sizeof(buf), fp) == NULL)
			{
				break;
			}
			printf("%s", buf);
		}
		(void)pclose(fp);
		exit(EXIT_SUCCESS);
	}
	return 0;
}