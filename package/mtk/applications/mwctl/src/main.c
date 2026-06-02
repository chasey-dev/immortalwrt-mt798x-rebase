/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include <net/if.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "iwpriv_compat.h"
#include "mwctl.h"

static const char *progname;
long g_band_link_id = MT_MLD_MAX_NUM_LINKS;

struct unl unl;

int (*registered_handler)(struct nl_msg *, void *);
void *registered_handler_data;

void register_handler(int (*handler)(struct nl_msg *, void *), void *data)
{
	registered_handler = handler;
	registered_handler_data = data;
}

int valid_handler(struct nl_msg *msg, void *arg)
{
	if (registered_handler)
		return registered_handler(msg, registered_handler_data);

	return NL_OK;
}

extern struct cmd *__start___cmd[];
extern struct cmd *__stop___cmd;

#define for_each_cmd(_cmd, i)					\
	for (i = 0; i < &__stop___cmd - __start___cmd; i++)	\
		if ((_cmd = __start___cmd[i]))

static int print_help(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	exit(3);
}
TOPLEVEL(help, "[command]", 0, 0, CIB_NONE, print_help,
	 "Print usage for all or a specific command, e.g.\n"
	 "\"help set\" or \"help set ap_vow\".");


void __usage_cmd(const struct cmd *cmd, char *indent, bool full)
{
	const char *start, *lend, *end;

	printf("%s", indent);

	switch (cmd->idby) {
	case CIB_NONE:
		break;
	case CIB_PHY:
		printf("phy <phyname> ");
		break;
	case CIB_NETDEV:
		printf("dev <devname> ");
		break;
	case CIB_WDEV:
		printf("wdev <idx> ");
		break;
	}
	if (cmd->parent && cmd->parent->name)
		printf("%s ", cmd->parent->name);
	printf("%s", cmd->name);

	if (cmd->args) {
		/* print line by line */
		start = cmd->args;
		end = strchr(start, '\0');
		printf(" ");
		do {
			lend = strchr(start, '\n');
			if (!lend)
				lend = end;
			if (start != cmd->args) {
				printf("\t\t\t\t");
			}
			printf("%.*s\n", (int)(lend - start), start);
			start = lend + 1;
		} while (end != lend);
	} else
		printf("\n");

	if (!full || !cmd->help)
		return;

	/* hack */
	if (strlen(indent))
		indent = "\t\t";
	else
		printf("\n");

	/* print line by line */
	start = cmd->help;
	end = strchr(start, '\0');

	do {
		lend = strchr(start, '\n');
		if (!lend)
			lend = end;
		printf("%s", indent);
		printf("%.*s\n", (int)(lend - start), start);
		start = lend + 1;
	} while (end != lend);

	printf("\n");
}

static void usage(int argc, char **argv)
{
	const struct cmd *section, *cmd;
	bool full = argc >= 0;
	const char *sect_filt = NULL;
	const char *cmd_filt = NULL;
	unsigned int i, j;

	if (argc > 0)
		sect_filt = argv[0];

	if (argc > 1)
		cmd_filt = argv[1];

	printf("Usage:\tmwctl command\n");
	printf("Commands:\n");
	for_each_cmd(section, i) {
		if (section->parent)
			continue;

		if (sect_filt && strcmp(section->name, sect_filt))
			continue;

		if (section->handler && !section->hidden)
			__usage_cmd(section, "\t", full);

		for_each_cmd(cmd, j) {
			if (section != cmd->parent)
				continue;
			if (!cmd->handler || cmd->hidden)
				continue;
			if (cmd_filt && strcmp(cmd->name, cmd_filt))
				continue;
			__usage_cmd(cmd, "\t", full);
		}
	}
	printf("\nYou can omit the 'phy' or 'dev' if "
			"the identification is unique,\n"
			"e.g. \"mwctl ra0 set\" or \"mwctl phy0 set\". "
			"(Don't when scripting.)\n\n"
			"Do NOT screenscrape this tool, we don't "
			"consider its output stable.\n\n");
}

struct cmd *get_cmd_by_sect_name(char *sect, char *name)
{
	struct cmd *section, *cmd;
	unsigned int i, j;

	for_each_cmd(section, i) {
		if (section->parent)
			continue;

		if (sect && strcmp(section->name, sect))
			continue;

		for_each_cmd(cmd, j) {
			if (section != cmd->parent)
				continue;
			if (!cmd->handler || cmd->hidden)
				continue;
			if (name && strcmp(cmd->name, name))
				continue;
			return cmd;
		}
	}

	return NULL;
}

SECTION(dump);

static int phy_lookup(char *name)
{
	char buf[200];
	int fd, pos;
	int ret;
	long val = 0;

	ret = snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/index", name);
	if (ret < 0 || ret >= sizeof(buf))
		return -1;

	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return -1;
	pos = read(fd, buf, sizeof(buf) - 1);
	if (pos < 0) {
		close(fd);
		return -1;
	}
	buf[pos] = '\0';
	close(fd);
	val = strtol(buf, NULL, 10);

	/* To fix overflowed return value (INTEGER_OVERFLOW) */
	if (val < 0 || val > 65535)
		return -1;
	else
		return (int)val;
}

static int dev_lookup(int argc, char **argv)
{
	char buf[200] = {0};
	int fd, pos;
	int ret;

	if (strncmp(argv[0], "dev", 3) == 0 && argc > 2) {
		if (strcmp(argv[1], "config") != 0)
			return 0;
		if (strcmp(argv[2], "set_device") == 0
			|| strcmp(argv[2], "fwdl") == 0)
			ret = snprintf(buf, sizeof(buf),
				"/sys/kernel/mtk_wifi/%s_main", argv[0]);
		else if (strcmp(argv[2], "set_band") == 0 && argc > 3) {
			ret = snprintf(buf, sizeof(buf),
				"/sys/kernel/mtk_wifi/%s_band%s", argv[0], argv[3]);
		} else
			return 0;

		if (ret < 0 || ret >= sizeof(buf))
			return 0;

		fd = open(buf, O_RDONLY);
		if (fd < 0)
			return 0;

		memset(buf, 0, sizeof(buf));
		pos = read(fd, buf, sizeof(buf) - 1);
		if (pos <= 0) {
			close(fd);
			return 0;
		}
		buf[pos-1] = '\0';
		close(fd);

		return if_nametoindex(buf);
	} else
		return 0;
}

int add_msg_band_link_id(struct nl_msg *msg, long band_link_id, u8 msg_link_id)
{
	if (VALID_MLD_LINK_ID(band_link_id)) {
		if (nla_put_u8(msg, msg_link_id, (u8)band_link_id)) {
			printf("nla_put_u8 link_id fails.\n");
			return -EMSGSIZE;
		}
	}
	return 0;
}

int set_band_link_id(int argc, char **argv)
{
	int offset = 0;
	char *endptr = NULL;
	long band_link_id = MT_MLD_MAX_NUM_LINKS;

	if (argc >= 2 && !strcmp(argv[0], "-l")) {
		errno = 0;
		band_link_id = strtol(argv[1], &endptr, 10);
		if (errno != 0 || endptr == argv[1] || *endptr != '\0') {
            printf("Conversion error occurred for band link id\n");
            return offset;
        }
		if (band_link_id >= MT_MLD_MAX_NUM_LINKS) {
			printf("band link id %ld exceeds max number of links\n", band_link_id);
			return offset;
		}
		g_band_link_id = band_link_id;
		offset = 2;
	}
	return offset;
}

long get_band_link_id(void) {
	return g_band_link_id;
}

int main(int argc, char **argv)
{
	int if_idx, ret = 0, offset = 0;
	const struct cmd *cmd, *match = NULL, *sectcmd;
	const char *command, *section;
	enum command_identify_by command_idby = CIB_NONE;
	int i;
	struct nl_msg *msg;

	progname = argv[0];

	if (argc == 1 || (argc >= 2 && !strcmp(argv[1], "help"))) {
		usage(argc - 2, argv + 2);
		return 0;
	}

	argc--;
	argv++;

	if (strcmp(*argv, "dev") == 0 && argc > 1) {
		argc--;
		argv++;
		if_idx = if_nametoindex(argv[0]);
		if (!if_idx) {
			(void)fprintf(stderr, "Invalid arg: errno=%d(%s)\n", errno, strerror(errno));
			return 1;
		}
		command_idby = CIB_NETDEV;
		argc--;
		argv++;
	} else if (strcmp(*argv, "phy") == 0 && argc > 1) {
		argc--;
		argv++;
		if ((if_idx = phy_lookup(argv[0])) < 0) {
			(void)fprintf(stderr, "Invalid arg: errno=%d(%s)\n", errno, strerror(errno));
			return 1;
		}
		command_idby = CIB_PHY;
		argc--;
		argv++;
	} else {
		if ((if_idx = if_nametoindex(argv[0])) != 0)
			command_idby = CIB_NETDEV;
		else if ((if_idx = phy_lookup(argv[0])) >= 0)
			command_idby = CIB_PHY;
		else {
			if_idx = dev_lookup(argc, argv);
			if (!if_idx) {
				(void)fprintf(stderr, "Invalid arg: errno=%d(%s)\n",
					errno, strerror(errno));
				return 1;
			}
			command_idby = CIB_NETDEV;
		}
		argc--;
		argv++;
	}

	if (argc <= 0) {
		(void)fprintf(stderr, "Incomplete arguments\n");
		return 1;
	}

	offset = set_band_link_id(argc, argv);
	argc -= offset;
	argv += offset;

	section = *argv;
	argc--;
	argv++;

	for_each_cmd(sectcmd, i) {
		if (sectcmd->parent)
			continue;
		/* ok ... bit of a hack for the dupe 'info' section */
		if (match && sectcmd->idby != command_idby)
			continue;
		if (strcmp(sectcmd->name, section) == 0)
			match = sectcmd;
	}

	sectcmd = match;
	match = NULL;
	if (!sectcmd) {
		ret = -ENOTSUP;
		(void)fprintf(stderr, "Command not support: errno=%d(%s)\n", ret, strerror(ENOTSUP));
		return 1;
	}

	if (argc > 0) {
		command = *argv;

		for_each_cmd(cmd, i) {
			if (!cmd->handler)
				continue;
			if (cmd->parent != sectcmd)
				continue;
			/*
			 * ignore mismatch id by, but allow WDEV
			 * in place of NETDEV
			 */
			if (cmd->idby != command_idby &&
			    !(cmd->idby == CIB_NETDEV &&
			      command_idby == CIB_WDEV))
				continue;
			if (strcmp(cmd->name, command))
				continue;
			if (argc > 1 && !cmd->args)
				continue;
			match = cmd;
			break;
		}

		if (match) {
			argc--;
			argv++;
		}
	}

	if (match)
		cmd = match;
	else {
		/* Use the section itself, if possible. */
		cmd = sectcmd;
		if (argc && !cmd->args){
			(void)fprintf(stderr, "Args are not allowed!\n");
			return 1;
		}
		if (cmd->idby != command_idby &&
			!(cmd->idby == CIB_NETDEV && command_idby == CIB_WDEV)) {
			(void)fprintf(stderr, "Second parameter is invalide\n");
			return 1;
		}
		if (!cmd->handler) {
			(void)fprintf(stderr, "No handler for current command\n");
			return 1;
		}
	}

	if (unl_genl_init(&unl, "nl80211") < 0) {
		(void)fprintf(stderr, "Failed to connect to nl80211\n");
		return 1;
	}

	msg = unl_genl_msg(&unl, NL80211_CMD_VENDOR, false);

	if (nla_put_u32(msg, command_idby == CIB_NETDEV ? NL80211_ATTR_IFINDEX : NL80211_ATTR_WIPHY, if_idx) ||
	    nla_put_u32(msg, NL80211_ATTR_VENDOR_ID, MTK_NL80211_VENDOR_ID) ||
	    nla_put_u32(msg, NL80211_ATTR_VENDOR_SUBCMD, cmd->cmd)) {
		nlmsg_free(msg);
		(void)fprintf(stderr, "Nla put error\n");
		return 1;
	}

	ret = cmd->handler(msg, argc, argv, (void*)&if_idx);
	if (ret) {
		nlmsg_free(msg);
		goto out;
	}

	ret = unl_genl_request(&unl, msg, valid_handler, NULL);
out:
	if (ret)
		(void)fprintf(stderr, "Command failed: %s (%d)\n", strerror(-ret), ret);

	unl_free(&unl);

	return ret;
}
