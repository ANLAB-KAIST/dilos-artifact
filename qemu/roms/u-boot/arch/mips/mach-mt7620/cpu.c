// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <ram.h>
#include <wdt.h>
#include <asm/io.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include "mt76xx.h"

#define STR_LEN			6

#ifdef CONFIG_BOOT_ROM
int mach_cpu_init(void)
{
	ddr_calibrate();

	return 0;
}
#endif

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE, SZ_256M);

	return 0;
}

int print_cpuinfo(void)
{
	static const char * const boot_str[] = { "PLL (3-Byte SPI Addr)",
						 "PLL (4-Byte SPI Addr)",
						 "XTAL (3-Byte SPI Addr)",
						 "XTAL (4-Byte SPI Addr)" };
	const void *blob = gd->fdt_blob;
	void __iomem *sysc_base;
	char buf[STR_LEN + 1];
	fdt_addr_t base;
	fdt_size_t size;
	char *str;
	int node;
	u32 val;

	/* Get system controller base address */
	node = fdt_node_offset_by_compatible(blob, -1, "ralink,mt7620a-sysc");
	if (node < 0)
		return -FDT_ERR_NOTFOUND;

	base = fdtdec_get_addr_size_auto_noparent(blob, node, "reg",
						  0, &size, true);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	sysc_base = ioremap_nocache(base, size);

	str = (char *)sysc_base + MT76XX_CHIPID_OFFS;
	snprintf(buf, STR_LEN + 1, "%s", str);
	val = readl(sysc_base + MT76XX_CHIP_REV_ID_OFFS);
	printf("CPU:   %-*s Rev %ld.%ld - ", STR_LEN, buf,
	       (val & GENMASK(11, 8)) >> 8, val & GENMASK(3, 0));

	val = (readl(sysc_base + MT76XX_SYSCFG0_OFFS) & GENMASK(3, 1)) >> 1;
	printf("Boot from %s\n", boot_str[val]);

	return 0;
}

#ifdef CONFIG_WATCHDOG
static struct udevice *watchdog_dev;

/* Called by macro WATCHDOG_RESET */
void watchdog_reset(void)
{
	static ulong next_reset;
	ulong now;

	if (!watchdog_dev)
		return;

	now = get_timer(0);

	/* Do not reset the watchdog too often */
	if (now > next_reset) {
		next_reset = now + 1000;	/* reset every 1000ms */
		wdt_reset(watchdog_dev);
	}
}
#endif

int arch_misc_init(void)
{
	/*
	 * It has been noticed, that sometimes the d-cache is not in a
	 * "clean-state" when U-Boot is running on MT7688. This was
	 * detected when using the ethernet driver (which uses d-cache)
	 * and a TFTP command does not complete. Flushing the complete
	 * d-cache (again?) here seems to fix this issue.
	 */
	flush_dcache_range(gd->bd->bi_memstart,
			   gd->bd->bi_memstart + gd->ram_size - 1);

#ifdef CONFIG_WATCHDOG
	/* Init watchdog */
	if (uclass_get_device_by_seq(UCLASS_WDT, 0, &watchdog_dev)) {
		debug("Watchdog: Not found by seq!\n");
		if (uclass_get_device(UCLASS_WDT, 0, &watchdog_dev)) {
			puts("Watchdog: Not found!\n");
			return 0;
		}
	}

	wdt_start(watchdog_dev, 60000, 0);	/* 60 seconds */
	printf("Watchdog: Started\n");
#endif

	return 0;
}
