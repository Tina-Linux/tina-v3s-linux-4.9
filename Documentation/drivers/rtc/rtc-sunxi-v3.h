/*
 * Some macro and struct of SUNXI RTC.
 *
 * Copyright (C) 2016 Allwinner.
 *
 * Matteo <duanmintao@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef _RTC_SUNXI_V3_H_
#define _RTC_SUNXI_V3_H_

#include "rtc-sunxi-common.h"

/* Registers */
#define SUNXI_LOSC_CTRL_REG				0x00000000
#define LOSC_CTRL_KEY_FIELD_MAGIC		0x16aa0000
#define SUNXI_LOSC_OUT_GATING_REG		0x00000060

/* debug */
#define SUNXI_DEBUG_MODE_FLAG           (0x59)
/* efex */
#define SUNXI_EFEX_CMD_FLAG             (0x5A)
/* boot-resignature */
#define SUNXI_BOOT_RESIGNATURE_FLAG     (0x5B)
/* recovery or boot-recovery */
#define SUNXI_BOOT_RECOVERY_FLAG        (0x5C)
/* sysrecovery */
#define SUNXI_SYS_RECOVERY_FLAG         (0x5D)
/* usb-recovery*/
#define SUNXI_USB_RECOVERY_FLAG         (0x5E)
/* bootloader */
#define SUNXI_FASTBOOT_FLAG             (0x5F)
/* uboot */
#define SUNXI_UBOOT_FLAG                (0x60)


#endif /* end of _RTC_SUNXI_V3_H_ */

