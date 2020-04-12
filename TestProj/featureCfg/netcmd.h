#ifndef __L0006_NETCMD_H__
#define __L0006_NETCMD_H__

#include <stdint.h>


#define L0006_NETCMD_RELAY_FEATURE_DATA_SIZE 	17
#define L0006_NETCMD_TERM_FEATURE_DATA_SIZE 	10
#define L0006_NETCMD_CHAN_MAX_TERMS				16
struct l0006_netcmd_relayfeature {
	uint16_t length;
	uint8_t feature_data[L0006_NETCMD_RELAY_FEATURE_DATA_SIZE];	
}__attribute__((packed));

struct l0006_netcmd_termfeature {
	uint8_t length;
	uint8_t chanid;
	uint8_t nodeid;
	uint8_t feature_flag;
	uint8_t feature_data[L0006_NETCMD_TERM_FEATURE_DATA_SIZE];
	uint8_t crc;
}__attribute__((packed));

struct l0006_netcmd_chantermsfeature {
	uint8_t term_count;
	struct l0006_netcmd_termfeature term_list[L0006_NETCMD_CHAN_MAX_TERMS];
}__attribute__((packed));


#endif

