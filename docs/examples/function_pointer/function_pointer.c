#include <stdio.h>

struct net_device;

typedef int (*net_fn)(struct net_device *dev);

struct net_device {
  const char *name;
  net_fn open;
  net_fn close;
};

int open_device(struct net_device *dev) {
  printf("open %s\n", dev->name);
  return 0;
}

int close_device(struct net_device *dev) {
  printf("close %s\n", dev->name);
  return 0;
}

int main(void) {
  printf("Hi, C!\n");

  struct net_device dev = {
      .name = "eth0",
      .open = open_device,
      .close = close_device,
  };

  dev.open(&dev);
  dev.close(&dev);
  return 0;
}
