#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"


typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	uint32_t value;
	char str[32];

	/* TODO: Add more members if necessary */


} WP;

WP* new_wp();
void free_wp();
bool check_wp();
void info_wp();
void delete_wp();

#endif
