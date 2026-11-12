#ifndef __MT_WIFI_EEPROM_H
#define __MT_WIFI_EEPROM_H

#include <linux/types.h>

int mt_mtd_read_nm_wifi(char *name, loff_t from, size_t len, u_char *buf);
int mt_mtd_write_nm_wifi(char *name, loff_t to, size_t len, const u_char *buf);

int mt_of_read_eeprom_wifi(loff_t from, size_t len, u_char *buf);

int mt_eeprom_read_wifi(loff_t from, size_t len, u_char *buf);
int mt_eeprom_write_wifi(loff_t to, size_t len, const u_char *buf);

#endif
