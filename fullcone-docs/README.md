# SONiC fullcone NAT integration

This branch carries the fullcone NAT work on top of `25.12`. The `25.12`
branch itself is kept clean (upstream merges only) so that syncing upstream
stays conflict-free - merge `25.12` into this branch to pick up updates.

## Contents

| Path | Purpose |
|---|---|
| `target/linux/generic/hack-6.12/984-add-sonic-fullcone-support.patch` | nf_nat_core: 3-tuple (proto / src-ip / src-port) hash table, EIM + EIF |
| `target/linux/generic/hack-6.12/986-add-sonic-fullcone-to-nft.patch` | nft_masq: registers the `fullcone` nftables expression |
| `package/network/config/firewall4/patches/001-firewall4-add-support-for-fullcone-nat.patch` | fw4: per-zone + per-protocol fullcone, counters on the rules |
| `package/network/config/firewall4/Makefile` | drops the `+kmod-nft-fullcone` dependency |
| `package/mtk/applications/luci-app-turboacc-mtk/htdocs/.../view/turboacc.js` | LuCI page: both tabs, global gates, per-zone table, live tables |
| `package/mtk/applications/luci-app-turboacc-mtk/root/usr/share/rpcd/ucode/luci.turboacc` | rpcd: status methods plus the counter / flow collectors |

Upstream patch 985 (iptables `FULLCONE` target) is deliberately not used:
this tree is firewall4-only, there is no `iptables-mod-fullconenat`.

## One prerequisite that lives outside git

`CONFIG_PACKAGE_kmod-nft-fullcone` must be unset - and `.config` is not tracked
by git, so nothing here enforces it. The old out-of-tree module registers the
very same `fullcone` nft expression as the implementation now built into
`nft_masq.ko`; whichever loads second fails to register, and a `nft_masq.ko`
that fails to load takes plain masquerading down with it as well:

    # CONFIG_PACKAGE_kmod-nft-fullcone is not set

The usual way this regresses is restoring a stale `.config`, so check any
backup before you use it. A full `make` removes the module from the image
itself via the package stale-file cleanup.

## Build

    make -j$(nproc)

## Configuration

    config defaults
        option fullcone      1    # global gate, IPv4
        option fullcone6     0    # global gate, IPv6

    config zone
        option fullcone            1        # opt this zone in (default 1)
        list   fullcone_proto     'udp'     # optional L4 protocol restriction

`defaults.fullcone` / `defaults.fullcone6` are global gates. `zone.fullcone`
defaults to 1, so with a gate enabled every masquerading zone gets fullcone -
unchanged behaviour compared to before per-zone control existed.
`zone.fullcone_proto` restricts fullcone to the listed protocols while all
other protocols keep plain masquerading: the per-protocol fullcone rules are
emitted first in the nat chain and the generic masquerade rule stays behind
them as the fallback (the first nat statement in a chain wins).

The two gates are independent. IPv6 fullcone needs `defaults.fullcone6` **and**
`zone.masq6`; without `masq6` there is nothing to make fullcone, so enabling
the v6 gate alone changes nothing. Note what that costs: `masq6` turns native
IPv6 into NAT6. For ordinary inbound access to a host, a `config rule` that
allows the specific port is the better answer and does not involve fullcone at
all.

All of this is also exposed in LuCI - see below.

Protocol names are not checked against a whitelist (same behaviour as fw4's
`proto` option, so that `gre`, `esp` etc. remain usable). A typo therefore
produces an invalid ruleset and the firewall refuses to load - use the LuCI
selector when editing by hand.

## Where the controls live

**Network -> TurboACC** - one page with two tabs.

*Settings* keeps the engine controls and the status card. Its "Full Cone NAT"
row reports "SONiC Fullcone" instead of always claiming "Disabled".

*Fullcone NAT* carries everything fullcone-specific:

  - two global gates, "Fullcone NAT (IPv4)" and "Fullcone NAT (IPv6)"
  - a per-zone table with columns *Zone / IPv4 masquerading / Fullcone NAT /
    Protocols*. The protocol column restricts fullcone to the selected L4
    protocols, leaving every other protocol on plain masquerading.
  - two live tables, described in the next section

There is no separate fullcone menu entry, and no button on the settings tab -
the controls moved into the tab.

The page lives in `luci-app-turboacc-mtk`, which is part of this repository.
Two earlier revisions are gone on purpose. The controls first lived on the
firewall page, via a patch against the luci feed; a feed re-clone silently
loses such a patch, and the switch then existed in two places. They then moved
into a separate `view/fullcone.js`, which was merged into this page's tab
instead - one page, one place to look. The firewall4 patch still provides the
actual fw4 rules; only the UI moved.

## The two live tables

Both read the running system and refresh every 10 seconds.

**Rule counters.** Every per-zone / per-protocol fullcone rule carries a
`counter` ahead of the `fullcone` statement - the same ordering fw4 already
uses for `dnat` / `masquerade` - so the page can show how much traffic actually
took the fullcone path. This is what separates "configured" from "working".
With `fastpath` set to MediaTek HNAT the offload takes packets out of the
nftables path, and only a counter reveals whether the two still meet: one that
keeps climbing means they do, one frozen at zero means the rule is bypassed.
Keep in mind that HNAT still counts the first packet of each flow, so a slow
climb is expected rather than suspicious.

**Fullcone inbound sessions.** Sessions from `/proc/net/nf_conntrack` that are

  - inbound - `original.src` public and `reply.src` a LAN address (the original
    tuple is pre-NAT, so the LAN host only appears in `reply`),
  - on a protocol the fullcone rules cover, and
  - **not** destined to a port you forwarded yourself via `config redirect`

What survives those filters could only have come from the fullcone mapping -
that is what makes it readable as "this is what fullcone opened up". Two limits
worth knowing:

  - the kernel keeps **no marker** for fullcone mappings - 984/986 add no
    `ct->status` bit - so this list is an **inference**, not a fact
  - it is a **live view, not a log**: an idle UDP mapping expires after roughly
    30 seconds, so an empty table is the normal state, not a fault

The collector parses `/proc/net/nf_conntrack` directly; this image has no
`conntrack` tool.

## Verify on the device

    nft list ruleset | grep fullcone
    cat /sys/module/nft_masq/refcnt
    fw4 print | grep -i fullcone

The first is the most direct. With the counters in place it shows both that the
rule exists and that traffic reaches it; an empty result means fw4 never emitted
a fullcone rule at all.

## Sync upstream

    git checkout 25.12
    git merge pr-source/25.12
    git checkout fullcone
    git merge 25.12

## Known risks / to measure

- **flow offloading**: not measured systematically yet. One data point so far -
  with `fastpath` on MediaTek HNAT the fullcone counters keep incrementing, so
  the first packet of each flow does traverse the nftables path. Whether that is
  enough for the NAT mapping to be established has not been verified.
- **memory**: the SONiC patch keeps a hash-table node per conntrack entry, so
  memory use grows with the connection count (BT/PT style workloads).
- `package/network/utils/fullconenat-nft/` is left in the tree but must stay
  unselected - it cannot coexist with this implementation.
