#ifndef _FLASH_H
#define _FLASH_H

#include <linux/ioctl.h>

/* ioctls and their arguments */
#define FLASH_SCHED _IOW('q', 0, flash_arg_t *)
#define FLASH_WRITE _IOW('q', 1, flash_arg_t *)

#endif
