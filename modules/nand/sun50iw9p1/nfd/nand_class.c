/***************************************************************************
 * nand_class.c for  SUNXI NAND .
 *
 * Copyright (C) 2016 Allwinner.
 *
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 ***************************************************************************/

#include "nand_class.h"

/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
void obj_test_release(struct kobject *kobject)
{
	nand_dbg_err("release");
}

/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static ssize_t nand_test_show(struct kobject *kobject, struct attribute *attr,
			      char *buf)
{
	//ssize_t count = 0;
	//struct nand_kobject *nand_kobj;

	//nand_kobj = (struct nand_kobject *)kobject;
	//print_nftl_zone(nand_kobj->nftl_blk->nftl_zone);
	//count = sprintf(buf, "%i", g_iShowVar);
	//return count;

	u32 chipid[2] = {0};
	PHY_ReadNandId_0(0, chipid);
	return sprintf(buf, "%.8x %.8x\n", chipid[0], chipid[1]);
}

/****************************************************************************
*Name         :
*Description  :receive testcase num from echo command
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static ssize_t nand_test_store(struct kobject *kobject, struct attribute *attr,
			       const char *buf, size_t count)
{
	int ret, i;
	int argnum = 0;
	char cmd[32] = { 0 };
	unsigned int param0 = 0;
	unsigned int param1 = 0;
	unsigned int param2 = 0;
	char param3[16] = { 0 };
	char *tempbuf;
	struct nand_kobject *nand_kobj;

	nand_kobj = (struct nand_kobject *) kobject;

	g_iShowVar = -1;

	argnum =
	    sscanf(buf, "%31s %u %u %u %15s", cmd, &param0, &param1, &param2,
		   param3);
	nand_dbg_err("argnum=%i, cmd=%s, param0=%u, param1=%u, param2=%u\n",
		     argnum, cmd, param0, param1, param2);

	if (-1 == argnum) {
		nand_dbg_err("cmd format err!");
		g_iShowVar = -3;
		goto NAND_TEST_STORE_EXIT;
	}

	if (strcmp(cmd, "help") == 0) {
		nand_dbg_err("nand debug cmd:\n");
		nand_dbg_err("2016-10-24 19:48\n");
	} else if (strcmp(cmd, "flush") == 0) {
		nand_dbg_err("nand debug cmd:\n");
		nand_dbg_err("  flush\n");
		mutex_lock(nand_kobj->nftl_blk->blk_lock);
		ret =
		    nand_kobj->nftl_blk->flush_write_cache(nand_kobj->nftl_blk,
							   param0);
		mutex_unlock(nand_kobj->nftl_blk->blk_lock);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "gcall") == 0) {
		nand_dbg_err("nand debug cmd:\n");
		nand_dbg_err("  gcall\n");
		mutex_lock(nand_kobj->nftl_blk->blk_lock);
		ret = gc_all(nand_kobj->nftl_blk->nftl_zone);
		mutex_unlock(nand_kobj->nftl_blk->blk_lock);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "gcone") == 0) {
		nand_dbg_err("nand debug cmd:\n");
		nand_dbg_err("  gcone\n");
		mutex_lock(nand_kobj->nftl_blk->blk_lock);
		ret = gc_one(nand_kobj->nftl_blk->nftl_zone);
		mutex_unlock(nand_kobj->nftl_blk->blk_lock);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "priogc") == 0) {
		nand_dbg_err("nand debug cmd:\n");
		nand_dbg_err("  priogc\n");
		mutex_lock(nand_kobj->nftl_blk->blk_lock);
		ret =
		    prio_gc_one(nand_kobj->nftl_blk->nftl_zone, param0, param1);
		mutex_unlock(nand_kobj->nftl_blk->blk_lock);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "test") == 0) {
		nand_dbg_err("nand debug cmd:\n");
		nand_dbg_err("  test\n");
		mutex_lock(nand_kobj->nftl_blk->blk_lock);
		ret =
		    nftl_set_zone_test((void *)nand_kobj->nftl_blk->nftl_zone,
				       param0);
		mutex_unlock(nand_kobj->nftl_blk->blk_lock);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "showall") == 0) {
		nand_dbg_err("nand debug cmd:\n");
		nand_dbg_err("  show all\n");
		print_free_list(nand_kobj->nftl_blk->nftl_zone);
		print_block_invalid_list(nand_kobj->nftl_blk->nftl_zone);

		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "showinfo") == 0) {
		nand_dbg_err("nand debug cmd:\n");
		nand_dbg_err("  show info\n");
		print_nftl_zone(nand_kobj->nftl_blk->nftl_zone);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "blkdebug") == 0) {
		nand_dbg_err("nand debug cmd:\n");
		nand_dbg_err("  blk debug\n");
		debug_data = param0;
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "smart") == 0) {
		nand_dbg_err("nand debug cmd:\n");
		nand_dbg_err("smart info\n");
		print_smart(nand_kobj->nftl_blk->nftl_zone);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "read1") == 0) {
		nand_dbg_err("nand read1 cmd:\n");
		nand_dbg_phy_read(param0, param1, param2);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "read2") == 0) {
		nand_dbg_err("nand read2 cmd:\n");
		nand_dbg_zone_phy_read(nand_kobj->nftl_blk->nftl_zone, param0,
				       param1);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "erase1") == 0) {
		nand_dbg_err("nand erase1 cmd:\n");
		nand_dbg_phy_erase(param0, param1);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "erase2") == 0) {
		nand_dbg_err("nand erase2 cmd:\n");
		nand_dbg_zone_erase(nand_kobj->nftl_blk->nftl_zone, param0,
				    param1);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "erase3") == 0) {
		nand_dbg_err("nand erase3 cmd:\n");
		nand_dbg_single_phy_erase(param0, param1);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "write1") == 0) {
		nand_dbg_err("nand read1 cmd:\n");
		nand_dbg_phy_write(param0, param1, param2);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "write2") == 0) {
		nand_dbg_err("nand read2 cmd:\n");
		nand_dbg_zone_phy_write(nand_kobj->nftl_blk->nftl_zone, param0,
					param1);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "checktable") == 0) {
		nand_dbg_err("nand read2 cmd:\n");

		mutex_lock(nand_kobj->nftl_blk->blk_lock);
		nand_check_table(nand_kobj->nftl_blk->nftl_zone);
		mutex_unlock(nand_kobj->nftl_blk->blk_lock);

		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "testspeed") == 0) {
		nand_dbg_err("nand runtest cmd:\n");
		udisk_test_speed(nand_kobj->nftl_blk);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "teststart") == 0) {
		nand_dbg_err("nand runtest cmd:\n");
		udisk_test_start(nand_kobj->nftl_blk);
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "teststop") == 0) {
		nand_dbg_err("nand runtest cmd:\n");
		udisk_test_stop();
		goto NAND_TEST_STORE_EXIT;
	} else if (strcmp(cmd, "readdev") == 0) {
		nand_dbg_err("nand dev read\n");
		if (param1 > 16) {
			nand_dbg_err("max len is 16!\n");
			goto NAND_TEST_STORE_EXIT;
		}
		mutex_lock(nand_kobj->nftl_blk->blk_lock);

		tempbuf = kmalloc(8192, GFP_KERNEL);
		_dev_nand_read2(param3, param0, param1, tempbuf);
		for (i = 0; i < (param1 << 9); i += 4) {
			nand_dbg_inf("%8x ", *((int *)&tempbuf[i]));
			if (((i + 4) % 64) == 0)
				nand_dbg_inf("\n");

		}
		kfree(tempbuf);

		mutex_unlock(nand_kobj->nftl_blk->blk_lock);

		goto NAND_TEST_STORE_EXIT;
	} else {
		nand_dbg_err("err, nand debug undefined cmd: %s\n", cmd);
	}

NAND_TEST_STORE_EXIT:
	return count;
}

/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
void udisk_test_speed(struct _nftl_blk *nftl_blk)
{
	char *test_buf;
	struct timeval tpstart, tpend;
	unsigned int start_sector;
	unsigned int len;
	unsigned int i;
	unsigned int sec;
	int usec;

	test_buf = kmalloc(0x100000, GFP_KERNEL);

	do_gettimeofday(&tpstart);

	start_sector = 0x32000;
	len = 0x800;
	for (i = 0; i < 500; i++) {
		mutex_lock(nftl_blk->blk_lock);
		_dev_nand_write2("UDISK", start_sector + i * len, len,
				 test_buf);
		mutex_unlock(nftl_blk->blk_lock);
	}

	do_gettimeofday(&tpend);

	sec = tpend.tv_sec - tpstart.tv_sec;
	usec = tpend.tv_usec - tpstart.tv_usec;

	nand_dbg_inf("write sec:%d  usec:%d\n", sec, usec);

	do_gettimeofday(&tpstart);

	start_sector = 0x32000;
	len = 0x800;
	for (i = 0; i < 500; i++) {
		mutex_lock(nftl_blk->blk_lock);
		_dev_nand_read2("UDISK", start_sector + i * len, len, test_buf);
		mutex_unlock(nftl_blk->blk_lock);
	}

	do_gettimeofday(&tpend);

	sec = tpend.tv_sec - tpstart.tv_sec;
	usec = tpend.tv_usec - tpstart.tv_usec;

	nand_dbg_inf("read sec:%d  usec:%d\n", sec, usec);

	kfree(test_buf);
}
