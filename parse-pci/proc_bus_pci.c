#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF 500
#define REVISION_ID 0x08
#define CLASS_DEVICE 0x0a
#define SUBSYSTEM_VENDOR_ID 0x2c
#define SUBSYSTEM_DEVICE_ID 0x2e

int GetConfig(short bus, short slot, short function);

int main(int argc, char *argv[])
{
  FILE* fp;
  char buf[BUF];
  char path[] = "/proc/bus/pci/devices";

  if ((fp = fopen(path,"r")) == NULL) 
  {
    perror("file open error");
    exit(EXIT_FAILURE);
  }

  while(fgets(buf, sizeof(buf), fp))
  {
    int bus_dev_fun, vndid_devid;
    sscanf(buf, "%x %x", &bus_dev_fun, &vndid_devid);

    short bus = bus_dev_fun >> 8U;
    short slot = 0x1f & ((bus_dev_fun & 0xff) >> 3);
    short function = 0x07 & (bus_dev_fun & 0xff);
    int vendor_id = 0xffff & (vndid_devid >> 16);
    int device_id = 0xffff & vndid_devid;

    if(slot == strtol(argv[1], NULL, 16))
    {
      printf("Bus: %x, Slot: %x, Function: %x, VendorID %04x, DeviceID %04x\n",
      bus, slot, function, vendor_id, device_id);
      GetConfig(bus, slot, function);
      printf("\n\n");
    }
  }

  fclose(fp);

  return 0;
}

int  GetConfig(short bus, short slot, short function)
{
  FILE* fp;
  char buf[BUF];
  char path[100] = {};
  sprintf(path,"/proc/bus/pci/%02x/%02x.%x", bus, slot, function);

  if ((fp = fopen(path,"r")) == NULL) 
  {
    perror("file open error");
    exit(EXIT_FAILURE);
  }

  fread(buf, sizeof(buf), 1, fp);

  short rev_id =  (short)buf[REVISION_ID];
  short class_id = (short)((buf[CLASS_DEVICE+1]&0xff)<<8)|(buf[CLASS_DEVICE]&0xff);
  short subvnd_id = (short)((buf[SUBSYSTEM_VENDOR_ID+1]&0xff)<<8) |(buf[SUBSYSTEM_VENDOR_ID]&0xff);
  short subdev_id = (short)((buf[SUBSYSTEM_DEVICE_ID+1]&0xff)<<8) | (buf[SUBSYSTEM_DEVICE_ID]&0xff);

  printf("ClassID: %04x, RevisionID: %x, ", class_id, rev_id);

  if((subvnd_id !=0) || (subdev_id != 0))
  {
    printf("SubVendorID: %x, SubDeviceID: %x", subvnd_id&0xffff, subdev_id&0xffff);
  }

  fclose(fp);

  return 0;
}
