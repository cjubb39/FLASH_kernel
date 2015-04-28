#ifndef __FLASH_DEV__
#define __FLASH_DEV__

#define SCHED_REQ  0
#define CHANGE_REQ 1

typedef struct {
	u8  type;
	u16 pid;
	u8  pri;
	u16 state;
} flash_arg_t;

struct flash_dev {
	struct resource res; /* Resource: our registers */
	void __iomem *virtbase; /* Where registers can be accessed in memory */
	void (*change_write_to_flash) (struct flash_dev *dev, flash_arg_t vla);
	uint16_t (*sched_write_to_flash)  (struct flash_dev *dev, flash_arg_t vla);
	int irq_pending;
	uint32_t next_task;
};

#endif

