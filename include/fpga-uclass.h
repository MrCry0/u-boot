/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * FPGA uclass interface - driver model ops for FPGA devices
 *
 * Parallel to include/clk-uclass.h
 */

#ifndef _FPGA_UCLASS_H_
#define _FPGA_UCLASS_H_

#include <dm/device.h>
#include <fpga.h>

/**
 * struct fpga_ops - operations for FPGA uclass devices
 *
 * @load:            Load bitstream into FPGA
 * @loads:           Load secure (auth/enc) bitstream (optional)
 * @loadfs:          Load bitstream from filesystem (optional)
 * @dump:            Dump FPGA state to buffer
 * @info:            Print device information
 * @compatible2flag: Convert compatible string to bitstream flag (optional)
 */
struct fpga_ops {
	int (*load)(struct udevice *dev, const void *buf, size_t bsize,
		    bitstream_type bstype, int flags);
	int (*loads)(struct udevice *dev, const void *buf, size_t bsize,
		     struct fpga_secure_info *info);
	int (*loadfs)(struct udevice *dev, const void *buf, size_t bsize,
		      fpga_fs_info *fsinfo);
	int (*dump)(struct udevice *dev, const void *buf, size_t bsize);
	int (*info)(struct udevice *dev);
	int (*compatible2flag)(struct udevice *dev, const char *str);
};

/**
 * fpga_get_ops() - get ops pointer for a FPGA device
 * @dev: FPGA device
 * Return: pointer to fpga_ops
 */
static inline const struct fpga_ops *fpga_get_ops(struct udevice *dev)
{
	return (const struct fpga_ops *)dev->driver->ops;
}

int fpga_uclass_load(struct udevice *dev, const void *buf, size_t bsize,
		     bitstream_type bstype, int flags);
int fpga_uclass_loads(struct udevice *dev, const void *buf, size_t bsize,
		      struct fpga_secure_info *info);
int fpga_uclass_loadfs(struct udevice *dev, const void *buf, size_t bsize,
		       fpga_fs_info *fsinfo);
int fpga_uclass_dump(struct udevice *dev, const void *buf, size_t bsize);
int fpga_uclass_info(struct udevice *dev);
int fpga_uclass_compatible2flag(struct udevice *dev, const char *str);

#endif /* _FPGA_UCLASS_H_ */
