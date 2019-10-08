#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF 1000
#define OPT_NUM 2
#define TITLE_NUM 8

int main(int argc, char *argv[])
{
	FILE *fp;
	char cmdline[] = "lspci -mm -s ";
	strcat(cmdline, argv[1]);
	if ((fp = popen(cmdline,"r")) == NULL) 
	{
		perror("command exec error");
		exit(EXIT_FAILURE);
	}
	else
	{
		const char title[TITLE_NUM][10] = { "Device:	", "Class:	", "Vendor:	", "Device:	", "SVendor:	", "SDevice:	", "Rev:	", "ProgIf:	"};
		const char option[OPT_NUM] = {'r','p'};
		while (!feof(fp)) 
		{
			char buf[BUF] = {};
			if(fgets(buf, sizeof(buf), fp) == NULL)
			{
				break;
			}
			int t_cnt = 0;
			char opt_str[OPT_NUM][10] = {};
			int opt_cnt = 0;
			for(int i = 0; i < sizeof(buf); i++)
			{
				if(i == 0)
				{
					//Print "Device:".
					printf("%s", title[t_cnt]);
					t_cnt++;
					while((i < sizeof(buf)) && buf[i] != ' ')
					{
						printf("%c", buf[i]);
						i++;
					}
					printf("\n");
				}

				if(buf[i] == ' ')
				{
					// Print "Class:" ~ "SDevice:".
					if((i+1 < sizeof(buf)) && (buf[i+1] == '\"'))
					{
						if((i+2 < sizeof(buf)) && (buf[i+2] == '\"'))
						{
							// Skip printing this info if it's empty string.
							t_cnt++;
							continue;
						}

						printf("%s", title[t_cnt]);
						t_cnt++;
						int j = i+2;
						while((j < sizeof(buf)) && buf[j] != '\"')
						{
							printf("%c", buf[j]);
							j++;
						}
						printf("\n");
						i = j;
					}
				}
				else if(buf[i] == '-')
				{
					// Store options beginning with "-"" (ex."-r" and "-p").
					if(i+1 < sizeof(buf))
					{
						int opt_ind = -1;
						for(int j = 0; j < sizeof(option); j++)
						{
							// Find a right option kind from the array "option[]".
							if(option[j] == buf[i+1])
							{
								opt_ind = j;
								break;
							}
						}

						if(opt_ind != -1)
						{
							// If found the right option kind, store the option string.
							opt_cnt++;
							int j = i+2;
							int cnt = 0;
							while(j < sizeof(buf))
							{
								opt_str[opt_ind][cnt] = buf[j];
								if((j+1 < sizeof(buf)) && (buf[j+1] == ' '))
								{
									break;
								}
								j++;
								cnt++;
							}
							i = j;
						}
					}
				}
			}

			// Print stored options. 
			for(int i = 0; i < opt_cnt; i++)
			{
				printf("%s%s\n", title[t_cnt], opt_str[i]);
				t_cnt++;
			}
			printf("\n");
		}
		(void)pclose(fp);
		exit(EXIT_SUCCESS);
	}
	return 0;
}