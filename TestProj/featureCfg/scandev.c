#include "l0001-0/l0001-0.h"
#include "l0002-0/l0002-0.h"
#include "l0003-0/l0003-0.h"
#include "l0004-0/l0004-0.h"
#include "l0006-0/l0006-0.h"
#include "l0007-0/l0007-0.h"

#include "config.h"
#include "scandev.h"
#include "cJSON.h"

#define MODNAME		"[A1][SD]"

static int _getshelf_cfgnodes(struct l0007_shelf_conf *shelf_cfg);

static int _copy_feature_to_rack(struct l0007_rack_conf *rack_cfg, struct l0006_netcmd_relayfeature *relay_feat);
static int _copy_feature_to_shelf(struct l0007_rack_conf *rack_cfg, struct l0006_netcmd_chantermsfeature *chan_feat);

static int _scan_isneeded(struct l0007_rack_conf *rack_cfg);
static int _set_scanflag(struct l0007_rack_conf *rack_cfg, int scanflag, int fwver);
static int _exec_scan(struct task_info *config_task, int rack_index, struct scan_rack_feature *scan_rack);

static int _exec_scan_rack(struct l0006_netcom_info *nc, struct task_info *task, struct l0007_rack_conf *rack_cfg, struct scan_rack_feature *scan_rack);
static int _exec_scan_chanterms(struct l0006_netcom_info *nc, int chan, struct task_info *task, struct l0007_rack_conf *rack_cfg, struct l0006_netcmd_chantermsfeature *scan_chan);

static int _load_rackfeature(struct l0007_rack_conf *rack_cfg, struct scan_rack_feature *rack_feat);
static int _save_rackfeature(struct l0007_rack_conf *rack_cfg, struct scan_rack_feature *rack_feat);


int _getshelf_cfgnodes(struct l0007_shelf_conf *shelf_cfg)
{
	int idx = 0;
	int nnodes = 0;
	for( idx = 0; idx < L0007_SHELF_LANE_MAX; idx++){
		struct l0007_lane_conf *lane_cfg = &shelf_cfg->laneconf[idx];
		if( shelf_cfg->lanes_bmp & (1 << idx)){
			nnodes += lane_cfg->nnodes;
		}
	}		
	return nnodes;
}

int _copy_feature_to_rack(struct l0007_rack_conf *rack_cfg, struct l0006_netcmd_relayfeature *relay_feat)
{
	struct l0007_rack_feature *rack_feat;

	rack_feat = (struct l0007_rack_feature *)&relay_feat->feature_data;

	memcpy(&rack_cfg->rackfeature, rack_feat, sizeof(struct l0007_rack_feature));
	rack_cfg->scanflag = 1;	
	return 0;
}

int _copy_feature_to_shelf(struct l0007_rack_conf *rack_cfg, struct l0006_netcmd_chantermsfeature *chan_feat)
{
	struct l0007_rack_feature *rack_feat;
	struct l0007_node_feature *node_feat;
	int i, j, k, nret = 0;
	int start_shelf = 0, shelf_num = 0, cfg_nodes = 0, start_lane = 0;
	int nsend = 0, scaned_flag;	

	start_shelf = -1;
	shelf_num = 0;
	cfg_nodes = 0;
	for( i = chan_feat->chanid; i < rack_cfg->shelves_num; i++){
		if( rack_cfg->shelves_bmp & (1 << i)){
			if(rack_cfg->shelfconf[i].channel_index == chan_feat->chanid){
				if(start_shelf < 0){
					start_shelf = i;
				}
				shelf_num++;
				cfg_nodes += _getshelf_cfgnodes(&rack_cfg->shelfconf[i]);
			}
		}
	}
	if( start_shelf < 0){
		printf("ch = %d, but not find shelf!! \n");
		return 1;
	}
	printf("start_shelf:%d, shelf_num:%d, cfg_nodes:%d\n", start_shelf, shelf_num, cfg_nodes);
	for( i = 0; i < chan_feat->term_count && i < L0006_NETCMD_CHAN_MAX_TERMS; i++){
		node_feat = (struct l0007_node_feature *)&chan_feat->term_list[i].feature_data;
		for( j = start_shelf; j < start_shelf + shelf_num; j++){
			for( k = 0; k < rack_cfg->shelfconf[j].lanes_num; k++){
				if( rack_cfg->shelfconf[j].lanes_bmp & ( 1 << k)){
					if( rack_cfg->shelfconf[j].laneconf[k].node_index[0] == i){
						rack_cfg->shelfconf[j].laneconf[k].scanflag = L0007_SCANFLAG_SUCCESS;
						memcpy(&rack_cfg->shelfconf[j].laneconf[k].node_feature, node_feat, sizeof(struct l0007_node_feature));
						rack_cfg->shelfconf[j].scanflag = L0007_SCANFLAG_SUCCESS;
					}
				}
			}
		}
	}
	if( chan_feat->term_count > cfg_nodes){
		i = start_shelf + shelf_num - 1;
		start_lane = rack_cfg->shelfconf[i].lanes_num;

		rack_cfg->shelfconf[i].scanlanes_num = rack_cfg->shelfconf[i].lanes_num;
		k = cfg_nodes;
		for( j = start_lane; j < L0007_SHELF_LANE_MAX; j++){
			if( k >= chan_feat->term_count){
				break;
			}
			node_feat = (struct l0007_node_feature *)&chan_feat->term_list[k].feature_data;
			
			rack_cfg->shelfconf[i].laneconf[j].scanflag = L0007_SCANFLAG_SUCCESS;
			memcpy(&rack_cfg->shelfconf[i].laneconf[j].node_feature, node_feat, sizeof(struct l0007_node_feature));
			rack_cfg->shelfconf[i].scanflag = L0007_SCANFLAG_SUCCESS;
			rack_cfg->shelfconf[i].scanlanes_num++;
			k++;
		}
	}

	for( i = 0; i < rack_cfg->shelves_num; i++){
		if( rack_cfg->shelves_bmp & (1 << i)){
			scaned_flag = 1;
			for( j = 0; j < rack_cfg->shelfconf[i].lanes_num; j++){
				if( rack_cfg->shelfconf[i].lanes_bmp & ( 1 << j)){
					if( rack_cfg->shelfconf[i].laneconf[j].scanflag != L0007_SCANFLAG_SUCCESS){
						scaned_flag = 0;
						break;
					}
				}
			}
			if(!scaned_flag){
				rack_cfg->shelfconf[i].scanflag = L0007_SCANFLAG_FAILED;
			}
		}
	}	
	return 0;
}

int _exec_scan_rack(struct l0006_netcom_info *nc, struct task_info *task, struct l0007_rack_conf *rack_cfg, struct scan_rack_feature *scan_rack)
{
	struct l0007_rack_feature *rack_feat;

	int nret = 0;

	printf("l0006_netset_readRelayFeature \n");
	nret = l0006_netset_readRelayFeature(nc, task, &scan_rack->rack_feat);

	if( nret == 0){
		rack_feat = (struct l0007_rack_feature *)&scan_rack->rack_feat.feature_data;

		scan_rack->rack_scanflag = 1;
		scan_rack->rack_index = rack_cfg->rack_index;
		strcpy(scan_rack->rack_id, rack_cfg->rackid);
		
		_copy_feature_to_rack(rack_cfg, scan_rack);
		
		printf("read relay feature nret = %d, length:%d \n", nret, scan_rack->rack_feat.length);		
		printf("rack_feat firmwarever:0x%x\n", rack_feat->firmwarever);
		printf("rack_feat hardwarever:0x%x\n", rack_feat->hardwarever);
		printf("rack_feat protocolver:0x%x\n", rack_feat->protocolver);
		printf("rack_feat hwid:0x%x\n", rack_feat->hwid);
		printf("rack_feat confsize:0x%x\n", rack_feat->confsize);
		printf("rack_feat chip:0x%x\n", rack_feat->chip);
		printf("rack_feat rs485count:0x%x\n", rack_feat->rs485count);
		printf("rack_feat ethspeed:0x%x\n", rack_feat->ethspeed);
		return 0;
	}
	else{
		scan_rack->rack_scanflag = 0;
		scan_rack->rack_index = rack_cfg->rack_index;
		strcpy(scan_rack->rack_id, rack_cfg->rackid);
		memset(&scan_rack->rack_feat.feature_data, 0x0, sizeof(struct l0007_rack_feature));
		
		rack_cfg->scanflag = 0;
		return -1;
	}
}

int _exec_scan_chanterms(struct l0006_netcom_info *nc, int chan, struct task_info *task, struct l0007_rack_conf *rack_cfg, struct l0006_netcmd_chantermsfeature *scan_chan)
{
	struct l0007_rack_feature *rack_feat;
	struct l0007_node_feature *node_feat;
	int i, j, k, nret = 0;
	int start_shelf = 0, shelf_num = 0, cfg_nodes = 0, start_lane = 0;
	int nsend = 0, scaned_flag;

	while(1){
		nsend++;
		if( nsend > 3){
			break;
		}
		nret = l0006_netset_readTermsFeature(nc, task, chan, &scan_chan);
		printf("readtermfeature, ch=%d, nret=%d, chid=%d, termcount:%d \n", chan, nret, scan_chan->chanid, scan_chan->term_count);
		if( nret == 0 || nret != 2){
			printf("nret = %d break!\n", nret);
			break;
		}
		usleep(10);
	};
	if( nret != 0){
		printf("read term featurn failed \n");
		return -1;
	}

	for( j = 0; j < scan_chan->term_count && j < L0006_NETCMD_CHAN_MAX_TERMS; j++){
		node_feat = (struct l0007_node_feature *)&scan_chan->term_list[j].feature_data;
		printf("[%d-%d] length:%d,chid:%d,nodeid:%d, featureflag:%d \n", scan_chan->chanid, j, scan_chan->term_list[j].length,
			scan_chan->term_list[j].chanid, scan_chan->term_list[j].nodeid, scan_chan->term_list[j].feature_flag);
		if( scan_chan->term_list[j].feature_flag){
			printf("[%d-%d] firmwarever:0x%x \n",scan_chan->chanid, j, node_feat->firmwarever);
			printf("[%d-%d] hardwarever:0x%x \n",scan_chan->chanid, j, node_feat->hardwarever);
			printf("[%d-%d] hwid:0x%x \n",scan_chan->chanid, j, node_feat->hwid);
			printf("[%d-%d] chip:0x%x \n",scan_chan->chanid, j, node_feat->chip);
			printf("[%d-%d] peripheral:0x%x \n",scan_chan->chanid, j, node_feat->peripheral);
			printf("[%d-%d] confsize:0x%x \n",scan_chan->chanid, j, node_feat->confsize);
			printf("[%d-%d] samplingrata:0x%x \n",scan_chan->chanid, j, node_feat->samplingrata);
			printf("[%d-%d] weighingrange:0x%x \n",scan_chan->chanid, j, node_feat->weighingrange);
		}
		
	}
	return _copy_feature_to_shelf(rack_cfg, scan_chan);
}

int _scan_isneeded(struct l0007_rack_conf *rack_cfg)
{
	int idx = 0, bNeedScan = 0;
	if(rack_cfg->scanflag){
		if( rack_cfg->rackfeature.firmwarever < L0007_RACK_FWVER_SUPPORT_SCANDEV){
			return 0;
		}
		for( idx = 0; idx < rack_cfg->shelves_num; idx++){
			if(rack_cfg->shelves_bmp & (1 << idx)){
				if(!rack_cfg->shelfconf[idx].scanflag){
					bNeedScan = 1;
					break;
				}
			}
		}
	}else{
		bNeedScan = 1;
	}
	return bNeedScan;
}

int _set_scanflag(struct l0007_rack_conf *rack_cfg, int scanflag, int fwver)
{
	int i = 0, j = 0, k = 0, retval = -1;
	for(i = 0; i < rack_cfg->shelves_num; i++){
		if( rack_cfg->shelves_bmp & ( 1 << i)){
			for( j = 0; j < rack_cfg->shelfconf[i].lanes_num; j++){
				if( rack_cfg->shelfconf[i].lanes_bmp & ( 1 << j)){
					for( k = 0; k < rack_cfg->shelfconf[i].laneconf[j].nnodes; k++){
						rack_cfg->shelfconf[i].laneconf[j].node_feature[k].firmwarever = fwver;
					}
					rack_cfg->shelfconf[i].laneconf[j].scanflag = scanflag;
					rack_cfg->shelfconf[i].laneconf[j].scannodes = rack_cfg->shelfconf[i].laneconf[j].nnodes;
				}
			}
			rack_cfg->shelfconf[i].scanflag = scanflag;
		}
	}
	rack_cfg->scanflag = scanflag;
	rack_cfg->rackfeature.firmwarever = fwver;
	return 0;
}


int _exec_scan(struct task_info *config_task, int rack_index, struct scan_rack_feature *scan_rack)
{
	struct config_proc *cfg_proc = config_task->proc.priv;
	struct l0007_global_conf *gconf = (struct l0007_global_conf *)cfg_proc->gconf;
	struct l0007_rack_conf *rack_cfg = &gconf->rackconf[rack_index];
	int i = 0,retval = -1;

	if(!rack_cfg->scanflag){
		retval = l0006_netcom_iscanconnet(rack_cfg->ipaddr, 8888);
		if(retval != 0){
			ERRSYS_INFOPRINT("rack[%d] scandev try connect %s port 8888 failed\n",rack_index, rack_cfg->ipaddr);
			return -1;
		}
		retval = l0006_netcom_iscanconnet(rack_cfg->ipaddr, 7777);
		if( retval != 0){
			ERRSYS_INFOPRINT("rack[%d] scandev try connect %s port 7777 failed\n",rack_index, rack_cfg->ipaddr);
			ERRSYS_INFOPRINT("rack[%d] not support scan dev!!!\n", rack_index);
			_set_scanflag(rack_cfg, L0007_SCANFLAG_SUCCESS, 0);
			return 1;
		}
		l0006_netcmd_sendagentinfo(cfg_proc->nc, rack_cfg->ipaddr, 7777, 1);
		sleep(1); // wait agent connect to ipaddr.
		retval = l0006_cmdset_hardwareversion(cfg_proc->nc, config_task, -1, 0);
		if( retval < 0){
			_set_scanflag(rack_cfg, L0007_SCANFLAG_FAILED, 0);
			ERRSYS_INFOPRINT("rack[%d] get hardwarever failed!!!\n", rack_index);
			return -1;
		}
		_set_scanflag(rack_cfg, L0007_SCANFLAG_SUCCESS, retval);
		if( retval < L0007_RACK_FWVER_SUPPORT_SCANDEV){
			ERRSYS_INFOPRINT("rack[%d] not support scan dev, fwver=0x%x!!!\n", rack_index, retval);
			return 1;
		}
		retval = _exec_scan_rack(cfg_proc->nc, config_task, rack_cfg, scan_rack);
		if( retval < 0){
			ERRSYS_INFOPRINT("rack[%d] scan failed, retval=%d !!!\n", rack_index, retval);
			return 1;			
		}
		for( i = 0; i < rack_cfg->rackfeature.rs485count; i++){
			retval = _exec_scan_chanterms(cfg_proc->nc, i, config_task, rack_cfg, &scan_rack->chan_list[i]);
			if( retval != 0){
				ERRSYS_INFOPRINT("rack[%d] ch=%d scan failed, retval=%d !!!\n", rack_index,i, retval);
			}
		}
	}
	else{
		for( i = 0; i < rack_cfg->shelves_num; i++){
			if( (rack_cfg->shelves_bmp & ( 1 << i)) && ( !rack_cfg->scanflag)){
				retval = _exec_scan_chanterms(cfg_proc->nc, config_task, rack_cfg->shelfconf[i].channel_index, rack_cfg, &scan_rack->chan_list[i]);
				if( retval != 0){
					ERRSYS_INFOPRINT("rack[%d] ch=%d scan failed, retval=%d !!!\n", rack_index,i, retval);
				}
			}
		}
	}
}


int _load_rackfeature(struct l0007_rack_conf *rack_cfg, struct scan_rack_feature *rack_feat)
{
	
	return 0;
}

int _save_rackfeature(struct l0007_rack_conf *rack_cfg, struct scan_rack_feature *rack_feat)
{
	return 0;
}

int scandev_rack_dumpcfg(struct l0007_global_conf *gconf, const char *szNote)
{
	FILE* fp = NULL;
	static int nDumpCount = 0;
	char szFileName[256] = {0};
	int i,j,k,l;
	
	nDumpCount++;
	mkdir("/tmp/log_dump",0755);
	sprintf(szFileName, "/tmp/log_dump/rackcfg_%d_%s",nDumpCount, szNote);
	fp = fopen(szFileName, "w");
	if( fp == NULL){
		return -1;
	}
	fprintf(fp, "---- rack_num:%d - rack_bmp:0x%x ---- \n", gconf->racks_num, gconf->racks_bmp);
	printf("---- rack_num:%d - rack_bmp:0x%x ---- \n", gconf->racks_num, gconf->racks_bmp);
	for( i = 0; i < L0007_ECU_RACK_MAX; i++){
		if( gconf->racks_bmp & ( 1 << i)){
			struct l0007_rack_conf *rack_cfg = &gconf->rackconf[i];
			fprintf(fp, "####### i=%d, rack_index:%d, rack_id:%d, ip:%s \n", i, rack_cfg->rack_index, rack_cfg->rackid, rack_cfg->ipaddr);
			fprintf(fp, "shelf_num:%d, shelf_bmp:0x%x, scan_flag:%d \n", rack_cfg->shelves_num, rack_cfg->shelves_bmp, rack_cfg->scanflag);
			
			struct l0007_rack_feature * rack_feat = (struct l0007_rack_feature *)&rack_cfg->rackfeature;
			fprintf(fp, "rack_feat firmwarever:0x%x, hardwarever:0x%x,protocolver:0x%x,hwid:0x%x,confsize:0x%x,chip:0x%x,rs485count:0x%x,ethspeed:0x%x \n",
				rack_feat->firmwarever, rack_feat->hardwarever, rack_feat->protocolver, rack_feat->hwid,
				rack_feat->confsize, rack_feat->chip, rack_feat->rs485count, rack_feat->ethspeed);
			
			for( j = 0; j < L0007_RACK_SHELF_MAX; j++){
				struct l0007_shelf_conf *shelf_cfg = &rack_cfg->shelfconf[j];
				if( rack_cfg->shelves_bmp & (1 << j)){
					fprintf(fp, "-- shelf_index:%d, chan_index:%d lanes_num:%d, lanes_bmp:0x%x \n", shelf_cfg->shelf_index, shelf_cfg->channel_index,
						shelf_cfg->lanes_num, shelf_cfg->lanes_bmp);
					fprintf(fp, "-- scanflag:%d, scanlanenum:%d \n", shelf_cfg->scanflag, shelf_cfg->scanlanes_num);
					for( k = 0; k < L0007_SHELF_LANE_MAX; k++){
						struct l0007_lane_conf *lane_cfg = &shelf_cfg->laneconf[j];
						if( shelf_cfg->lanes_bmp & (1 << k)){
							fprintf("-------lane_idx:%d, nnodes:%d, node_idx:%d, scan:%d, scannodes:%d\n", lane_cfg->lane_index, lane_cfg->nnodes,
									lane_cfg->node_index, lane_cfg->scanflag, lane_cfg->scannodes);
							struct l0007_node_feature *node_feat = (struct l0007_node_feature *)&lane_cfg->node_feature[0];
							
							fprintf("node_feat firmwarever:0x%x,hardwarever:0x%x,hwid:0x%x,chip:0x%x,peripheral:0x%x,confsize:0x%x,samplingrata:0x%x,weighingrange:0x%x \n",
								node_feat->firmwarever, node_feat->hardwarever,
								node_feat->hwid,node_feat->chip,
								node_feat->peripheral,node_feat->confsize,
								node_feat->samplingrata,node_feat->weighingrange);
						}
					}
				}
			}
		}
	}
	if( fp != NULL){
		fclose(fp);
	}
	return 0;
}

int scandev_rack_readfeature(struct l0006_netcom_info *nc, struct task_info *task, struct l0007_rack_conf *rack_cfg)
{
	struct scan_rack_feature *scan_rack = NULL;
	int retval = -1;
	
	scan_rack = (struct scan_rack_feature *)zmalloc(sizeof(struct scan_rack_feature));
	if( scan_rack == NULL){
		return -1;
	}

	retval = _load_rackfeature(rack_cfg, scan_rack);
	if( retval == -1){
		printf("rack_loadfeature failed! \n");
		goto err;
	}

	if( _scan_isneeded(rack_cfg)){
		retval = _exec_scan(task, rack_cfg->rack_index, scan_rack);
		if( retval != 0){
			printf("_exec_scan failed!\n");
			goto err;
		}
		retval = _save_rackfeature(rack_cfg, scan_rack);
	}

err:
	if( scan_rack != NULL)
		free(scan_rack);
	return retval;
}

