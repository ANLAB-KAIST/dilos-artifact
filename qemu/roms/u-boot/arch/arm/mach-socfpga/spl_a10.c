// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/pl310.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <image.h>
#include <asm/arch/reset_manager.h>
#include <spl.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/freeze_controller.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/scan_manager.h>
#include <asm/arch/sdram.h>
#include <asm/arch/scu.h>
#include <asm/arch/misc.h>
#include <asm/arch/nic301.h>
#include <asm/sections.h>
#include <fdtdec.h>
#include <watchdog.h>
#include <asm/arch/pinmux.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

u32 spl_boot_device(void)
{
	const u32 bsel = readl(&sysmgr_regs->bootinfo);

	switch (SYSMGR_GET_BOOTINFO_BSEL(bsel)) {
	case 0x1:	/* FPGA (HPS2FPGA Bridge) */
		return BOOT_DEVICE_RAM;
	case 0x2:	/* NAND Flash (1.8V) */
	case 0x3:	/* NAND Flash (3.0V) */
		socfpga_per_reset(SOCFPGA_RESET(NAND), 0);
		return BOOT_DEVICE_NAND;
	case 0x4:	/* SD/MMC External Transceiver (1.8V) */
	case 0x5:	/* SD/MMC Internal Transceiver (3.0V) */
		socfpga_per_reset(SOCFPGA_RESET(SDMMC), 0);
		socfpga_per_reset(SOCFPGA_RESET(DMA), 0);
		return BOOT_DEVICE_MMC1;
	case 0x6:	/* QSPI Flash (1.8V) */
	case 0x7:	/* QSPI Flash (3.0V) */
		socfpga_per_reset(SOCFPGA_RESET(QSPI), 0);
		return BOOT_DEVICE_SPI;
	default:
		printf("Invalid boot device (bsel=%08x)!\n", bsel);
		hang();
	}
}

#ifdef CONFIG_SPL_MMC_SUPPORT
u32 spl_boot_mode(const u32 boot_device)
{
#if defined(CONFIG_SPL_FAT_SUPPORT) || defined(CONFIG_SPL_EXT_SUPPORT)
	return MMCSD_MODE_FS;
#else
	return MMCSD_MODE_RAW;
#endif
}
#endif

void spl_board_init(void)
{
	/* enable console uart printing */
	preloader_console_init();
	WATCHDOG_RESET();

	arch_early_init_r();
}

void board_init_f(ulong dummy)
{
	socfpga_init_security_policies();
	socfpga_sdram_remap_zero();

	/* Assert reset to all except L4WD0 and L4TIMER0 */
	socfpga_per_reset_all();
	socfpga_watchdog_disable();

	spl_early_init();

	/* Configure the clock based on handoff */
	cm_basic_init(gd->fdt_blob);

#ifdef CONFIG_HW_WATCHDOG
	/* release osc1 watchdog timer 0 from reset */
	socfpga_reset_deassert_osc1wd0();

	/* reconfigure and enable the watchdog */
	hw_watchdog_init();
	WATCHDOG_RESET();
#endif /* CONFIG_HW_WATCHDOG */

	config_dedicated_pins(gd->fdt_blob);
	WATCHDOG_RESET();
}
