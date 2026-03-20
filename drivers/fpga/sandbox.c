// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Alexander Dahl <post@lespocky.de>
 */

#include <dm.h>
#include <fpga-uclass.h>
#include <log.h>

struct sandbox_fpga_priv {
	const void *last_buf;
	size_t last_bsize;
};

static int sandbox_fpga_load(struct udevice *dev, const void *buf, size_t bsize,
			     bitstream_type bstype, int flags)
{
	struct sandbox_fpga_priv *priv = dev_get_priv(dev);

	log_debug("%s: loading %zu bytes\n", dev->name, bsize);
	priv->last_buf = buf;
	priv->last_bsize = bsize;

	return 0;
}

static int sandbox_fpga_info(struct udevice *dev)
{
	printf("Sandbox FPGA device '%s'\n", dev->name);
	return 0;
}

static int sandbox_fpga_dump(struct udevice *dev, const void *buf, size_t bsize)
{
	printf("Sandbox FPGA dump %zu bytes to '%s'\n", bsize, dev->name);
	return 0;
}

static const struct fpga_ops sandbox_fpga_ops = {
	.load = sandbox_fpga_load,
	.info = sandbox_fpga_info,
	.dump = sandbox_fpga_dump,
};

static const struct udevice_id sandbox_fpga_match[] = {
	{ .compatible = "sandbox,fpga" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sandbox_fpga) = {
	.name = "sandbox_fpga",
	.id = UCLASS_FPGA,
	.of_match = sandbox_fpga_match,
	.priv_auto = sizeof(struct sandbox_fpga_priv),
	.ops = &sandbox_fpga_ops,
};
