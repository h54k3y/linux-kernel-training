#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#define BUF 500
#define REVISION_ID 0x08
#define CLASS_PROG 0x09
#define DEVICE_CLASS 0x0a
#define SUBSYSTEM_VENDOR_ID 0x2c
#define SUBSYSTEM_DEVICE_ID 0x2e
#define STR_LEN 30
#define PATH_LEN 200

int PrintProc(short arg_slot);
int PrintProcConfig(short bus, short slot, short function);
int PrintSys(short arg_slot);
int PrintSysConfig(char *dir_name);

int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    printf("Wrong count of arguements. Please excute only with slot number as an arguement.\n");
    exit(EXIT_FAILURE);
  }
  char *e;
  errno = 0;
  short arg_slot = strtol(argv[1], &e, 16);
  if(((errno == ERANGE) && ((arg_slot == LONG_MAX) || (arg_slot == LONG_MIN))) || (*e != '\0'))
  {
    printf("Argument conversion failed. Please input a hex number.\n");
    exit(EXIT_FAILURE);
  }
  PrintProc(arg_slot);
  PrintSys(arg_slot);
  return 0;
}

int PrintProc(short arg_slot)
{
  FILE* fp;
  char buf[BUF];
  char path[] = "/proc/bus/pci/devices";

  if((fp = fopen(path,"r")) == NULL) 
  {
    perror("file open error");
    exit(EXIT_FAILURE);
  }

  printf("---------- Print /proc/bus/pci ----------\n");
  while(fgets(buf, sizeof(buf), fp))
  {
    int bus_slot_fun, vndid_devid;
    sscanf(buf, "%x %x", &bus_slot_fun, &vndid_devid);

    short bus = bus_slot_fun >> 8U;
    short slot = 0x1f & ((0xff & bus_slot_fun) >> 3);
    short function = 0x07 & (0xff & bus_slot_fun);
    int vendor_id = 0xffff & (vndid_devid >> 16);
    int device_id = 0xffff & vndid_devid;

    if(slot == arg_slot)
    {
      printf("Bus: %x\nSlot: %x\nFunction: %x\nVendor ID: %x\nDevice ID: %x\n", bus, slot, function, vendor_id, device_id);

      PrintProcConfig(bus, slot, function);

      printf("\n");
    }
  }

  fclose(fp);

  return 0;
}

int  PrintProcConfig(short bus, short slot, short function)
{
  FILE* fp;
  char buf[BUF];
  char path[100] = {};
  sprintf(path,"/proc/bus/pci/%02x/%02x.%x", bus, slot, function);

  if((fp = fopen(path,"r")) == NULL) 
  {
    perror("file open error");
    exit(EXIT_FAILURE);
  }

  fread(buf, sizeof(buf), 1, fp);

  short rev_id =  (short)(0xff & buf[REVISION_ID]);
  short class_prog = (short)(0xff & buf[CLASS_PROG]);
  short device_class = (short)((0xff & buf[DEVICE_CLASS + 1]) << 8) | (0xff & buf[DEVICE_CLASS]);
  short subvnd_id = (short)((0xff & buf[SUBSYSTEM_VENDOR_ID + 1]) << 8) | (0xff & buf[SUBSYSTEM_VENDOR_ID]);
  short subdev_id = (short)((0xff & buf[SUBSYSTEM_DEVICE_ID + 1]) << 8) | (0xff & buf[SUBSYSTEM_DEVICE_ID]);

  printf("Revision ID: %x\n", rev_id);

  if(class_prog != 0)
  {
    printf("Class Prog: %x\n", class_prog);
  }

  printf("Device Class: %04x\n", device_class);

  if((subvnd_id != 0) || (subdev_id != 0))
  {
    printf("Subsystem Vendor ID: %x\nSubsystem Device ID: %x\n", 0xffff & subvnd_id, 0xffff & subdev_id);
  }

  fclose(fp);

  return 0;
}

int PrintSys(short arg_slot)
{
  DIR *dir;
  struct dirent *dp;
  char path[] = "/sys/bus/pci/devices/";
  const char info_title[3][STR_LEN] = { "Bus: ", "Slot: ", "Function: "};
  dir = opendir(path);

  printf("---------- Print /sys/bus/pci  ----------\n");
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
        if((cnt == 1) && (bus_slot_fun[cnt] == arg_slot))
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
      PrintSysConfig(dp->d_name);
      printf("\n");
    }
  }

  closedir(dir);
  return 0;
}

int PrintSysConfig(char *dir_name)
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