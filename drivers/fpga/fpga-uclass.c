// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Alexander Dahl <post@lespocky.de>
 */

#include <dm.h>
#include <fpga-uclass.h>

int fpga_uclass_load(struct udevice *dev, const void *buf, size_t bsize,
		     bitstream_type bstype, int flags)
{
	const struct fpga_ops *ops = fpga_get_ops(dev);

	if (!ops || !ops->load)
		return -ENOSYS;

	return ops->load(dev, buf, bsize, bstype, flags);
}

int fpga_uclass_loads(struct udevice *dev, const void *buf, size_t bsize,
		      struct fpga_secure_info *info)
{
	const struct fpga_ops *ops = fpga_get_ops(dev);

	if (!ops || !ops->loads)
		return -ENOSYS;

	return ops->loads(dev, buf, bsize, info);
}

int fpga_uclass_loadfs(struct udevice *dev, const void *buf, size_t bsize,
		       fpga_fs_info *fsinfo)
{
	const struct fpga_ops *ops = fpga_get_ops(dev);

	if (!ops || !ops->loadfs)
		return -ENOSYS;

	return ops->loadfs(dev, buf, bsize, fsinfo);
}

int fpga_uclass_dump(struct udevice *dev, const void *buf, size_t bsize)
{
	const struct fpga_ops *ops = fpga_get_ops(dev);

	if (!ops || !ops->dump)
		return -ENOSYS;

	return ops->dump(dev, buf, bsize);
}

int fpga_uclass_info(struct udevice *dev)
{
	const struct fpga_ops *ops = fpga_get_ops(dev);

	if (!ops || !ops->info)
		return -ENOSYS;

	return ops->info(dev);
}

int fpga_uclass_compatible2flag(struct udevice *dev, const char *str)
{
	const struct fpga_ops *ops = fpga_get_ops(dev);

	if (!ops || !ops->compatible2flag)
		return 0;

	return ops->compatible2flag(dev, str);
}

UCLASS_DRIVER(fpga) = {
	.name	= "fpga",
	.id	= UCLASS_FPGA,
};
