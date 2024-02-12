/*
 * Copyright 2022-2023 CrossDB.ORG. All rights reserved.
 *
 *   https://crossdb.org
 *   https://github.com/crossdb-org/crossdb
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __BENCHMRK__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sched.h>
#include <pthread.h>

typedef enum {
	DB_ONDISK,
	DB_INMEM,
	DB_RAMDISK
} dbtype_e;

typedef enum {
	KEY_DEFAULT,
	KEY_TREE,
	KEY_HASH,
	KEY_MAX
} keytype_e;

typedef struct {
	uint32_t 			magicbeg;
	uint32_t 			prefix;
	uint32_t			nexthop;
	uint8_t 			mask;
	uint8_t 			metric;
	char				intf[16];
	uint32_t			birth;
	uint32_t			flags;
	uint32_t 			magicend;
} route_t;

#define IP4ADDR(a,b,c,d)		(uint32_t)((a)<<24|(b)<<16|(c)<<8|(d))
#define IP4STR(ip)				ip>>24,(ip>>16)&0xff,(ip>>8)&0xff,ip&0xff
#define DUMP_ROUTE(route)		printf ("%d.%d.%d.%d/%d->%d.%d.%d.%d intf: %s metric: %d birth: %u flags: 0x%x\n", 	\
									IP4STR(route.prefix), route.mask, IP4STR(route.nexthop), route.intf, route.metric, (uint32_t)route.birth, route.flags)
#define DUMP_RTKEY(route)		printf ("%d.%d.%d.%d/%d\n", IP4STR((route).prefix), (route).mask)
						
pthread_rwlock_t rtTblLock = PTHREAD_RWLOCK_INITIALIZER;	

static inline void rt_rdLock ()
{
	pthread_rwlock_rdlock(&rtTblLock);	
}
static inline void rt_wrLock ()
{
	pthread_rwlock_wrlock(&rtTblLock);	
}

static inline void rt_unLock ()
{
	pthread_rwlock_unlock(&rtTblLock);	
}

static inline const char *keyTypeStr(keytype_e key)
{
	const char *keyStr[] = {"Default", "TREE", "HASH"};
	return key<KEY_MAX ? keyStr[key] : "unknown";
}

extern bool s_quiet;
extern int  row_count;

#endif
