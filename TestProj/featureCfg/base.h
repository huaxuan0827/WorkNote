#ifndef __L0007_BASE_H__
#define __L0007_BASE_H__

#include <stdint.h>


/* CONDITIONAL COMPILE */

/* LOGICAL LIMIT */
#define L0007_RACK_SHELF_MAX						16
#define L0007_SHELF_LANE_MAX						16
#define L0007_SHELF_SENSOR_MAX						16
#define L0007_LANE_SENSOR_MAX						8

#define L0007_EVENTID_LEN							128
#define L0007_STOREID_LEN							128
#define L0007_RACKID_LEN							64
#define L0007_LANEID_LEN							64
#define L0007_SHELFID_LEN							64
#define L0007_IDENTIFIER_LEN						64

#define L0007_IPADDRESS_LEN							32
#define L0007_WIRINGMODE_LEN						32
#define L0007_URLLENGTH_MAX							256

/* PHYSICAL LIMIT */

/* DATA SINK UNIT */
#define L0007_DSU_CHANNEL_MAX						10//8
#define L0007_DSU_NODEPERCHNL_MAX					16

/* EVENT CALC UNIT */
#define L0007_ECU_RACK_MAX						24

/* SHELF TYPE */
#define L0007_SHELFCONF_TYPE_REGULAR					0
#define L0007_SHELFCONF_TYPE_HOOK						1
#define L0007_SHELFCONF_TYPE_DISPLACEMENT2			2
#define L0007_SHELFCONF_TYPE_DISPLACEMENT4			3
#define L0007_SHELFCONF_TYPE_JOINT2					4
#define L0007_SHELFCONF_TYPE_JOINT4					5

#define L0007_SHELFTYPE_NAME_MAX		32
#define L0007_SENSORTYPE_NAME_MAX		32

#define L0007_CONFIG_VERSION			200	//this is format version, nothing to do with a0001-0 software version

#define L0007_CONFFLAGS_RACK_ENABLE							1

#define L0007_CONFFLAGS_SHELF_ENABLE					1
#define L0007_CONFFLAGS_SHELF_LOWMEM					2
#define L0007_CONFFLAGS_LANE_ENABLE						1	//rack/shelf/lane enable flags
#define L0007_CONFFLAGS_LANE_HOOK						2
#define L0007_CONFFLAGS_LANE_TRACE						4

#define L0007_FREQDIV_DEFAULT							1

#define L0007_EVENT_TIMETOLIVE							(2 * 60)

#define L0007_HBOARD_SUPPORT_CONFIGSTORAGE_VERSION		6
#define L0007_CBOARD_SUPPORT_CONFIGSTORAGE_VERSION		6

/* 
 * for hisku rackshelf, loadsensor does not map to any lanes where loadsensor's lane_index = 0xFFFFFFFF 
 */

#define L0007_CONFIGSPACE_SIZE				1024
#define L0007_NODETYPE_LOADSENSOR			0
//#define L0007_NODETYPE_GENERNALSENSOR		1		//unknown types of sensors, just pass the data to the CCS
#define L0007_NODETYPE_TEMPSENSOR			1
#define L0007_NODETYPE_COLDCTRBOARD			2

#define L0007_SCANFLAG_FAILED	0
#define L0007_SCANFLAG_SUCCESS 	1

struct l0007_rack_feature{
	uint16_t firmwarever;
	uint16_t hardwarever;
	uint16_t protocolver;
	uint16_t hwid;
	uint32_t confsize;
	uint8_t chip;
	uint8_t rs485count;
	uint8_t ethspeed;	
}__attribute__((packed));

struct l0007_node_feature{
	uint8_t firmwarever;
	uint8_t hardwarever;
	uint16_t hwid;
	uint8_t chip;
	uint8_t peripheral;
	uint16_t confsize;
	uint8_t samplingrata;
	uint8_t weighingrange;
}__attribute__((packed));

struct l0007_lane_conf {
	uint32_t lane_index;
	uint32_t node_index[L0007_LANE_SENSOR_MAX];		//node index this lane used.
	uint32_t nnodes;
	uint32_t flags;
	uint32_t freqdiv_db;	//frequency division for recording the sensor(s) for this lane. e.g. 80 means, save 1 data in db every 80 samples.
	uint8_t laneid[L0007_LANEID_LEN];

	uint8_t scanflag;
	struct l0007_node_feature node_feature[L0007_LANE_SENSOR_MAX];
}__attribute__((packed));

struct l0007_shelf_conf {
	uint32_t shelf_width;
	uint32_t channel_index;
	uint32_t shelf_index;
	uint32_t flags;
	uint8_t shelftype[L0007_SHELFTYPE_NAME_MAX];
	uint8_t shelfid[L0007_SHELFID_LEN];//shelf name in the shop
	uint32_t lanes_num;
	uint32_t lanes_bmp;
	struct l0007_lane_conf laneconf[L0007_SHELF_LANE_MAX];
	uint8_t config[L0007_CONFIGSPACE_SIZE];			//store k&b, for example, for loadsensor. ordered by their node address

	uint8_t scanflag;
}__attribute__((packed));

struct l0007_rack_conf {
	uint32_t rack_index;
	uint32_t flags;
	uint8_t rackid[L0007_RACKID_LEN];
	uint8_t ipaddr[L0007_IPADDRESS_LEN];
	uint32_t shelves_num;
	uint32_t shelves_bmp;
	struct l0007_shelf_conf shelfconf[L0007_RACK_SHELF_MAX];

	uint8_t scanflag;
	struct l0007_rack_feature rackfeature;
}__attribute__((packed));


#endif
