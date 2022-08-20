// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_rtc
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test the real time clock runtime services.
 */

#include <efi_selftest.h>

#define EFI_ST_NO_RTC "Could not read real time clock\n"

static struct efi_runtime_services *runtime;

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * @return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	runtime = systable->runtime;
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Display current time.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	struct efi_time tm;

	/* Display current time */
	ret = runtime->get_time(&tm, NULL);
	if (ret != EFI_SUCCESS) {
#ifdef CONFIG_CMD_DATE
		efi_st_error(EFI_ST_NO_RTC);
		return EFI_ST_FAILURE;
#else
		efi_st_todo(EFI_ST_NO_RTC);
		return EFI_ST_SUCCESS;
#endif
	} else {
		efi_st_printf("Time according to real time clock: "
			      "%.4u-%.2u-%.2u %.2u:%.2u:%.2u\n",
			      tm.year, tm.month, tm.day,
			      tm.hour, tm.minute, tm.second);
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(rtc) = {
	.name = "real time clock",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
