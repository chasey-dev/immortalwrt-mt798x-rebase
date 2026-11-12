# MTK HNAT：`skb->mark` 位布局与队列掩码定义

适用于 `target/linux/mediatek`（kernel 6.12，`mtk-openwrt-feeds` a50e2e2c45 / 2026-10-13）。

> 编写缘由：short-pkt-qos 修复时发现，判定"流量是否已显式指定队列"完全依赖 `skb->mark` 的位布局，
> 而这层布局在 MTK feeds 里被反复改写。本文档把偏移与定义的**最终落值**钉死，便于日后核对。
> 维护 chips：mt7981 / mt7986（netsys v2）、mt7988（netsys v3）。

---

## 1. `skb->mark` 位布局（最终确认值）

```
  bit   7        6   5   4   3   2   1   0
      ┌───┬───────────────────────────────┐
      │ E │           QDMA queue id       │
      └───┴───────────────────────────────┘
        │            │                 │
        │            └─ netsys v2/v3: bit 0..6  (mask 0x7f)
        │                netsys v1.x : bit 0..3  (mask 0x0f)
        │
        └─ HNAT_EXCEPTION_BIT (BIT(7) = 0x80)
           置位 = 该 flow 不做硬件加速
```

**来源**：`drivers/net/ethernet/mediatek/mtk_hnat/hnat.h:279`

```c
/* The user prevents hardware acceleration for a packet flow by setting
 * HNAT_EXCEPTION_BIT in skb->mark. The rest of the mark is left alone, so
 * it can still carry the QDMA queue id in the lower bits.
 */
#define HNAT_EXCEPTION_BIT	BIT(7)
```

**队列位宽**：`hnat_nf_hook.c` 里 HQoS 回填队列时已有权威口径（第 2937 行）：

```c
entry.ipv4_hnapt.iblk2.qid =
	(hnat_priv->data->version == MTK_HNAT_V2 ||
	 hnat_priv->data->version == MTK_HNAT_V3) ?
		 skb->mark & 0x7f : skb->mark & 0xf;
```

即：**netsys v2/v3 取 bit0..6，v1.x 取 bit0..3**。任何自研判定都应与此一致。

**NETSYS 版本枚举**：`hnat.h:989`

```c
enum mtk_hnat_version {
	MTK_HNAT_V1_1 = 1,	/* mt7621, mt7623 */
	MTK_HNAT_V1_2,		/* mt7622        */
	MTK_HNAT_V1_3,		/* mt7629        */
	MTK_HNAT_V2,		/* mt7981, mt7986 */
	MTK_HNAT_V3,		/* mt7988        */
};
```

运行时取值：`hnat_priv->data->version`。

---

## 2. `MTK_QDMA_QUEUE_MASK` 的演变 —— 为什么不能用它

该宏在同一套源码里被改过三次，**最终值已与"队列位"完全脱钩**。
按补丁应用顺序（文件名升序）：

| 顺序 | 补丁 | 对宏的改动 | 当时的值 |
|---|---|---|---|
| 1 | `999-eth-26` | 新增 `#define MTK_QDMA_QUEUE_MASK (MTK_QDMA_NUM_QUEUES - 1)` | `0xF`（NUM_QUEUES=16） |
| 2 | `999-eth-56` | `MTK_QDMA_NUM_QUEUES` 改为 per-SoC 运行时值 `(eth->soc->num_tx_queues)` | 依赖 `eth` 处于作用域 |
| 3 | `999-zzz-5113`（**已于 2026-10-22 删除**） | `#define MTK_QDMA_QUEUE_MASK GENMASK(15, 0)` | **`0xFFFF`** |

> ⚠️ 第 3 步是致命的。`0xFFFF` 覆盖 bit0..15，其中 bit8 以上是 conntrack / fwmark 的地盘，
> bit7 是 `HNAT_EXCEPTION_BIT`，都不是队列位。
>
> ⚠️ 第 2 步也不能回退：若走 per-SoC 表达式，需要 `struct mtk_eth *eth` 在当前作用域，
> 而 `hnat_nf_hook.c` 里该函数内的 `eth` 实际是 `struct ethhdr eth`（以太网头），**直接编译失败**。
>
> ℹ️ `999-zzz-5113` 曾把队列数强设为 64（V2/V3）。2026-10-13 的 MTK feeds 带来了官方补丁
> `999-eth-56`（改编自 OpenWrt upstream 795-04，作者 chak-kei.lam），已实现同样的
> "per-SoC 队列数 + `qdma.page` 寄存器分页"，且更完善（page 复位、`dev->num_tx_queues` 校验）。
> 5113 的 4 个 hunk 因此全部失效（replay 验证：3/3 + 1/1 FAILED），会中止构建，已删除。
> 删除后 `MTK_QDMA_QUEUE_MASK` = `(eth->soc->num_tx_queues) - 1` = **15**（多数 SoC）或 **31**（MT7988）。

因此**禁止**在自定义判定里复用 `MTK_QDMA_QUEUE_MASK`。

---

## 3. 正确写法（本仓库采用）

`hnat.h` 中新增自有宏，exeption bit 以下才是队列位，并按 NETSYS 版本区分，**与第 1 节官方口径严格一致**：

```c
#define HNAT_QDMA_QUEUE_MASK_MORE	GENMASK(6, 0)	/* netsys v2/v3 -> 0x7f */
#define HNAT_QDMA_QUEUE_MASK_LESS	GENMASK(3, 0)	/* netsys v1.x  -> 0x0f */
#define HNAT_QDMA_QUEUE_MASK								\
	((hnat_priv->data->version == MTK_HNAT_V2 ||				\
	  hnat_priv->data->version == MTK_HNAT_V3) ?				\
		 HNAT_QDMA_QUEUE_MASK_MORE : HNAT_QDMA_QUEUE_MASK_LESS)
```

约束：
- 插入点必须在 `HNAT_EXCEPTION_BIT`(279) 与 `extern struct mtk_hnat *hnat_priv;`(1468) **之后**，当前位于 1555。
- 宏体使用 GNU C 续行 `\`，行尾不得残留任何不可见字符（曾误插入 `DEL 0x7f`，会编译失败）。

**"是否已显式指定队列"的判定阈值**：`MAX_PPPQ_QUEUE_NUM`

```c
#define MAX_SWITCH_PORT_NUM	(6)
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
#define MAX_PPPQ_QUEUE_NUM	(2 * MAX_SWITCH_PORT_NUM + 2)	/* = 14 */
#else
#define MAX_PPPQ_QUEUE_NUM	(2 * MAX_SWITCH_PORT_NUM)	/* = 12 */
#endif
```

这与上方 PPPQ/HQoS 官方代码同口径：

```c
if (IS_HQOS_MODE || (skb->mark & MTK_QDMA_QUEUE_MASK) >= MAX_PPPQ_QUEUE_NUM)
	qid = skb->mark & (MTK_QDMA_QUEUE_MASK);      /* 显式队列 */
else if (IS_PPPQ_MODE && IS_PPPQ_PATH(dev, skb))
	qid = per-port queue;
else
	qid = 0;
```

> 注：`mark` 落在 `[0, MAX_PPPQ_QUEUE_NUM)` 时无法区分"用户指定"与"per-port queue"，
> 官方同样是此语义，故遵循它即可。

---

## 4. short-pkt-qos 判定与真值表

最终代码（`999-zzz-5120`）：

```c
if (short_pkt_toggle &&
    (skb->mark & HNAT_QDMA_QUEUE_MASK) < MAX_PPPQ_QUEUE_NUM) {
	/* TCP ACK (payload 0) / 小 UDP → qid = short_pkt_qid */
}
```

⚠️ **不要给这个判定加 `!IS_HQOS_MODE`**——详见第 5 节。只有真正的短包才会改 qid，
已分类流的大包仍留在规则选定的队列里。

### netsys v3（MAX_PPPQ_QUEUE_NUM = 14, mask 0x7f）

| skb->mark | 说明 | mark & mask | 判定 | 结果 | 旧逻辑(0xffff) |
|---|---|---|---|---|---|
| `0x0000` | 无 mark（默认流量） | `0x00` | `< 14` | **短包调度生效** | 生效 |
| `0x0080` | HNAT_EXCEPTION_BIT | `0x00` | `< 14` | 生效（exception bit 不参与队列） | ❌ 失效 |
| `0x000d` | queue 13（PPPQ 范围内） | `0x0d` | `< 14` | **生效** | ❌ 失效 |
| `0x000e` | queue 14（显式指定） | `0x0e` | `>= 14` | 跳过（保留用户队列） | ❌ 失效 |
| `0x0040` | bit6 置位 | `0x40` | `>= 14` | 跳过（属显式队列） | ❌ 失效 |
| `0x0001` | conntrack/fwmark 低位 | `0x01` | `< 14` | **生效** | ❌ 失效 |

### netsys v2（MAX = 12, mask 0x7f）

| skb->mark | 说明 | mark & mask | 判定 | 结果 |
|---|---|---|---|---|
| `0x0000` | 无 mark | `0x00` | `< 12` | **生效** |
| `0x0080` | exception bit | `0x00` | `< 12` | 生效 |
| `0x000d` | queue 13 | `0x0d` | `>= 12` | 跳过 |
| `0x000e` | queue 14 | `0x0e` | `>= 12` | 跳过 |
| `0x0040` | bit6 置位 | `0x40` | `>= 12` | 跳过 |
| `0x0001` | fwmark 低位 | `0x01` | `< 12` | **生效** |

### netsys v1.x（MAX = 12, mask 0x0f）

| skb->mark | 说明 | mark & mask | 判定 | 结果 |
|---|---|---|---|---|
| `0x0000` | 无 mark | `0x00` | `< 12` | **生效** |
| `0x0080` | exception bit | `0x00` | `< 12` | 生效 |
| `0x000d` | queue 13 | `0x0d` | `>= 12` | 跳过 |
| `0x0040` | bit6 置位 | `0x00` | `< 12` | **生效**（v1 只看 bit0..3，符合官方口径） |

**修复前后对比（典型值 `mark = 0x01`，任何 conntrack/fwmark 都会带入）**

```
旧：!(skb->mark & 0xffff)      -> False  整个短包块被跳过 → 功能静默失效
新：(skb->mark & 0x7f) < 14    -> True   正常调度
```

---

## 5. 致命陷阱：判定不要加 `!IS_HQOS_MODE`

`IS_HQOS_MODE` 的定义是 `(qos_toggle == 1)`，而 **HQoS 模式正是 `skb->mark → QDMA 队列` 映射生效的前提**。
用户态 `luci-app-eqos-mtk` 在开启短包优先级时，会**主动**把 `qos_toggle` 写成 1：

```sh
# package/mtk/applications/luci-app-eqos-mtk/root/usr/sbin/eqos
# apply_short_pkt()  第 230-235 行
if [ -e "$HNAT_DIR/qos_toggle" ]; then
	qos_cur="$(cat "$HNAT_DIR/qos_toggle")"
	if [ "$qos_cur" -ne 1 ] && [ "$qos_cur" -ne 2 ]; then
		printf '%s' 1 > "$HNAT_DIR/qos_toggle"     # ← HQoS 模式
	fi
fi
printf '%s' 1 > "$HNAT_DIR/short_pkt_toggle"
```

`apply_priority_rules()`（第 470 行）同样会先设 `qos_toggle=1`，注释写明：
*"HQoS mode must be on for the mark -> queue mapping to apply"*

于是两者的组合结果是：

| 角色 | 值 |
|---|---|
| `luci-app-eqos-mtk` 写入 | `qos_toggle = 1`、`short_pkt_toggle = 1` |
| `IS_HQOS_MODE` | **真** |
| 若判定写成 `short_pkt_toggle && !IS_HQOS_MODE && ...` | `!IS_HQOS_MODE` = 假 → **整段永不执行** |

**即：只要用户在 LuCI 界面勾选 "Short-packet priority"，功能就 100% 静默失效。**
（这是一个曾经真实踩过并已修正的坑，改动务必不要把它加回来。）

「用户自定义规则优先」这层保护由 `(mark & mask) < MAX_PPPQ_QUEUE_NUM` 提供即可，
且短包块内部只对真正的短包改 qid，不需要再叠 HQoS 判断。

### LuCI EQOS 实际行为（用户勾选后，netsys v3，MAX=14）

| 流量来源 | mark | 结果 |
|---|---|---|
| 默认流量（无 mark） | `0x00` | 短包 → `short_pkt_qid` |
| conntrack / fwmark 低位 | `0x01` | 短包 → `short_pkt_qid` |
| prio 规则 prio=1/3（上传 qid 1/7） | `0x01/0x07` | 该规则的**短包**被 rescue 到 `short_pkt_qid`；大包留在原队列 |
| prio 规则 prio=5/8/10（qid 14/24/31） | `0x0e/0x18/0x1f` | 保留用户队列，不覆盖 |
| 下载方向规则（qid 32+） | `0x20+` | 保留用户队列，不覆盖 |

这正是设计意图：*ACKs do not queue behind bulk in the rule's queue*。

### 与 nft 侧 `sp_redirect` 是否重复

`emit_priority_mark()` 的小包 rescue 会把规则内的小包再标成 `spq = short_pkt_qid`（默认 14）。
内核侧看到 `mark & 0x7f = 14`，`14 < 14` 为假 → **跳过，不会重复处理**。两套机制自洽。

> 建议 `short_pkt_qid` 保持 `>= MAX_PPPQ_QUEUE_NUM`（v3 为 14，v2 为 12）。
> 若设成更小的值（如 3），短包 rescue 仍会工作，只是落到一个优先级较低的队列，效果不明显。

---

## 6. 另一个必须记住的坑：HNAT 只在首次绑定时算队列

`hnat` 编译后由硬件转发，**只在首次绑定（bind）时走上述分类路径**，之后队列号固化进 PPE 表项。
因此改 debugfs 开关对已绑定的流无效。

`999-zzz-5119` 已在写回调里处理（照 `mcast_table_write` 的现成写法）：

```c
if (var != (int)val) {
	var = val;
	/* 已绑定流会保留 PPE 表项里的旧队列，需清掉让新值对新建流生效 */
	foe_clear_all_bind_entries();
}
```

---

## 7. 调试开关与生效验证

```sh
# 逐个 cat，切勿用 glob 遍历整个 hnat 目录
cat /sys/kernel/debug/hnat/short_pkt_toggle
echo 1 > /sys/kernel/debug/hnat/short_pkt_toggle
cat /sys/kernel/debug/hnat/short_pkt_qid
cat /sys/kernel/debug/hnat/short_pkt_len
```

⚠️ **不要用 `cat /sys/kernel/debug/hnat/*`**：NETSYS v2 上 `mcast_table` 节点会因
`hnat_priv->pmcast == NULL` 而 panic（`999-zzz-5115` 已补 `!pmcast` 保护，仍建议避免 glob）。

开启动后内核日志应出现（`pr_info_once`）：

```
hnat shortpkt: TCP ACK -> qid <short_pkt_qid>
```

若 `dmesg` 无此行，按顺序排查：

1. `short_pkt_toggle` 是否真的为 1
2. **`qos_toggle` 是否为 1 或 2** —— HQoS 没开时 `skb->mark → QDMA 队列` 映射不生效，
   即使 `short_pkt_toggle=1` 也看不到效果。走 LuCI 界面会自动置 1；手工 debugfs 调试需自己先设。
3. 这些流是否已绑定（改动新值时应触发 `foe_clear_all_bind_entries`，见第 6 节）
4. 是否为 IPv6 + 端口/分片组合（第 4 节未覆盖的边角情况）

> 注意：**不要**因为担心 HQoS 就给判定加 `!IS_HQOS_MODE`，那会让 Lua/LuCI 路径完全失效（第 5 节）。

---

## 8. 相关文件索引

| 文件 | 作用 |
|---|---|
| `target/linux/mediatek/files-6.12/drivers/net/ethernet/mediatek/mtk_hnat/hnat.h` | `HNAT_EXCEPTION_BIT`、`enum mtk_hnat_version`、`MAX_PPPQ_QUEUE_NUM`、`HNAT_QDMA_QUEUE_MASK` |
| `.../mtk_hnat/hnat_nf_hook.c:2937` | HQoS 回填队列的官方权威口径（`0x7f` / `0x0f`） |
| `.../mtk_hnat/hnat_nf_hook.c:3070` | PPPQ/HQoS 队列判定（short-pkt 块紧随其后） |
| `.../mtk_hnat/hnat_nf_hook.c:4207` | `skb->mark & HNAT_EXCEPTION_BIT` 判定 |
| `target/linux/mediatek/patches-6.12/999-eth-26` | 引入 `MTK_QDMA_QUEUE_MASK` |
| `target/linux/mediatek/patches-6.12/999-eth-56` | NUM_QUEUES 改 per-SoC 运行时值 |
| ~~`target/linux/mediatek/patches-6.12/999-zzz-5113`~~ | MASK 改 `GENMASK(15,0)`；队列数强设 64。**已删除**（被 `999-eth-56` 取代，见第 2 节） |
| ~~`999-zzz-5119` / `5120` / `5121`~~ | short-pkt 全部实现。**2026-10-22 已折叠进 `files-6.12` 源码，补丁删除**（见第 11 节） |
| `package/mtk/applications/luci-app-eqos-mtk/root/usr/sbin/eqos` | 用户态：`apply_short_pkt()` 置 `qos_toggle=1` 再开 `short_pkt_toggle`；`emit_priority_mark()` 做 nft 侧小包 rescue |
| `.../luci-app-eqos-mtk/htdocs/.../eqos.js` | LuCI 界面：`short_pkt_priority` / `short_pkt_len` / `short_pkt_qid` |

---

## 9. 每次同步 MTK feeds 后的核对清单

1. 重新确认 `MTK_QDMA_QUEUE_MASK` 是否被再次改写（`grep -rn "define MTK_QDMA_QUEUE_MASK" target/linux/mediatek/patches-6.12/`）
2. 重新确认 `MAX_PPPQ_QUEUE_NUM` 与 `MAX_SWITCH_PORT_NUM` 是否变化
3. 确认 `HNAT_EXCEPTION_BIT` 是否仍是 `BIT(7)`；若变化，`HNAT_QDMA_QUEUE_MASK` 需同步调整（它应始终等于 exception bit 以下的所有位）
4. 重跑补丁链回放：全树补丁应全部干净应用（2026-10-22 起自有补丁仅剩 5 个：
   `5000`/`5001`/`5101`/`5117`/`5300`，其余已折叠进 `files-6.12`，见第 11 节）
5. 确认 `luci-app-eqos-mtk` 的 `apply_short_pkt()` 仍在开 `short_pkt_toggle` 前把 `qos_toggle` 置为 1；
   若它改为不再设 HQoS，则需相应调整第 5 节的前提
6. 确认短包判定时**没有**被加回 `!IS_HQOS_MODE` 之类的 HQoS 排除条件（第 5 节的已知陷阱）
7. 确认 HNAT 目录（`mtk_hnat/`）下**没有任何**对 `MTK_QDMA_QUEUE_MASK` /
   `MTK_QDMA_NUM_QUEUES` 的引用（第 10 节的已知陷阱）：
   `grep -rn "MTK_QDMA_QUEUE_MASK\|MTK_QDMA_NUM_QUEUES" target/linux/mediatek/files-6.12/drivers/net/ethernet/mediatek/mtk_hnat/ | grep -v hnat.h`

---

## 10. HNAT 文件禁止引用 per-SoC QDMA 宏

`999-eth-56` 之后，`mtk_eth_soc.h` 中的两个宏变成了 per-SoC 运行时形式，**展开需要
当前作用域存在 `struct mtk_eth *eth`**：

```c
#define MTK_QDMA_NUM_QUEUES  (eth->soc->num_tx_queues)
#define MTK_QDMA_QUEUE_MASK  (MTK_QDMA_NUM_QUEUES - 1)
```

HNAT 的文件里没有这个 `eth`，反而有同名遮蔽：

| 文件 / 函数 | 该作用域的 `eth` | 引用 per-SoC 宏的后果 |
|---|---|---|
| `hnat_nf_hook.c` `hnat_bind_crypto_entry()` | `struct ethhdr eth`（**值**，非指针） | `eth.soc` 编译错误 |
| `hnat_nf_hook.c` `skb_to_hnat_info()` | `struct ethhdr *eth` | `eth->soc` 类型错误 |
| `hnat_api.c` `hnat_entry_set_hqos()` | 无 | 未定义标识符 |
| `hnat_debugfs.c` `hnat_init_debugfs()` | 无 | 未定义标识符 |

这曾是 5113（硬编码 64 / `GENMASK(15,0)` 常量）掩盖的隐患：5113 存在时这些引用能编译，
**删除 5113 当天就炸**（`hnat_nf_hook.o Error 1`）。

处理：HNAT 侧一律用自有宏——队列判定用 `HNAT_QDMA_QUEUE_MASK`，
队列数上限用新增的 `HNAT_QDMA_NUM_QUEUES`（=32，最宽 `num_tx_queues`），
`hnat_ppd_xmit()` 直接以 `dev->real_num_tx_queues` 为界去掉 QDMA 约束。
`mtk_eth_soc.c` / `mtk_eth_dbg.c` / `mtk_ppe_offload.c` / `mtk_ppe_debugfs.c`
的作用域里有真 `struct mtk_eth *eth`，无需改动。

---

## 11. 自有补丁的折叠策略（2026-10-22 起）

为降低跟随父仓库/feeds 同步的维护成本，自有补丁只保留**必须与上游共享文件**的部分：

| 保留的补丁 | 原因 |
|---|---|
| `999-zzz-5000`（mt7986a dts） | 改 `mt7986a.dtsi`，上游 115/194/197/199 等补丁也改它 |
| `999-zzz-5001`（mt7981b dts） | 改 `mt7981b.dtsi`，上游 117/999-dts-* 等补丁也改它 |
| `999-zzz-5101`（mtk_eth_soc 合并） | 改 `mtk_eth_soc.c`，generic 702/721/730/732 + feeds 也改它 |
| `999-zzz-5117`（Kconfig 依赖） | feeds `999-eth-91` 等也改该 Kconfig |
| `999-zzz-5300`（xfrm 检查） | feeds `999-ppe-29/38` 等也改该文件 |

其中 5000/5001 是原 6 个 DTS 补丁合并（每 SoC 一个），5101 是原
5101/5106/5110 三个 `mtk_eth_soc.c` 补丁合并；内容均未变。

**已折叠进 `files-6.12`（补丁删除，共 21 个）**：5102~5121 中除 5101/5106/5110/5117
外的全部 HNAT 补丁（5102-5105、5107-5109、5111、5112、5114、5115、5116、5118、
5119、5120、5121），以及 5200-5204 全部 wifi_utility/iwconfig 补丁。
折叠方式：把回放树（全部补丁应用后的最终态）中对应文件直接拷入
`target/linux/mediatek/files-6.12/`。这些文件没有任何上游补丁触碰，
因此折叠后与上游**零交互**——feeds 更新时不会再产生冲突或 fuzz。

折叠后 short-pkt-qos 的实现位置（不再是补丁）：
- `files-6.12/.../mtk_hnat/hnat.h`：`HNAT_QDMA_QUEUE_MASK`、`HNAT_QDMA_NUM_QUEUES`
- `files-6.12/.../mtk_hnat/hnat_debugfs.c`：`short_pkt_toggle/qid/len` 节点 + 写时清表
- `files-6.12/.../mtk_hnat/hnat_nf_hook.c`：短包调度判定（第 4 节）

验证：折叠/合并后全树补丁回放（1056 个）0 失败，且最终内核树与
折叠前逐字节一致（`backup-011515/replay.sh` + 目录 diff）。
