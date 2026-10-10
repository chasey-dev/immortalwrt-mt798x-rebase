#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <l1parser.h>

#include "iwinfo.h"
#include "mtwifi.h"

static int mtk_dev_match_id(const char* chip, struct iwinfo_hardware_id *id)
{
	if (!strcmp(chip, "MT7981")) {
		id->vendor_id = 0x14c3;
		id->device_id = 0x7981;
		id->subsystem_vendor_id = id->vendor_id;
		id->subsystem_device_id = id->device_id;
	} else if (!strcmp(chip, "MT7986")) {
		id->vendor_id = 0x14c3;
		id->device_id = 0x7986;
		id->subsystem_vendor_id = id->vendor_id;
		id->subsystem_device_id = id->device_id;
	} else if (!strcmp(chip, "MT7916")) {
		id->vendor_id = 0x14c3;
		id->device_id = 0x7916;
		id->subsystem_vendor_id = id->vendor_id;
		id->subsystem_device_id = id->device_id;
	} else {
		return -1;
	}

	return 0;
}

int mtk_get_id_by_l1util(const char *dev, struct iwinfo_hardware_id *id)
{
	L1Context *ctx = l1_init();
	if (!ctx)
		return -1;

	char *chip_id = NULL;

	/* Determine if 'dev' is an interface name or an internal device reference */
	if (strstr(dev, "ra") || strstr(dev, "apcli") || strstr(dev, "wds") || strstr(dev, "mesh"))
		chip_id = l1_get_chip_id_by_ifname(ctx, dev);
	else
		chip_id = l1_get_chip_id_by_devname(ctx, dev);

	int ret = -1;
	if (chip_id) {
		ret = mtk_dev_match_id(chip_id, id);
		free(chip_id);
	}

	l1_free(ctx);
	return ret;
}

int mtk_get_id_from_l1profile(struct iwinfo_hardware_id *id)
{
	L1Context *ctx = l1_init();
	if (!ctx)
		return -1;

	size_t count = 0;
	char **list = l1_list(ctx, &count);

	if (!list || count == 0) {
        if (list) l1_free_str_array(list, count);
        l1_free(ctx);
        return -1;
    }

	/* 
	 * Check for ambiguity: if multiple chipsets are defined (MainIdx >= 2),
	 * the keys will contain "_2_". If found, we cannot safely guess the ID.
	 */
	for (size_t i = 0; i < count; i++) {
		if (strstr(list[i], "_2_")) {
			l1_free_str_array(list, count);
			l1_free(ctx);
			return -1;
		}
	}

	int ret = -1;
	/* 
	 * Extract the first token (e.g., "MT7981_1_1") and isolate the chip name.
	 * Since we verified there is only one MainIdx (1), any entry will suffice.
	 */
	if (count > 0 && list[0]) {
		char *token = strdup(list[0]);
		if (token) {
			char *p = strchr(token, '_');
			if (p) {
				*p = '\0'; /* Terminate string at the first underscore */
				ret = mtk_dev_match_id(token, id);
			}
		}
	}

	free(list);
	l1_free(ctx);
	return ret;
}