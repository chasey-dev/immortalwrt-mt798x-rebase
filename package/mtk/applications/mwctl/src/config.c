/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

#define CONFIG_HELP "config settings"
#define CONFIG_OPTIONS "<param> <value>"

static int handle_config_command(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	return 0;
}
TOPLEVEL(config,
	CONFIG_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_CONFIG, 0, CIB_NETDEV, handle_config_command,
	CONFIG_HELP);


#define CONFIG_FWDL_HELP "config interface fwdl cmd_string\n"
#define CONFIG_FWDL_OPTIONS "config interface fwdl cmd_string\n"\
	"      cmd_string: download, re-download, remove\n"

int handle_config_fwdl_action(struct nl_msg *msg, int argc,
				char **argv, void *ctx)
{
	void *data;
	size_t len = 0;
	char *cmd_str;

	if (!argc) {
		printf("      %s\n", CONFIG_FWDL_OPTIONS);
		return -EINVAL;
	}

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (argc > 0) {
		cmd_str = argv[0];

		len = strlen(cmd_str);
		if (len) {
			if (nla_put_string(
				msg, MTK_NL80211_VENDOR_ATTR_CONFIG_FWDL_ACTION, cmd_str)) {
				return -EMSGSIZE;
			}
		}
	}

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(config, fwdl,
	CONFIG_FWDL_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_CONFIG, 0, CIB_NETDEV, handle_config_fwdl_action,
	CONFIG_FWDL_HELP);


int handle_config_mac_addr(struct nl_msg *msg, int argc,
				char **argv, void *ctx)
{
	char *token;
	unsigned int i = 0, valid = 1;
	u8 input_data[1+ETH_ALEN];
	void *data;
	char *value;

	if (!argc || argc != 2)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	value = argv[0];
	input_data[0] = strtol(value, NULL, 10);

	value = argv[1];

	i = 1;
	for (token = strtok(value, ":,"); token != NULL; token = strtok(NULL, ":,")) {
		if (strlen(token) != 2 || i > ETH_ALEN) {
			valid = 0;
			break;
		}
		input_data[i] = strtol(token, NULL, 16);
		i++;
	}

	if (!valid) {
		return -EINVAL;
	}

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_CONFIG_MAC_ADDR, 1+ETH_ALEN, input_data)) {
		return -EMSGSIZE;
	}

	nla_nest_end(msg, data);
	return 0;
}

#define CONFIG_MAC_ADDR_HELP "config interface mac address\n"
#define CONFIG_MAC_ADDR_OPTIONS "0 xx:xx:xx:xx:xx:xx\n"
COMMAND(config, mac_addr,
	CONFIG_MAC_ADDR_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_CONFIG, 0, CIB_NETDEV, handle_config_mac_addr,
	CONFIG_MAC_ADDR_HELP);

#define CONFIG_SET_DEVICE_OPTIONS "E2pAccessMode=<1/2/3/4>\n"\
		"                1: efuse mode\n"\
		"                2: flash mode\n"\
		"                3: eeprom mode\n"\
		"                4: bin mode\n\n"\
		"         NPU_EN=<0/1>\n"\
		"                0: disable\n"\
		"                1: enable\n\n"\
		"         LocalMaxTxPwrBW20=value <ex. 47 --> 23.5dB>\n"\
		"         LocalMaxTxPwrBW40=value\n"\
		"         LocalMaxTxPwrBW80=value\n"\
		"         LocalMaxTxPwrBW160=value\n"\
		"         LocalMaxTxPwrBW320=value\n"\
		"                <Range -64 dBm to 63 dBm with a 0.5 dB step.>\n\n"\
		"         WHNAT=<0/1>\n"\
		"                0: disable\n"\
		"                1: enable\n\n"\
		"         RxEnhanceEn=<0/1>\n"\
		"                0: disable\n"\
		"                1: enable\n\n"\
		"         ThreeWireFunctionEnable=<0/1>\n"\
		"                0: disable\n"\
		"                1: enable\n\n"\
		"         RED_Enable=<0/1>\n"\
		"                0: disable\n"\
		"                1: enable\n\n"\
		"         MloStrBitMap=<0x7/0x3>\n"\
		"         MloSyncTx=<0/1>\n"\
		"                0: disable\n"\
		"                1: enable\n\n"\
		"         MloV1=<0/1>\n"\
		"                0: disable\n"\
		"                1: enable\n\n"\
		"         WtblDupNum=<0 ~ 80>\n"\
		"         TestModeEn=<0/1>\n"\
		"                0: disable\n"\
		"                1: enable\n\n"\
		"         MacAddrAdj=<0/1>\n"\
		"                0: disable\n"\
		"                1: enable\n\n"\
		"         Dot11vHostDupBcn=<0/1>\n"\
		"                0: disable\n"\
		"                1: enable\n\n"\
		"         SwStaEnable=<0/1>\n"\
		"                0: disable\n"\
		"                1: enable\n\n"\
		"         EDCCARegion=<0/1/2/3/4>\n"\
		"                0: Default\n"\
		"                1: FCC\n"\
		"                2: ETSI_2017\n"\
		"                3: JAPAN\n"\
		"                4: ETSI_2023\n"\
		"         RROSupport=<0/1>\n"\
		"                0: disable\n"\
		"                1: enable\n\n"

int handle_config_set_device(struct nl_msg *msg, int argc,
				char **argv, void *ctx)
{
	void *data;
	size_t len = 0;
	char *cmd_str;

	if (!argc) {
		printf("         %s\n", CONFIG_SET_DEVICE_OPTIONS);
		return -EINVAL;
	}

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (argc > 0) {
		cmd_str = argv[0];

		len = strlen(cmd_str);
		if (len) {
			if (nla_put_string(
				msg, MTK_NL80211_VENDOR_ATTR_CONFIG_SET_DEVICE_CMD_STR, cmd_str)) {
				return -EMSGSIZE;
			}
		}
	}

	nla_nest_end(msg, data);
	return 0;
}

#define CONFIG_SET_HELP "config set_device commands\n"
COMMAND(config, set_device,
	CONFIG_SET_DEVICE_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_CONFIG, 0, CIB_NETDEV, handle_config_set_device,
	CONFIG_SET_HELP);


#define CONFIG_SET_BAND_OPTIONS "WifiCert=<0/1>\n"\
		"      TgnWMMCert=<0/1>\n"\
		"      CountryRegion=<0~7>\n"\
		"             Region for 2.4G\n"\
		"      CountryRegionABand=<0~37>\n"\
		"             Region for 5G/6G\n"\
		"      FreqList=<val-val-val-...>\n"\
		"             val: frequency. ex. 2412\n"\
		"      CountryCode=<AL/US/TW/...>\n"\
		"      DfsCalibration=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      DfsEnable=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      Band4DfsEnable=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      DfsUseCsaCfg=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      DfsRCSAEn=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      DfsZeroWaitCacTime=<value>\n"\
		"             Unit is minute\n"\
		"      OcacEnable=<0/1/2>\n"\
		"             0: Off Channel CAC Disable\n"\
		"             1: Off Channel CAC Non-Channel Switch\n"\
		"             2: Off Channel CAC Channel Switch\n"\
		"      DisableHostapd=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      AutoChannelSelect=<1/2/3/8/9>\n"\
		"      AutoChannelSkipList=<channel list>\n"\
		"             ex. 2;3;4;5;7\n"\
		"      ACSScanMode=<0/1>\n"\
		"             0: partial scan off\n"\
		"             1: partial scan on\n"\
		"      ACSScanDwell=<value>\n"\
		"      ACSRestoreDwell=<value>\n"\
		"      ACSPartialScanChNum=<value>\n"\
		"      ACSChUtilThr=<0 ~ 255>\n"\
		"      ACSIceChUtilThr=<0 ~ 255>\n"\
		"      ACSMaxACSTimes=<0 ~ 255>\n"\
		"      ACSStaNumThr=<value>\n"\
		"      ACSDataRateWt=<0 ~ 255>\n"\
		"      ACSPrioWt=<0 ~ 255>\n"\
		"      ACSTxPowerCons=<value>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      ACSSwChThr=<0 ~ 255>\n"\
		"      ACSBgndScan=<value>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      ACSBgndScanStopTxThr=<0 ~ 10>\n"\
		"      ACSBgndScanAffectTxThr=<0 ~ 10>\n"\
		"      PSC_ACS=<value>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      obss_wt=<0 ~ 255>\n"\
		"      AfcDeviceType=<0/1/2>\n"\
		"      AfcSpBwDup=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      AfcFreqrange=<frequency>\n"\
		"      ACSAfterAFC=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      AfcSpectrumType=<value>\n"\
		"      AfcOpClass131=<channel list>\n"\
		"      AfcOpClass132=<channel list>\n"\
		"      AfcOpClass133=<channel list>\n"\
		"      AfcOpClass134=<channel list>\n"\
		"      AfcOpClass135=<channel list>\n"\
		"      AfcOpClass136=<channel list>\n"\
		"      AfcOpClass137=<channel list>\n"\
		"      SeamlessCSA=<0/1>\n"\
		"             0: 11H disable\n"\
		"             1: 11H enable\n"\
		"      BasicRate=<value>\n"\
		"      GNMixMode=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      BeaconPeriod=<value>\n"\
		"             20~1000 ms\n"\
		"      BandSteering=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      TxPower=<vale>\n"\
		"             0~100 %\n"\
		"      SKUenable=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      BFBACKOFFenable=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      SkuTableIdx=<value>\n"\
		"      PsuSkuEnable=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      PsuSkuTableIdx=<value>\n"\
		"      PERCENTAGEenable=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      BGProtection=<0/1/2>\n"\
		"      DisableOLBC=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      TxPreamble=<0/1/2>\n"\
		"      VLANEn=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      TxBurst=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      NoForwardingBTNBSSID=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      DfsZeroWait=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      DfsDedicatedZeroWait=<value>\n"\
		"      DfsZeroWaitDefault=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      DfsZeroWaitBandidx=<0/1/2>\n"\
		"      DfsTargetCh=<value>\n"\
		"      DfsChSelPrefer=<value>\n"\
		"      DfsNopExpireSetChPolicy=<value>\n"\
		"      DfsDedicatedRxPreselectCh=<value>\n"\
		"      EDCCAEnable=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      ACSCheckTime=<value>\n"\
		"      ACSCheckMinTime=<value>\n"\
		"      ShortSlot=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      RDRegion=<str>\n"\
		"      QoSEnable=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      SE_OFF=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      BcnDataRate=<value>\n"\
		"      BSSColorValue=<value>\n"\
		"      Wifi6GCap=<value>\n"\
		"      LPIEnable=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      MLREnable=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      IdcState=<0/1>\n"\
		"             0: disable\n"\
		"             1: enable\n"\
		"      Single_RNR=<value>\n"\
		"      Channel_priority=<value>\n"\
		"      Forbid_BW40=<value>\n"

int handle_config_set_band(struct nl_msg *msg, int argc,
				char **argv, void *ctx)
{
	void *data;
	size_t len = 0;
	char *cmd_str;

	if (argc <= 1) {
		printf("      %s\n", CONFIG_SET_BAND_OPTIONS);
		return -EINVAL;
	}

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (argc > 1) {
		cmd_str = argv[1];

		len = strlen(cmd_str);
		if (len) {
			if (nla_put_string(
				msg, MTK_NL80211_VENDOR_ATTR_CONFIG_SET_BAND_CMD_STR, cmd_str)) {
				return -EMSGSIZE;
			}
		}
	}

	nla_nest_end(msg, data);
	return 0;
}

#define CONFIG_SET_BAND_HELP "config set_band commands\n"
COMMAND(config, set_band,
	CONFIG_SET_BAND_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_CONFIG, 0, CIB_NETDEV, handle_config_set_band,
	CONFIG_SET_BAND_HELP);


