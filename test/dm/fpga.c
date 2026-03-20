// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Alexander Dahl <post@lespocky.de>
 */

#include <dm.h>
#include <dm/test.h>
#include <fpga-uclass.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_fpga(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_FPGA, &dev));

	return 0;
}
DM_TEST(dm_test_fpga, UTF_SCAN_FDT);

static int dm_test_fpga_load(struct unit_test_state *uts)
{
	struct udevice *dev;
	char buf[16] = { 0 };

	ut_assertok(uclass_first_device_err(UCLASS_FPGA, &dev));
	ut_assertok(fpga_uclass_load(dev, buf, sizeof(buf), BIT_FULL, 0));

	return 0;
}
DM_TEST(dm_test_fpga_load, UTF_SCAN_PDATA | UTF_PROBE_TEST);

static int dm_test_fpga_dump(struct unit_test_state *uts)
{
	struct udevice *dev;
	char buf[16] = { 0 };

	ut_assertok(uclass_first_device_err(UCLASS_FPGA, &dev));
	ut_assertok(fpga_uclass_dump(dev, buf, sizeof(buf)));

	return 0;
}
DM_TEST(dm_test_fpga_dump, UTF_SCAN_PDATA | UTF_PROBE_TEST);
