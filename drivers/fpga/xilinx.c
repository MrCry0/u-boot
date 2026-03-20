// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012-2013, Xilinx, Michal Simek
 *
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com
 */

/*
 *  Xilinx FPGA support
 */

#include <fpga.h>
#include <log.h>
#include <virtex2.h>
#include <spartan2.h>
#include <spartan3.h>
#include <zynqpl.h>
#include <linux/string.h>
#if CONFIG_IS_ENABLED(DM_FPGA)
#include <dm.h>
#include <fpga-uclass.h>
#if IS_ENABLED(CONFIG_FPGA_ZYNQMPPL)
#include <zynqmppl.h>
#endif
#if IS_ENABLED(CONFIG_FPGA_VERSALPL)
#include <versalpl.h>
#endif

struct xilinx_fpga_priv {
	xilinx_desc desc;
};
#endif

/* Local Static Functions */
static int xilinx_validate(xilinx_desc *desc, char *fn);

/* ------------------------------------------------------------------------- */

int fpga_is_partial_data(int devnum, size_t img_len)
{
	xilinx_desc *desc_xilinx;

	if (CONFIG_IS_ENABLED(DM_FPGA)) {
		struct udevice *dev;

		if (!uclass_get_device(UCLASS_FPGA, devnum, &dev)) {
			struct xilinx_fpga_priv *priv = dev_get_priv(dev);

			return img_len < priv->desc.size ? 1 : 0;
		}
	}

	desc_xilinx = fpga_get_desc(devnum)->devdesc;
	if (!desc_xilinx)
		return 0;

	/* Check datasize against FPGA size */
	if (img_len >= desc_xilinx->size)
		return 0;

	/* datasize is smaller, must be partial data */
	return 1;
}

int fpga_loadbitstream(int devnum, char *fpgadata, size_t size,
		       bitstream_type bstype)
{
	unsigned int length;
	unsigned int swapsize;
	unsigned char *dataptr;
	unsigned int i;
	const fpga_desc *desc;
	xilinx_desc *xdesc;

	dataptr = (unsigned char *)fpgadata;
	/* Find out fpga_description */
	desc = fpga_validate(devnum, dataptr, 0);
	if (!desc)
		return FPGA_FAIL;

	/* Assign xilinx device description */
	xdesc = desc->devdesc;

	/* skip the first bytes of the bitsteam, their meaning is unknown */
	length = (*dataptr << 8) + *(dataptr + 1);
	dataptr += 2;
	dataptr += length;

	/* get design name (identifier, length, string) */
	length = (*dataptr << 8) + *(dataptr + 1);
	dataptr += 2;
	if (*dataptr++ != 0x61) {
		debug("%s: Design name id not recognized in bitstream\n",
		      __func__);
		return FPGA_FAIL;
	}

	length = (*dataptr << 8) + *(dataptr + 1);
	dataptr += 2;
	printf("  design filename = \"%s\"\n", dataptr);
	dataptr += length;

	/* get part number (identifier, length, string) */
	if (*dataptr++ != 0x62) {
		printf("%s: Part number id not recognized in bitstream\n",
		       __func__);
		return FPGA_FAIL;
	}

	length = (*dataptr << 8) + *(dataptr + 1);
	dataptr += 2;

	if (xdesc->name) {
		i = (ulong)strstr((char *)dataptr, xdesc->name);
		if (!i) {
			printf("%s: Wrong bitstream ID for this device\n",
			       __func__);
			printf("%s: Bitstream ID %s, current device ID %d/%s\n",
			       __func__, dataptr, devnum, xdesc->name);
			return FPGA_FAIL;
		}
	} else {
		printf("%s: Please fill correct device ID to xilinx_desc\n",
		       __func__);
	}
	printf("  part number = \"%s\"\n", dataptr);
	dataptr += length;

	/* get date (identifier, length, string) */
	if (*dataptr++ != 0x63) {
		printf("%s: Date identifier not recognized in bitstream\n",
		       __func__);
		return FPGA_FAIL;
	}

	length = (*dataptr << 8) + *(dataptr + 1);
	dataptr += 2;
	printf("  date = \"%s\"\n", dataptr);
	dataptr += length;

	/* get time (identifier, length, string) */
	if (*dataptr++ != 0x64) {
		printf("%s: Time identifier not recognized in bitstream\n",
		       __func__);
		return FPGA_FAIL;
	}

	length = (*dataptr << 8) + *(dataptr + 1);
	dataptr += 2;
	printf("  time = \"%s\"\n", dataptr);
	dataptr += length;

	/* get fpga data length (identifier, length) */
	if (*dataptr++ != 0x65) {
		printf("%s: Data length id not recognized in bitstream\n",
		       __func__);
		return FPGA_FAIL;
	}
	swapsize = ((unsigned int)*dataptr << 24) +
		   ((unsigned int)*(dataptr + 1) << 16) +
		   ((unsigned int)*(dataptr + 2) << 8) +
		   ((unsigned int)*(dataptr + 3));
	dataptr += 4;
	printf("  bytes in bitstream = %d\n", swapsize);

	return fpga_load(devnum, dataptr, swapsize, bstype, 0);
}

int xilinx_load(xilinx_desc *desc, const void *buf, size_t bsize,
		bitstream_type bstype, int flags)
{
	if (!xilinx_validate(desc, (char *)__FUNCTION__)) {
		printf("%s: Invalid device descriptor\n", __FUNCTION__);
		return FPGA_FAIL;
	}

	if (!desc->operations || !desc->operations->load) {
		printf("%s: Missing load operation\n", __func__);
		return FPGA_FAIL;
	}

	return desc->operations->load(desc, buf, bsize, bstype, flags);
}

#if defined(CONFIG_CMD_FPGA_LOADFS)
int xilinx_loadfs(xilinx_desc *desc, const void *buf, size_t bsize,
		  fpga_fs_info *fpga_fsinfo)
{
	if (!xilinx_validate(desc, (char *)__func__)) {
		printf("%s: Invalid device descriptor\n", __func__);
		return FPGA_FAIL;
	}

	if (!desc->operations || !desc->operations->loadfs) {
		printf("%s: Missing loadfs operation\n", __func__);
		return FPGA_FAIL;
	}

	return desc->operations->loadfs(desc, buf, bsize, fpga_fsinfo);
}
#endif

#if CONFIG_IS_ENABLED(FPGA_LOAD_SECURE)
int xilinx_loads(xilinx_desc *desc, const void *buf, size_t bsize,
		 struct fpga_secure_info *fpga_sec_info)
{
	if (!xilinx_validate(desc, (char *)__func__)) {
		printf("%s: Invalid device descriptor\n", __func__);
		return FPGA_FAIL;
	}

	if (!desc->operations || !desc->operations->loads) {
		printf("%s: Missing loads operation\n", __func__);
		return FPGA_FAIL;
	}

	return desc->operations->loads(desc, buf, bsize, fpga_sec_info);
}
#endif

int xilinx_dump(xilinx_desc *desc, const void *buf, size_t bsize)
{
	if (!xilinx_validate(desc, (char *)__FUNCTION__)) {
		printf("%s: Invalid device descriptor\n", __FUNCTION__);
		return FPGA_FAIL;
	}

	if (!desc->operations || !desc->operations->dump) {
		printf("%s: Missing dump operation\n", __func__);
		return FPGA_FAIL;
	}

	return desc->operations->dump(desc, buf, bsize);
}

int xilinx_info(xilinx_desc *desc)
{
	int ret_val = FPGA_FAIL;

	if (xilinx_validate(desc, (char *)__FUNCTION__)) {
		printf("Family:        \t");
		switch (desc->family) {
		case xilinx_spartan2:
			printf("Spartan-II\n");
			break;
		case xilinx_spartan3:
			printf("Spartan-III\n");
			break;
		case xilinx_virtex2:
			printf("Virtex-II\n");
			break;
		case xilinx_zynq:
			printf("Zynq PL\n");
			break;
		case xilinx_zynqmp:
			printf("ZynqMP PL\n");
			break;
		case xilinx_versal:
			printf("Versal PL\n");
			break;
		/* Add new family types here */
		default:
			printf("Unknown family type, %d\n", desc->family);
		}

		printf("Interface type:\t");
		switch (desc->iface) {
		case slave_serial:
			printf("Slave Serial\n");
			break;
		case master_serial: /* Not used */
			printf("Master Serial\n");
			break;
		case slave_parallel:
			printf("Slave Parallel\n");
			break;
		case jtag_mode: /* Not used */
			printf("JTAG Mode\n");
			break;
		case slave_selectmap:
			printf("Slave SelectMap Mode\n");
			break;
		case master_selectmap:
			printf("Master SelectMap Mode\n");
			break;
		case devcfg:
			printf("Device configuration interface (Zynq)\n");
			break;
		case csu_dma:
			printf("csu_dma configuration interface (ZynqMP)\n");
			break;
		case cfi:
			printf("CFI configuration interface (Versal)\n");
			break;
			/* Add new interface types here */
		default:
			printf("Unsupported interface type, %d\n", desc->iface);
		}

		printf("Device Size:   \t%zd bytes\n"
		       "Cookie:        \t0x%x (%d)\n",
		       desc->size, desc->cookie, desc->cookie);
		if (desc->name)
			printf("Device name:   \t%s\n", desc->name);

		if (desc->iface_fns)
			printf("Device Function Table @ 0x%p\n",
			       desc->iface_fns);
		else
			printf("No Device Function Table.\n");

		if (desc->operations && desc->operations->info)
			desc->operations->info(desc);

		ret_val = FPGA_SUCCESS;
	} else {
		printf("%s: Invalid device descriptor\n", __FUNCTION__);
	}

	return ret_val;
}

/* ------------------------------------------------------------------------- */

static int xilinx_validate(xilinx_desc *desc, char *fn)
{
	int ret_val = false;

	if (desc) {
		if ((desc->family > min_xilinx_type) &&
		    (desc->family < max_xilinx_type)) {
			if ((desc->iface > min_xilinx_iface_type) &&
			    (desc->iface < max_xilinx_iface_type)) {
				if (desc->size) {
					ret_val = true;
				} else
					printf("%s: NULL part size\n", fn);
			} else
				printf("%s: Invalid Interface type, %d\n", fn,
				       desc->iface);
		} else
			printf("%s: Invalid family type, %d\n", fn,
			       desc->family);
	} else
		printf("%s: NULL descriptor!\n", fn);

	return ret_val;
}

/* ------------------------------------------------------------------------- */
/* Driver model support */

#if CONFIG_IS_ENABLED(DM_FPGA)

static int xilinx_dm_load(struct udevice *dev, const void *buf, size_t bsize,
			  bitstream_type bstype, int flags)
{
	struct xilinx_fpga_priv *priv = dev_get_priv(dev);

	return xilinx_load(&priv->desc, buf, bsize, bstype, flags);
}

#if CONFIG_IS_ENABLED(FPGA_LOAD_SECURE)
static int xilinx_dm_loads(struct udevice *dev, const void *buf, size_t bsize,
			   struct fpga_secure_info *info)
{
	struct xilinx_fpga_priv *priv = dev_get_priv(dev);

	return xilinx_loads(&priv->desc, buf, bsize, info);
}
#endif

#if IS_ENABLED(CONFIG_CMD_FPGA_LOADFS)
static int xilinx_dm_loadfs(struct udevice *dev, const void *buf, size_t bsize,
			    fpga_fs_info *fsinfo)
{
	struct xilinx_fpga_priv *priv = dev_get_priv(dev);

	return xilinx_loadfs(&priv->desc, buf, bsize, fsinfo);
}
#endif

static int xilinx_dm_dump(struct udevice *dev, const void *buf, size_t bsize)
{
	struct xilinx_fpga_priv *priv = dev_get_priv(dev);

	return xilinx_dump(&priv->desc, buf, bsize);
}

static int xilinx_dm_info(struct udevice *dev)
{
	struct xilinx_fpga_priv *priv = dev_get_priv(dev);

	return xilinx_info(&priv->desc);
}

#if CONFIG_IS_ENABLED(FPGA_LOAD_SECURE)
static int xilinx_dm_compatible2flag(struct udevice *dev, const char *str)
{
	struct xilinx_fpga_priv *priv = dev_get_priv(dev);
	xilinx_desc *xdesc = &priv->desc;

	if (xdesc->operations && xdesc->operations->str2flag)
		return xdesc->operations->str2flag(xdesc, str);

	return 0;
}
#endif

static const struct fpga_ops xilinx_fpga_ops = {
	.load = xilinx_dm_load,
#if CONFIG_IS_ENABLED(FPGA_LOAD_SECURE)
	.loads = xilinx_dm_loads,
	.compatible2flag = xilinx_dm_compatible2flag,
#endif
#if IS_ENABLED(CONFIG_CMD_FPGA_LOADFS)
	.loadfs = xilinx_dm_loadfs,
#endif
	.dump = xilinx_dm_dump,
	.info = xilinx_dm_info,
};

static int xilinx_fpga_of_to_plat(struct udevice *dev)
{
	struct xilinx_fpga_priv *priv = dev_get_priv(dev);
	xilinx_desc *desc = &priv->desc;
	const char *label;

	desc->family = (xilinx_family)dev_get_driver_data(dev);
	desc->size = dev_read_u32_default(dev, "xlnx,bitstream-size", 1);

	label = dev_read_string(dev, "label");
	if (label)
		desc->name = (char *)label;

	return 0;
}

static int xilinx_fpga_probe(struct udevice *dev)
{
	struct xilinx_fpga_priv *priv = dev_get_priv(dev);
	xilinx_desc *desc = &priv->desc;

	switch (desc->family) {
#if IS_ENABLED(CONFIG_FPGA_ZYNQMPPL)
	case xilinx_zynqmp:
		desc->iface = csu_dma;
		desc->iface_fns = &zynqmp_op;
		desc->operations = &zynqmp_op;
		desc->flags = ZYNQMP_FPGA_FLAGS;
		break;
#endif
#if IS_ENABLED(CONFIG_FPGA_VERSALPL)
	case xilinx_versal:
	case xilinx_versal_net:
	case xilinx_versal2:
		desc->iface = cfi;
		desc->iface_fns = &versal_op;
		desc->operations = &versal_op;
		desc->flags = FPGA_LEGACY;
		break;
#endif
#if IS_ENABLED(CONFIG_FPGA_ZYNQPL)
	case xilinx_zynq:
		desc->iface = devcfg;
		desc->iface_fns = &zynq_op;
		desc->operations = &zynq_op;
		desc->flags = FPGA_LEGACY;
		break;
#endif
	default:
		return -ENODEV;
	}

	return 0;
}

static int xilinx_fpga_remove(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id xilinx_fpga_match[] = {
#if IS_ENABLED(CONFIG_FPGA_ZYNQMPPL)
	{ .compatible = "xlnx,zynqmp-pcap-fpga", .data = xilinx_zynqmp },
#endif
#if IS_ENABLED(CONFIG_FPGA_VERSALPL)
	{ .compatible = "xlnx,versal-fpga", .data = xilinx_versal },
	{ .compatible = "xlnx,versal-net-fpga", .data = xilinx_versal_net },
#endif
#if IS_ENABLED(CONFIG_FPGA_ZYNQPL)
	{ .compatible = "xlnx,zynq-devcfg-1.0", .data = xilinx_zynq },
#endif
	{ /* sentinel */ }
};

U_BOOT_DRIVER(xilinx_fpga) = {
	.name = "xilinx_fpga",
	.id = UCLASS_FPGA,
	.of_match = xilinx_fpga_match,
	.of_to_plat = xilinx_fpga_of_to_plat,
	.probe = xilinx_fpga_probe,
	.remove = xilinx_fpga_remove,
	.priv_auto = sizeof(struct xilinx_fpga_priv),
	.ops = &xilinx_fpga_ops,
};

#endif /* CONFIG_IS_ENABLED(DM_FPGA) */
