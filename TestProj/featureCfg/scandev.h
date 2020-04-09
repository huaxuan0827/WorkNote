#ifndef __SCANDEV_H__
#define __SCANDEV_H__

struct scan_rack_feature{
	uint8_t rack_id[L0007_RACKID_LEN];
	int rack_index;
	int rack_scanflag;
	int rack_fwver;
	int chan_num;
	struct l0006_netcmd_relayfeature 	 rack_feat;
	struct l0006_netcmd_chantermsfeature chan_list[L0007_DSU_CHANNEL_MAX];
};

int scandev_rack_dumpcfg(struct l0007_global_conf *gconf, const char *szNote);
int scandev_rack_readfeature(struct l0006_netcom_info *nc, struct task_info *task, struct l0007_rack_conf *rack_cfg);


#endif
