#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdint.h>

#include "scandev.h"
#include "netcmd.h"
#include "base.h"

int main(int argc, char *argv[])
{
	printf("generate 11 \n");
	_generate_rackcfg2();
	printf("generate 22 \n");

	struct l0007_rack_conf rack_conf;
	struct scan_rack_feature scan_rack;
	
	_load_rackfeature(&rack_conf, &scan_rack);

	printf("_load_rackfeature !!! \n");
	_save_rackfeature(&rack_conf,  &scan_rack);
	printf("_save_rackfeature !!! \n");
	
	return 0;
}

