#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#define BUF 500
#define STR_LEN 30
#define PATH_LEN 200

int PrintConfig(char *dir_name);

int main(int argc, char *argv[])
{
  DIR *dir;
  struct dirent *dp;
  char path[] = "/sys/bus/pci/devices/";
  const char info_title[3][STR_LEN] = { "Bus: ", "Slot: ", "Function: "};
  dir = opendir(path);

  for (dp = readdir(dir); dp != NULL; dp = readdir(dir))
  {
    char *ptr;
    char str[] = {};
    strcpy(str, dp->d_name);
    ptr = strtok(str, ":");
    short bus_slot_fun[3] = {};
    
    int cnt = 0;
    bool flg = false;
    while(ptr != NULL)
    {
      ptr = strtok(NULL, ":.");
      if(ptr != NULL)
      {
        bus_slot_fun[cnt] = strtol(ptr, NULL, 16);
        // Check the slot number.
        if((cnt == 1) && (bus_slot_fun[cnt] == strtol(argv[1], NULL, 16)))
        {
          flg = true;
        }
      }
      cnt++;
    }

    if(flg)
    {
      // Print "Bus", "Slot", and "Function".
      for(int i = 0; i < 3; i++)
      {
        printf("%s%x\n", info_title[i], bus_slot_fun[i]);
      }

      // Print "Vendor ID", "Device ID", "Revision ID", "Device Class", "Class Prog", "Subsystem Vendor ID" and "Subsystem Device ID".
      PrintConfig(dp->d_name);
      printf("\n");
    }
  }

  closedir(dir);
  return 0;
}

int PrintConfig(char *dir_name)
{
  FILE* fp;
  char buf[BUF];
  char path[PATH_LEN] = {};
  const char file_name [6][STR_LEN] = {"vendor", "device", "revision", "class", "subsystem_vendor", "subsystem_device"};
  const char config_title[6][STR_LEN] = { "Vendor ID: ", "Device ID: ", "Revision ID: ", "Class", "Subsystem Vendor ID: ", "Subsystem Device ID: "};
  
  for(int i = 0; i < 6; i++)
  {
    sprintf(path,"/sys/bus/pci/devices/%s/%s", dir_name, file_name[i]);
    if((fp = fopen(path,"r")) == NULL) 
    {
      perror("file open error");
      exit(EXIT_FAILURE);
    }
    else
    {
      fread(buf, sizeof(buf), 1, fp);
      if(i == 3)
      {
        // In "/class" file, after "0x", the upper 4 digits inicate "Device Class" and the lower 2 digits inicate "Class Prog".
        char class_prog_str[STR_LEN] = {};
        char device_class_str[STR_LEN] = {};
        sprintf(class_prog_str, "%.2s", buf + 6);
        sprintf(device_class_str, "%.6s", buf);
        long int class_prog = strtol(class_prog_str, NULL, 16);
        long int device_class = strtol(device_class_str, NULL, 16);

        if(class_prog != 0)
        {
          printf("Class Prog: %lx\n", class_prog);
        }
        printf("Device Class: %04lx\n", device_class);
      }
      else
      {
        printf("%s%lx\n", config_title[i], strtol(buf, NULL, 16));
      }
    }
    fclose(fp);
  }
}