#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/concat.h>
#include <linux/mtd/partitions.h>
#include "mt_wifi_eeprom.h"
#ifdef CONFIG_MIPS
#include <asm/addrspace.h>
#endif

int mt_mtd_write_nm_wifi(char *name, loff_t to, size_t len, const u_char *buf)
{
	int ret;
	size_t rdlen, wrlen;
	struct mtd_info *mtd;
	struct erase_info ei = {};
	u_char *bak = NULL;

	if (to < 0)
		return -EINVAL;

	mtd = get_mtd_device_nm(name);
	if (IS_ERR(mtd))
		return PTR_ERR(mtd);

	if (to > mtd->erasesize || len > mtd->erasesize - to) {
		ret = -EINVAL;
		goto out_put_mtd;
	}

	bak = kmalloc(mtd->erasesize, GFP_KERNEL);
	if (!bak) {
		ret = -ENOMEM;
		goto out_put_mtd;
	}

	ret = mtd_read(mtd, 0, mtd->erasesize, &rdlen, bak);
	if (mtd_is_bitflip(ret))
		ret = 0;

	if (ret != 0)
		goto out_free_bak;

	if (rdlen != mtd->erasesize) {
		pr_warn("ra_mtd_write: rdlen is not equal to erasesize\n");
		ret = -EIO;
		goto out_free_bak;
	}

	memcpy(bak + (size_t)to, buf, len);

	ei.addr = 0;
	ei.len = mtd->erasesize;
	ret = mtd_erase(mtd, &ei);

	if (ret != 0)
		goto out_free_bak;

	ret = mtd_write(mtd, 0, mtd->erasesize, &wrlen, bak);
	if (ret != 0)
		goto out_free_bak;
	if (wrlen != mtd->erasesize) {
		pr_warn("ra_mtd_write: wrlen is not equal to erasesize\n");
		ret = -EIO;
	}

out_free_bak:
	kfree(bak);
out_put_mtd:
	put_mtd_device(mtd);
	return ret;
}
EXPORT_SYMBOL(mt_mtd_write_nm_wifi);

int mt_mtd_read_nm_wifi(char *name, loff_t from, size_t len, u_char *buf)
{
	int ret;
	size_t rdlen;
	struct mtd_info *mtd;

	if (from < 0)
		return -EINVAL;

	mtd = get_mtd_device_nm(name);
	if (IS_ERR(mtd))
		return PTR_ERR(mtd);

	ret = mtd_read(mtd, from, len, &rdlen, buf);
	if (mtd_is_bitflip(ret))
		ret = 0;

	if (ret == 0 && rdlen != len) {
		pr_warn("ra_mtd_read_nm: rdlen is not equal to len\n");
		ret = -EIO;
	}

	put_mtd_device(mtd);

	return ret;
}
EXPORT_SYMBOL(mt_mtd_read_nm_wifi);
