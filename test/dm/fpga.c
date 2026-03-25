// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Alexander Dahl <post@lespocky.de>
 */

#include <dm.h>
#include <dm/test.h>
#include <fpga.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_fpga(struct unit_test_state *uts)
{
	struct udevice *dev;
	const char test_data[] = "test fpga data";
	char dump_buf[sizeof(test_data)];

	/* Test that FPGA device is found */
	ut_assertok(uclass_first_device_err(UCLASS_FPGA, &dev));

	/* Test fpga_info operation */
	ut_assertok(fpga_info(0));

	/* Test fpga_load operation */
	ut_assertok(fpga_load(0, test_data, sizeof(test_data), 0, 0));

	/* Test fpga_dump operation */
	ut_assertok(fpga_dump(0, dump_buf, sizeof(dump_buf)));
	ut_asserteq_mem(test_data, dump_buf, sizeof(test_data));

	return 0;
}
DM_TEST(dm_test_fpga, UTF_SCAN_FDT);
