#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include "mt_wifi_eeprom.h"

int mt_eeprom_read_wifi(loff_t from, size_t len, u_char *buf)
{
	int ret;

	if (!buf && len)
		return -EINVAL;

	ret = mt_of_read_eeprom_wifi(from, len, buf);
	if (!ret)
		return 0;

	pr_warn("%s: read eeprom from OF failed from=0x%llx len=0x%zx ret=%d\n",
		__func__, (unsigned long long)from, len, ret);

	ret = mt_mtd_read_nm_wifi("Factory", from, len, buf);
	if (!ret) {
		pr_info("%s: read MTD Factory from=0x%llx len=0x%zx\n",
			__func__, (unsigned long long)from, len);
		return 0;
	}

	pr_warn("%s: read MTD Factory failed from=0x%llx len=0x%zx ret=%d\n",
		__func__, (unsigned long long)from, len, ret);

	ret = mt_mtd_read_nm_wifi("factory", from, len, buf);
	if (!ret) {
		pr_info("%s: read MTD factory from=0x%llx len=0x%zx\n",
			__func__, (unsigned long long)from, len);
		return 0;
	}

	pr_err("%s: failed from=0x%llx len=0x%zx",
	       __func__, (unsigned long long)from, len);

	return ret;
}
EXPORT_SYMBOL(mt_eeprom_read_wifi);

int mt_eeprom_write_wifi(loff_t to, size_t len, const u_char *buf)
{
	int ret;

	if (!buf && len)
		return -EINVAL;

	ret = mt_mtd_write_nm_wifi("Factory", to, len, buf);
	if (!ret) {
		pr_info("%s: wrote MTD Factory to=%lld len=%zu\n",
			__func__, (long long)to, len);
		return 0;
	}

	pr_warn("%s: write MTD Factory failed to=%lld len=%zu ret=%d\n",
		__func__, (long long)to, len, ret);

	ret = mt_mtd_write_nm_wifi("factory", to, len, buf);
	if (!ret) {
		pr_info("%s: wrote MTD factory to=%lld len=%zu\n",
			__func__, (long long)to, len);
		return 0;
	}

	pr_err("%s: failed to=%lld len=%zu",
	       __func__, (long long)to, len);

	return ret;
}
EXPORT_SYMBOL(mt_eeprom_write_wifi);
