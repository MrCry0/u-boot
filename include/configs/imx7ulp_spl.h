/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SPL definitions for the i.MX7ULP SPL
 *
 * (C) Copyright 2019 Foundries.io
 */

#ifndef __IMX7ULP_SPL_CONFIG_H
#define __IMX7ULP_SPL_CONFIG_H

#if CONFIG_IS_ENABLED(SPL)
/*
 * see figure 35-5 in i.MX 7ULP Reference manual:
 *  - IMX7ULP A7 OCRAM free area RAM is from 0x2F010000 to 0x2F03FF00.
 *  - Set the stack at the end of the free area section, at 0x2003FEB8.
 *  - The BOOT ROM loads what they consider the firmware image
 *    which consists of a 4K header in front of us that contains the IVT, DCD,
 *    and some padding thus 'our' max size is really 0x2F03FF00 - 0x2F011000.
 *    187KB is more than enough for the SPL.
 */
#define CONFIG_SPL_STACK		0x2F03FEB8

/* MMC support */
#if CONFIG_IS_ENABLED(SPL_MMC)
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SYS_MONITOR_LEN			409600	/* 400 KB */
#endif

/* Define the payload for FAT/EXT support */
#if CONFIG_IS_ENABLED(SPL_FS_FAT) || CONFIG_IS_ENABLED(SPL_FS_EXT4)
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot-dtb.img"
#endif

#define CONFIG_SYS_SPL_MALLOC_START    0x68300000
#define CONFIG_SYS_SPL_MALLOC_SIZE     0x100000		/* 1 MB */

#endif /* CONFIG_SPL */

#endif /* __IMX7ULP_SPL_CONFIG_H */
