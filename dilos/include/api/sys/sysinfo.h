#ifndef _SYS_SYSINFO_H
#define _SYS_SYSINFO_H

#include <osv/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SI_LOAD_SHIFT 16

struct sysinfo {
	unsigned long uptime;
	unsigned long loads[3];
	unsigned long totalram;
	unsigned long freeram;
	unsigned long sharedram;
	unsigned long bufferram;
	unsigned long totalswap;
	unsigned long freeswap;
	u16 procs, pad;
	unsigned long totalhigh;
	unsigned long freehigh;
	u32 mem_unit;
};

int sysinfo (struct sysinfo *);
int get_nprocs_conf (void);
int get_nprocs (void);
long get_phys_pages (void);
long get_avphys_pages (void);

#ifdef __cplusplus
}
#endif

#endif
