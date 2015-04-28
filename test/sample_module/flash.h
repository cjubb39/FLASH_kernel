#ifndef _FLASH_H
#define _FLASH_H

#include <linux/ioctl.h>

#define CHANGE_REQ 0
#define SCHED_REQ  1

// type, pid, pri, state 8 16 8 16
typedef struct {
	u8  type;
	u16 pid;
	u8  pri;
	u16 state;
} flash_arg_t;

/* ioctls and their arguments */
#define FLASH_SCHED _IOW('q', 0, flash_arg_t *)
#define FLASH_WRITE _IOW('q', 1, flash_arg_t *)

#endif
