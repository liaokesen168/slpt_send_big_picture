#include <config.h>
#include <common.h>
#include <stdarg.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <rio_dev.h>
#include <linux/err.h>

#define binfo(fmt,args...) serial_printf(fmt, ##args)
#define berr(fmt,args...) serial_printf("\033[1;31m" fmt "\033[0m", ##args);
#define bprintf(fmt,args...) printf(fmt, ##args)

#if 0
#define bdbg(fmt,args...) serial_printf("rio: " fmt, ##args)
#define bmsg(fmt,args...) serial_printf(fmt, ##args)
#else
#define bdbg(fmt,args...) do{}while(0)
#define bmsg(fmt,args...) do{}while(0)
#endif

#define SIZE_1M (1024 * 1024)

static int cmd_burn(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 rsize;
	u32 addr;
	u32 asize;
	u32 read_time;

	ssize_t n = 0;
	struct rio_dev *rdev;
	const char* name = "usbtty";

	if (argc != 4)
		return CMD_RET_USAGE;

	rdev = rio_open(name, 0, 0);
	if (IS_ERR(rdev)) {
		binfo("Cannot open rio dev %s\n", name);
		goto DEV_ERR;
	}

	asize = (u32) simple_strtoul(argv[1], NULL, 10);
	addr = (u32) simple_strtoul(argv[2], NULL, 16);
	rsize = (u32) simple_strtoul(argv[3], NULL, 16);

	binfo("burn %s addr 0x%x size 0x%x\n", name, addr, rsize);
	/* use ioctl to set rio read time out */
	rio_ioctl(rdev, RIO_IOCTL_SET_TIMEOUT, 3000);

	read_time = get_timer(0);

	do {
		int block, ret, tmp;
		block = SIZE_1M * asize;
		if (block > (rsize - n))
			tmp = rsize - n;
		else
			tmp = block;

		ret = rio_read(rdev, (void *)((u32)addr + n), tmp);
		if(ret < 0) {
			binfo("stdio %s read %d bytes return err total %d\n",
					name, tmp, n);
			goto READ_ERR;
		}
		n += ret;
		bprintf ("stdio %s read %d[ox%x] total %d[ox%x] !\n",
				name, ret, ret, n, n);
	}while(n < rsize);

	read_time = get_timer(read_time);

	binfo ("stdio %s read total %d[ox%x] speed:", name, n, n);

	print_size ((rsize / read_time) * 1000, "/s\n");

	rio_close(rdev);
	return CMD_RET_SUCCESS;

READ_ERR:
	rio_close(rdev);
DEV_ERR:
	return CMD_RET_FAILURE;
}

U_BOOT_CMD(
	burner, 6, 1, cmd_burn,
	"burner function test command",
	"\nburner n[*1MB] addr size\n"
	);

