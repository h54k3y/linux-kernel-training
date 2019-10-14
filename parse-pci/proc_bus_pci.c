#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF 500
#define REVISION_ID 0x08
#define CLASS_PROG 0x09
#define DEVICE_CLASS 0x0a
#define SUBSYSTEM_VENDOR_ID 0x2c
#define SUBSYSTEM_DEVICE_ID 0x2e

int PrintConfig(short bus, short slot, short function);

int main(int argc, char *argv[])
{
  FILE* fp;
  char buf[BUF];
  char path[] = "/proc/bus/pci/devices";

  if((fp = fopen(path,"r")) == NULL) 
  {
    perror("file open error");
    exit(EXIT_FAILURE);
  }

  while(fgets(buf, sizeof(buf), fp))
  {
    int bus_slot_fun, vndid_devid;
    sscanf(buf, "%x %x", &bus_slot_fun, &vndid_devid);

    short bus = bus_slot_fun >> 8U;
    short slot = 0x1f & ((0xff & bus_slot_fun) >> 3);
    short function = 0x07 & (0xff & bus_slot_fun);
    int vendor_id = 0xffff & (vndid_devid >> 16);
    int device_id = 0xffff & vndid_devid;

    if(slot == strtol(argv[1], NULL, 16))
    {
      printf("Bus: %x\nSlot: %x\nFunction: %x\nVendor ID: %x\nDevice ID: %x\n", bus, slot, function, vendor_id, device_id);

      PrintConfig(bus, slot, function);

      printf("\n");
    }
  }

  fclose(fp);

  return 0;
}

int  PrintConfig(short bus, short slot, short function)
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
