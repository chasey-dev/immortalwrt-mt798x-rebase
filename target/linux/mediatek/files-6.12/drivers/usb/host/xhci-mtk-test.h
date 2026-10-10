/* SPDX-License-Identifier: GPL-2.0 AND GPL-2.0-only */
/*
 * xhci-mtk-unusuallib.h -- xhci toolkit header file
 *
 * Copyright (C) 2021 Mediatek Inc - http://www.mediatek.com
 * Author:
 *  Zhanyong Wang <zhanyong.wang@mediatek.com>
 */


#ifndef __XHCI_MTK_TEST_H
#define __XHCI_MTK_TEST_H

#if IS_ENABLED(CONFIG_USB_XHCI_MTK_DEBUGFS)
int hqa_create_attr(struct device *dev);
void hqa_remove_attr(struct device *dev);
#else
static inline int hqa_create_attr(struct device *dev)
{
	return 0;
}
static inline void hqa_remove_attr(struct device *dev)
{
}
#endif
#endif /* __XHCI_MTK_TEST_H */
