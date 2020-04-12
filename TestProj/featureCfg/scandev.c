#include <stdio.h>
#include "netcmd.h"
#include "base.h"
#include "scandev.h"
#include "stddef.h"
#include "cJSON.h"

static int _parse_rackfeature(const char *json, struct scan_rack_feature *scan_rack)
{
	cJSON *root = NULL, *tmpnode = NULL;
	cJSON *chan_list = NULL, *chan_root = NULL;
	cJSON *node_list = NULL, *node_root = NULL;
	struct l0007_rack_feature *rack_feat = NULL;
	int i = 0,j = 0, nChanList = 0, nNodeList = 0;
	int retval = -1;

	rack_feat = (struct l0007_rack_feature *)&scan_rack->rack_feat;
	root = cJSON_Parse(json);
	if( root == NULL){
		goto err;
	}

	// rack_id.
	tmpnode = cJSON_GetObjectItem(root, "rack_id");
	if( tmpnode == NULL || tmpnode->valuestring == NULL){
		goto err;
	}
	strncpy(scan_rack->rack_id, tmpnode->valuestring, L0007_RACKID_LEN -1);

	// rack_index.
	tmpnode = cJSON_GetObjectItem(root, "rack_index");
	if( tmpnode == NULL){
		goto err;
	}
	scan_rack->rack_index = tmpnode->valueint;

	// rack_scanflag.
	tmpnode = cJSON_GetObjectItem(root, "rack_scanflag");
	if( tmpnode == NULL){
		goto err;
	}
	scan_rack->rack_scanflag = tmpnode->valueint;

	// rack_firmwarever.
	tmpnode = cJSON_GetObjectItem(root, "rack_firmwarever");
	if( tmpnode == NULL){
		goto err;
	}
	scan_rack->rack_fwver = tmpnode->valueint;
	rack_feat->firmwarever = tmpnode->valueint;
	
	// rack_hardwarever
	tmpnode = cJSON_GetObjectItem(root, "rack_hardwarever");
	if( tmpnode == NULL){
		goto err;
	}
	rack_feat->hardwarever = tmpnode->valueint;

	// rack_protocolver
	tmpnode = cJSON_GetObjectItem(root, "rack_protocolver");
	if( tmpnode == NULL){
		goto err;
	}
	rack_feat->protocolver = tmpnode->valueint;

	// rack_hwid
	tmpnode = cJSON_GetObjectItem(root, "rack_hwid");
	if( tmpnode == NULL){
		goto err;
	}
	rack_feat->hwid = tmpnode->valueint;
	
	// rack_confsize
	tmpnode = cJSON_GetObjectItem(root, "rack_confsize");
	if( tmpnode == NULL){
		goto err;
	}
	rack_feat->confsize = tmpnode->valueint;
	
	// rack_chip
	tmpnode = cJSON_GetObjectItem(root, "rack_chip");
	if( tmpnode == NULL){
		goto err;
	}
	rack_feat->chip = tmpnode->valueint;
	
	// rack_rs485count
	tmpnode = cJSON_GetObjectItem(root, "rack_rs485count");
	if( tmpnode == NULL){
		goto err;
	}
	rack_feat->rs485count = tmpnode->valueint;
	
	// rack_ethspeed
	tmpnode = cJSON_GetObjectItem(root, "rack_ethspeed");
	if( tmpnode == NULL){
		goto err;
	}
	rack_feat->ethspeed = tmpnode->valueint;

	// rack_chanList.
	chan_list = cJSON_GetObjectItem(root, "rack_chanList");
	if( chan_list == NULL){
		goto err;
	}
	nChanList = cJSON_GetArraySize(chan_list);
	scan_rack->chan_num = nChanList;
	for( i = 0 ; i < nChanList; i++){
		chan_root = cJSON_GetArrayItem(chan_list, i);
		// chan_index
		tmpnode = cJSON_GetObjectItem(chan_root, "chan_index");
		if( tmpnode == NULL){
			goto err;
		}
		int chan_index = tmpnode->valueint;
		struct l0006_netcmd_termfeature *term_feat = NULL;
		struct l0007_node_feature *node_feat = NULL;

		if( chan_index < 0 || chan_index >= L0007_DSU_CHANNEL_MAX){
			goto err;
		}
		// chan_nodeList
		node_list = cJSON_GetObjectItem(chan_root, "chan_nodeList");
		if( tmpnode == NULL){
			goto err;
		}
		nNodeList = cJSON_GetArraySize(node_list);
		scan_rack->chan_list[chan_index].term_count = nNodeList;
		
		for( j = 0; j < nNodeList; j++){
			node_root = cJSON_GetArrayItem(node_list, j);
			term_feat = &scan_rack->chan_list[chan_index].term_list[j];
			node_feat = (struct l0007_node_feature *)term_feat->feature_data;

			term_feat->chanid = chan_index;
			// node_index
			tmpnode = cJSON_GetObjectItem(node_root, "node_index");
			if( tmpnode == NULL){
				goto err;
			}
			term_feat->nodeid = tmpnode->valueint;
			// node_featureflag
			tmpnode = cJSON_GetObjectItem(node_root, "node_featureflag");
			if( tmpnode == NULL){
				goto err;
			}
			term_feat->feature_flag = tmpnode->valueint;
			// node_firmwarever
			tmpnode = cJSON_GetObjectItem(node_root, "node_firmwarever");
			if( tmpnode == NULL){
				goto err;
			}
			node_feat->firmwarever = tmpnode->valueint;
			// node_hardwarever
			tmpnode = cJSON_GetObjectItem(node_root, "node_hardwarever");
			if( tmpnode == NULL){
				goto err;
			}
			node_feat->hardwarever = tmpnode->valueint;
			// node_hwid
			tmpnode = cJSON_GetObjectItem(node_root, "node_hwid");
			if( tmpnode == NULL){
				goto err;
			}
			node_feat->hwid = tmpnode->valueint;
			// node_chip
			tmpnode = cJSON_GetObjectItem(node_root, "node_chip");
			if( tmpnode == NULL){
				goto err;
			}
			node_feat->chip = tmpnode->valueint;
			// node_peripheral
			tmpnode = cJSON_GetObjectItem(node_root, "node_peripheral");
			if( tmpnode == NULL){
				goto err;
			}
			node_feat->peripheral = tmpnode->valueint;
			// node_confsize
			tmpnode = cJSON_GetObjectItem(node_root, "node_confsize");
			if( tmpnode == NULL){
				goto err;
			}
			node_feat->confsize = tmpnode->valueint;
			// node_samplingrata
			tmpnode = cJSON_GetObjectItem(node_root, "node_samplingrata");
			if( tmpnode == NULL){
				goto err;
			}
			node_feat->samplingrata = tmpnode->valueint;
			// node_weighingrange
			tmpnode = cJSON_GetObjectItem(node_root, "node_weighingrange");
			if( tmpnode == NULL){
				goto err;
			}
			node_feat->weighingrange = tmpnode->valueint;
		}
	}
	retval = 0;
err:
	return retval;
}


int _load_rackfeature(struct l0007_rack_conf *rack_cfg, struct scan_rack_feature *scan_rack)
{
	char filepath[256] = {0};
	FILE *fp = NULL;
	char *szFileBuff = NULL;
	int filesize = 0;
	
	strcpy(filepath, "./feature.txt");
	fp = fopen(filepath, "r");
	if( fp == NULL){
		goto err;
	}

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if( filesize < 2){
		goto err;
	}

	szFileBuff = (char *)malloc(filesize *sizeof(char));
	if( szFileBuff == NULL){
		goto err;
	}

	if( fread( szFileBuff, filesize, sizeof(char), fp) < 0){
		goto err;
	}
	fclose(fp);
	fp = NULL;

	if( _parse_rackfeature(szFileBuff, scan_rack) != 0){
		goto err;
	}

	if( szFileBuff != NULL)
		free(szFileBuff);
	
	return 0;
err:
	if( fp != NULL)
		fclose(fp);
	if( szFileBuff != NULL)
		free(szFileBuff);
	return -1;
}


char* _generate_rackcfg2()
{
	cJSON *root,*chanlist,*nodelist;
	int i = 0;
	int j = 0;
	
	char *json;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "rack_id", 			"123");
	cJSON_AddNumberToObject(root, "rack_index", 		1);
	cJSON_AddNumberToObject(root, "rack_scanflag", 		2);
	cJSON_AddNumberToObject(root, "rack_firmwarever", 	3);
	cJSON_AddNumberToObject(root, "rack_hardwarever", 	4);
	cJSON_AddNumberToObject(root, "rack_protocolver", 	5);
	cJSON_AddNumberToObject(root, "rack_hwid",			6);
	cJSON_AddNumberToObject(root, "rack_confsize", 		7);
	cJSON_AddNumberToObject(root, "rack_chip", 			8);
	cJSON_AddNumberToObject(root, "rack_rs485count", 	9);
	cJSON_AddNumberToObject(root, "rack_ethspeed", 		10);
	chanlist = cJSON_CreateArray();
	for( i = 0; i < 5 ; i++){	
		cJSON *chan = cJSON_CreateObject();
		cJSON_AddNumberToObject(chan, "chan_index", i);
		nodelist = cJSON_CreateArray();
		for( j = 0; j < 2; j++){
			cJSON *node = cJSON_CreateObject();
			cJSON_AddNumberToObject(node, "node_index", 			i);
			cJSON_AddNumberToObject(node, "node_featureflag", 		i);
			cJSON_AddNumberToObject(node, "node_firmwarever", 		i);
			cJSON_AddNumberToObject(node, "node_hardwarever", 		i);
			cJSON_AddNumberToObject(node, "node_hwid", 				i);
			cJSON_AddNumberToObject(node, "node_chip", 				i);
			cJSON_AddNumberToObject(node, "node_peripheral", 		i);
			cJSON_AddNumberToObject(node, "node_confsize", 			i);
			cJSON_AddNumberToObject(node, "node_samplingrata", 		i);
			cJSON_AddNumberToObject(node, "node_weighingrange", 	i);
			cJSON_AddItemToArray(nodelist, node);
		}
		cJSON_AddItemToObject(chan, "chan_nodeList", nodelist);
		cJSON_AddItemToArray(chanlist, chan);
	}
	cJSON_AddItemToObject(root, "rack_chanList", chanlist);
	json = cJSON_Print(root);
	cJSON_Delete(root);	

	printf("%s \n", json);
	return json;
}

static char* _generate_rackcfg(struct scan_rack_feature *scan_rack)
{
	struct l0007_rack_feature *rack_feat = NULL;
	struct l0007_node_feature *node_feat = NULL;
	cJSON *root,*chanlist,*nodelist;
	int i = 0, j = 0;
	char *json = NULL;

	rack_feat = (struct l0007_rack_feature *)&scan_rack->rack_feat;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "rack_id", 			scan_rack->rack_id);
	cJSON_AddNumberToObject(root, "rack_index", 		scan_rack->rack_index);
	cJSON_AddNumberToObject(root, "rack_scanflag", 		scan_rack->rack_scanflag);
	cJSON_AddNumberToObject(root, "rack_firmwarever", 	scan_rack->rack_fwver);
	cJSON_AddNumberToObject(root, "rack_hardwarever", 	rack_feat->hardwarever);
	cJSON_AddNumberToObject(root, "rack_protocolver", 	rack_feat->protocolver);
	cJSON_AddNumberToObject(root, "rack_hwid",			rack_feat->hwid);
	cJSON_AddNumberToObject(root, "rack_confsize", 		rack_feat->confsize);
	cJSON_AddNumberToObject(root, "rack_chip", 			rack_feat->chip);
	cJSON_AddNumberToObject(root, "rack_rs485count", 	rack_feat->rs485count);
	cJSON_AddNumberToObject(root, "rack_ethspeed", 		rack_feat->ethspeed);
	chanlist = cJSON_CreateArray();
	for( i = 0; i < scan_rack->chan_num ; i++){	
		cJSON *chan = cJSON_CreateObject();
		if( scan_rack->chan_list[i].term_count > 0){
			cJSON_AddNumberToObject(chan, "chan_index", scan_rack->chan_list[i].term_list[0].chanid);
			nodelist = cJSON_CreateArray();
			for( j = 0; j < scan_rack->chan_list[i].term_count; j++){
				cJSON *node = cJSON_CreateObject();
				node_feat = (struct l0007_node_feature *)&scan_rack->chan_list[i].term_list[j].feature_data;
				cJSON_AddNumberToObject(node, "node_index", 			scan_rack->chan_list[i].term_list[j].nodeid);
				cJSON_AddNumberToObject(node, "node_featureflag", 		scan_rack->chan_list[i].term_list[j].feature_flag);
				cJSON_AddNumberToObject(node, "node_firmwarever", 		node_feat->firmwarever);
				cJSON_AddNumberToObject(node, "node_hardwarever", 		node_feat->hardwarever);
				cJSON_AddNumberToObject(node, "node_hwid", 				node_feat->hwid);
				cJSON_AddNumberToObject(node, "node_chip", 				node_feat->chip);
				cJSON_AddNumberToObject(node, "node_peripheral", 		node_feat->peripheral);
				cJSON_AddNumberToObject(node, "node_confsize", 			node_feat->confsize);
				cJSON_AddNumberToObject(node, "node_samplingrata", 		node_feat->samplingrata);
				cJSON_AddNumberToObject(node, "node_weighingrange", 	node_feat->weighingrange);
				cJSON_AddItemToArray(nodelist, node);
			}
			cJSON_AddItemToObject(chan, "chan_nodeList", nodelist);
			cJSON_AddItemToArray(chanlist, chan);
		}
	}
	cJSON_AddItemToObject(root, "rack_chanList", chanlist);
	json = cJSON_Print(root);
	cJSON_Delete(root);	
	
	printf("%s \n", json);
	return json;	
}

int _save_rackfeature(struct l0007_rack_conf *rack_cfg, struct scan_rack_feature *scan_rack)
{
	char filepath[256] = {0};
	FILE *fp = NULL;
	char *json = NULL;

	strcpy(filepath, "./feature2.txt");
	fp = fopen(filepath, "w");
	if( fp == NULL){
		return -1;
	}	
	json = _generate_rackcfg(scan_rack);
	if( json == NULL){
		fclose(fp);
		return -1;
	}
	
	fprintf(fp, "%s", json);
	fclose(fp);
	
	return 0;
}

