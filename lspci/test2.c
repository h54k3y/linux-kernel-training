#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF 1000
#define OPT_NUM 2
#define INFO_NUM 8
#define STR_LEN 15

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
		const char info_title[INFO_NUM][STR_LEN] = { "Device:	", "Class:	", "Vendor:	", "Device:	", "SVendor:	", "SDevice:	"};
		const char option_title[OPT_NUM][STR_LEN] = {"Rev:	", "ProgIf:	"};
		const char option_type[OPT_NUM] = {'r','p'};
		while (!feof(fp)) 
		{
			char buf[BUF] = {};
			if(fgets(buf, sizeof(buf), fp) == NULL)
			{
				break;
			}
			int info_title_ind = 0;
			char opt_str[OPT_NUM][STR_LEN] = {};
			for(int i = 0; i < sizeof(buf); i++)
			{
				if(i == 0)
				{
					//Print "Device:".
					printf("%s", info_title[info_title_ind]);
					info_title_ind++;
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
							info_title_ind++;
							continue;
						}

						printf("%s", info_title[info_title_ind]);
						info_title_ind++;
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
						int opt_type_ind = -1;
						for(int j = 0; j < OPT_NUM; j++)
						{
							// Find a right option type from the array "option_type[]".
							if(option_type[j] == buf[i+1])
							{
								opt_type_ind = j;
								break;
							}
						}

						if(opt_type_ind != -1)
						{
							// If found the right option type, store the option string.
							int j = i+2;
							int cnt = 0;
							while(j < sizeof(buf))
							{
								opt_str[opt_type_ind][cnt] = buf[j];
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
			int option_titile_ind =0; 
			for(int i = 0; i < OPT_NUM; i++)
			{
				if(opt_str[i][0] != '\0')
				{
					printf("%s%s\n", option_title[option_titile_ind], opt_str[i]);
				}
				option_titile_ind++;
			}
			printf("\n");
		}
		(void)pclose(fp);
		exit(EXIT_SUCCESS);
	}
	return 0;
}