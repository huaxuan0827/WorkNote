#ifndef __SCANDEV_H__
#define __SCANDEV_H__

#include <stdint.h>
#include "base.h"
#include "netcmd.h"

struct scan_rack_feature{
	uint8_t rack_id[L0007_RACKID_LEN];
	int rack_index;
	int rack_scanflag;
	int rack_fwver;
	int chan_num;
	struct l0006_netcmd_relayfeature 	 rack_feat;
	struct l0006_netcmd_chantermsfeature chan_list[L0007_DSU_CHANNEL_MAX];
};

int _load_rackfeature(struct l0007_rack_conf *rack_cfg, struct scan_rack_feature *scan_rack);
int _save_rackfeature(struct l0007_rack_conf *rack_cfg, struct scan_rack_feature *scan_rack);

char* _generate_rackcfg2();


#endif
