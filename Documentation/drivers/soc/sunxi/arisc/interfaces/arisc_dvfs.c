/*
 *  drivers/arisc/interfaces/arisc_dvfs.c
 *
 * Copyright (c) 2012 Allwinner.
 * 2012-05-01 Written by sunny (sunny@allwinnertech.com).
 * 2012-10-01 Written by superm (superm@allwinnertech.com).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "../arisc_i.h"

/*
 * set specific pll target frequency.
 * @freq:    target frequency to be set, based on KHZ;
 * @pll:     which pll will be set
 * @mode:    the attribute of message, whether syn or asyn;
 * @cb:      callback handler;
 * @cb_arg:  callback handler arguments;
 *
 * return: result, 0 - set frequency successed,
 *                !0 - set frequency failed;
 */
int arisc_dvfs_set_cpufreq(unsigned int freq, unsigned int pll, unsigned int mode, arisc_cb_t cb, void *cb_arg)
{
	int                   ret = 0;

	pr_info("arisc dvfs request : %d\n", freq);
	ret = invoke_scp_fn_smc(ARM_SVC_ARISC_CPUX_DVFS_REQ, freq, pll, mode);
	if (ret) {
		if (cb == NULL) {
			pr_warn("callback not install\n");
		} else {
			/* call callback function */
			pr_warn("call the callback function\n");
			(*(cb))(cb_arg);
		}
	}

	return ret;
}

EXPORT_SYMBOL(arisc_dvfs_set_cpufreq);

int arisc_dvfs_cfg_vf_table(unsigned int cluster, unsigned int vf_num,
				unsigned long vf_tbl)
{
	int result;

	result = invoke_scp_fn_smc(ARM_SVC_ARISC_CPUX_DVFS_CFG_VF_REQ, cluster,
			vf_num, vf_tbl);

	return 0;
}
EXPORT_SYMBOL(arisc_dvfs_cfg_vf_table);
