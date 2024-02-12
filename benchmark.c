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

typedef struct Random {
  uint32_t seed_;
} Random;

void rand_init(Random* rand_, uint32_t s)
{
  rand_->seed_ = s & 0x7fffffffu;
  // Avoid bad seeds.
  if (rand_->seed_ == 0 || rand_->seed_ == 2147483647L) {
    rand_->seed_ = 1;
  }
}

uint32_t rand_next(Random* rand_)
{
  static const uint32_t M = 2147483647L;
  static const uint64_t A = 16807;

  uint64_t product = rand_->seed_ * A;

  rand_->seed_ = (uint32_t)((product >> 31) + (product & M));

  if (rand_->seed_ > M) {
    rand_->seed_ -= M;
  }

  return rand_->seed_;
}

static uint64_t timestamp_us ()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000LL + tv.tv_usec;
}

static inline uint32_t get_next_rand (Random *pRand, uint8_t *pMark, uint32_t max)
{
	uint32_t id;
	while (pMark[(id = (rand_next(pRand) % max))])
		;
	pMark[id] = 1;
	return id;
}

static int int_cmp (const void *pL, const void *pR)
{
	int left = *(int*)pL, right = *(int*)pR;
	return (left == right) ? 0 : ((left > right) ? -1 : 1);
}

#ifndef DFT_ROW_COUNT
#define DFT_ROW_COUNT	1000000
#endif

bool s_quiet = false;
int  row_count = 0;

int static inline count_convert(const char *arg)
{
	int count = atoi (optarg);
	switch (toupper(arg[strlen(arg)-1])) {
	case 'K':	return count * 1000;
	case 'M':	return count * 1000000;
	}
	return count;
}

int main (int argc, char **argv)
{
	int 	i;
	char 	ch;
	uint64_t 		ts;
	route_t 		route;	
	dbtype_e 		type = DB_RAMDISK;
	keytype_e		key = KEY_DEFAULT;
	int 			load_count = 0;
	int 			insert_count = DFT_ROW_COUNT;
	int 			query_count  = DFT_ROW_COUNT;
	int 			update_count = DFT_ROW_COUNT;
	int 			delete_count = DFT_ROW_COUNT;
	int				cpuid = -1;

	bool			sequential = false, verify = false, header = false;
	uint32_t		insert_tps = 0, load_tps = 0, query_qps = 0, update_tps = 0, delete_tps = 0;

	while ((ch = getopt(argc, argv, "s:k:l:i:q:u:d:r:c:SVQHh")) != -1) {
		switch (ch) {
		case 'h':
			printf ("Usage: {count unit can be k or K (*1000)  m or M (*1000000)}\n");
            printf ("  -h                        show this help\n");
            printf ("  -s <db storage>           ramdisk | inmem | ondisk, default is ramdisk\n");
            printf ("                            shortcut r: ramdisk  m: inmem    d: ondisk\n");
            printf ("                            ondisk store db in current folder crossbench\n");
            printf ("  -i <insert count>         default %d, do insert test, conflict with -l load\n", insert_count);
            printf ("  -r <row count>            open db which already has these rows\n");        
            printf ("  -q <query count>          default %d, do query test\n",  query_count);
            printf ("  -u <update count>         default %d, do update test\n", update_count);
            printf ("  -d <delete count>         default %d, do delete test\n", delete_count);
            printf ("  -c <cpu core>             bind cpu core\n");
            printf ("  -l <load count>           quick load data, conflict with -i insert\n");        
            printf ("  -k <key type>             default | hash | tree\n");
            printf ("                            shortcut h: hash  t: tree    d: default\n");
            printf ("  -S                        sequential, defaut is random\n");
            printf ("  -V                        verify driver basic CRUD operations\n");
            printf ("  -Q                        quiet mode, only show last result\n");
            printf ("  -H                        show header, if -Q is set, then don't show header\n");
			printf ("\nExample:\n");
            printf ("  default: do ramdisk random mode test, insert 1M rows, query 1M times, update 1M times, delete 1M rows\n");
            printf ("     -s m: do inmem random mode test,   insert 1M rows, query 1M times, update 1M times, delete 1M rows\n");
            printf ("     -s d: do ondisk random mode test,  insert 1M rows, query 1M times, update 1M times, delete 1M rows\n");
            printf ("     -i 500k -q 5m:            do ramdisk random mode test, insert 500K rows, query 5M times, updat 1M times, delete 500k rows\n");
            printf ("     -i 10k -q 0 -u 0 -d 0:    do ramdisk random mode test, insert 10K rows\n");
            printf ("     -r 10k -q 100k -u 0 -d 0: do ramdisk random mode test, query  100K times\n");
            printf ("     -r 10k -q 0 -u 100k -d 0: do ramdisk random mode test, update 100K times\n");
            printf ("     -r 10k -q 0 -u 0 -d 10k:  do ramdisk random mode test, delete 10K rows\n");
            printf ("     -V:  verify db driver is ok\n");
			return -1;
		case 's':
			if (('m' == optarg[0]) || !strcmp (optarg, "inmem")) { 
				type = DB_INMEM;
			} else if (('d' == optarg[0]) || !strcmp (optarg, "ondisk")) { 
				type = DB_ONDISK; 
			}
			break;
		case 'k':
			if (('h' == optarg[0]) || !strcmp (optarg, "hash")) { 
				key = KEY_HASH;
			} else if (('t' == optarg[0]) || !strcmp (optarg, "tree")) { 
				key = KEY_TREE;
			}
			break;
		case 'r':
			row_count = count_convert (optarg);
			load_count = 0;
			insert_count = 0;
			delete_count = 0;
			break;
		case 'l':
			load_count = count_convert (optarg);
			row_count = 0;
			insert_count = 0;
			delete_count = 0;
			break;
		case 'i':
			insert_count = count_convert (optarg);
			delete_count = insert_count;
			load_count = 0;
			row_count = 0;
			break;
		case 'q':
			query_count = count_convert (optarg);
			break;
		case 'u':
			update_count = count_convert (optarg);
			break;
		case 'd':
			delete_count = count_convert (optarg);
			break;
		case 'c':
			cpuid = atoi (optarg);
			break;
		case 'S':
			sequential = true;
			break;
		case 'V':
			verify = true;
		case 'Q':
			s_quiet = true;
			break;
		case 'H':
			header = true;
			break;
		}
	}

	if (load_count > 0 && insert_count > 0) {
		printf ("Both load data and insert test are set, only one can be set\n");
		return -1;
	}

	// dbname
	char name[256], cmd[512];
 	if (DB_ONDISK == type) {
		sprintf (name, "_benchmarkdb/%s", db_name);
		if (0 == row_count) {
			sprintf (cmd, "rm -rf %s", name);
			int rc = system (cmd);
			(void)rc;
		}
		sprintf (cmd, "mkdir -p %s", name);
		int rc = system (cmd);
		(void)rc;
	} else {
		sprintf (name, "/tmp/_benchmarkdb/%s", db_name);
		if (0 == row_count) {
			sprintf (cmd, "rm -rf %s", name);
			int rc = system (cmd);
			(void)rc;
		}
		sprintf (cmd, "mkdir -p %s", name);
		int rc = system (cmd);
		(void)rc;
	}

	if (0 == row_count) {
		row_count = load_count + insert_count;
	}
	if (0 == row_count) {
		if (header) {
			printf ("%16s\t%8s\t%8s\t%8s\t%8s\t%8s\n", "DB", "Rows", "Insert", "Query", "Update", "Delete");
			return 0;
		}
		
		printf ("Both load data and insert test are not set, only one can be set\n");
		return -1;
	}
	if (delete_count > row_count) {
		delete_count = row_count;
	}

	srand (time(NULL));
	
	// rand init
	uint8_t *pRandMark = (uint8_t*)malloc (row_count);
	if (NULL == pRandMark) {
		return -1;
	}
	Random rand;
	rand_init (&rand, time(NULL));

	const char *type_str[] = { "ONDISK", "INMEM", "RAMDISK", };

	if (!s_quiet) {
		printf ("%s Database Benchmark\n\n", db_name);
		printf ("Location: %s\nType: %s\nKey: %s\nAccess: %s\n", name, 
					type_str[type], keyTypeStr(key), sequential?"Sequential":"Random");
	}

	const char *dbname = db_create (name, type, key, 0);

	if (NULL == dbname) {
		dbname = db_name;
	}

	// init route
	uint32_t prefix = IP4ADDR(10,1,1,0);
	route.prefix	= prefix;
	route.mask		= 24;	
	route.nexthop	= IP4ADDR(10,1,1,254);
	route.metric	= 1;
	route.flags 	= 0;
	strcpy (route.intf, "eth1");
	route.birth 	= time (NULL);

	if (verify) {
		printf ("------ Verify %s ------\n", dbname);
		printf ("Insert route\n");
		db_insert (&route);

		printf ("Query route\n");
		memset (&route, 0, sizeof(route));
		route.prefix	= prefix;
		route.mask		= 24;			
		if (!db_query (&route)) {
			printf ("Didn't get route\n");
			return -1;
		}
		DUMP_ROUTE (route);

		printf ("Update route\n");
		route.birth += 10;
		time_t birth = route.birth;
		db_update (&route);

		printf ("Query route\n");
		memset (&route, 0, sizeof(route));
		route.prefix	= prefix;
		route.mask		= 24;			
		if (!db_query (&route)) {
			printf ("Didn't get route\n");
			return -1;
		}
		DUMP_ROUTE (route);
		if (route.birth != birth) {
			printf ("db.birth %u != %u\n", (uint32_t)route.birth, (uint32_t)birth);
			return -1;
		}

		printf ("Delete route\n");
		db_delete (&route);
		printf ("Query route\n");
		if (db_query (&route)) {
			printf ("Route was not deleted\n");
			DUMP_ROUTE (route);
		}

		printf ("------ Verify Done ------\n");

		db_close ();
		return 0;
	}

	if (cpuid >= 0) {
		cpu_set_t  mask;
		CPU_ZERO(&mask);
		CPU_SET(cpuid, &mask);
		if (sched_setaffinity(0, sizeof(mask), &mask) < 0) {
			perror ("sched_setaffinity");
			return -1;
		}
		if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
			perror("pthread_setaffinity_np");
		}
		if (!s_quiet) {
			printf ("\nRun test on CPU %d\n", cpuid);
		}
	}

	if (load_count > 0) {
		ts = timestamp_us();
		db_begin ();
		for (i = 0; i < row_count; ++i) {
			db_insert (&route);
			route.prefix++;
		}
		db_commit ();
		ts = timestamp_us() - ts;
		load_tps = row_count*1000000LL/ts;
		if (!s_quiet) {
			printf ("\nLoad rows: %d    Use time: %uus    TPS: %d\n\n", row_count, (uint32_t)ts, load_tps);
		}
	} 

	if (insert_count > 0) {
		memset (pRandMark, 0, row_count);
		ts = timestamp_us();
		for (i = 0; i < row_count; ++i) {
			if (!sequential) {
				route.prefix = prefix + get_next_rand(&rand, pRandMark, row_count);
			} else {
				route.prefix = prefix + i;
			}
			db_insert (&route);
		}
		ts = timestamp_us() - ts;
		insert_tps = row_count*1000000LL/ts;
		if (!s_quiet) {
			printf ("\nInsert rows: %d    Use time: %uus    TPS: %d\n\n", row_count, (uint32_t)ts, insert_tps);
		}
	}

	if (query_count > 0) {
		int 			qps[10];
		if (!s_quiet) {
			printf ("Query Test: %d\n", query_count);
		}
		for (int r = 0; r < 10; ++r) {
			ts = timestamp_us();
			route.mask		= 24;
			for (i = 0; i < query_count; ++i) {
				if (!sequential) {
					route.prefix = prefix +  (rand_next(&rand) % row_count);
				} else {
					route.prefix = prefix + (i % row_count);
				}
				db_query (&route);
			}
			ts = timestamp_us() - ts;
			qps[r] = (int)((int64_t)i*1000000/ts);
			if (!s_quiet) {
				printf ("  Round: %d\tUse Time: %uus\tQPS: %d\n", r+1, (uint32_t)ts, qps[r]);
			}
		}
		qsort (qps, 10, sizeof(qps[0]), int_cmp);
		query_qps = (qps[0] + qps[1] + qps[2]) / 3;
	}

	if (update_count > 0) {
		int tps[10];
		if (!s_quiet) {
			printf ("Update Test: %d\n", update_count);
		}
		uint32_t	id = time(NULL);
		for (int r = 0; r < 10; ++r) {
			memset (pRandMark, 0, row_count);
			ts = timestamp_us();
			route.mask		= 24;
			for (i = 0; i < update_count; ++i) {
				route.birth 	= id++;
				if (!sequential) {
					route.prefix = prefix + (rand_next(&rand) % row_count);
				} else {
					route.prefix = prefix + (i % row_count);
				}
				db_update (&route);
			}
			ts = timestamp_us() - ts;
			tps[r] = update_count*1000000LL/ts;
			if (!s_quiet) {
				printf ("  Round: %d\tUse Time: %uus\tTPS: %d\n", r+1, (uint32_t)ts, tps[r]);
			}
		}
		qsort (tps, 10, sizeof(tps[0]), int_cmp);
		update_tps = (tps[0] + tps[1] + tps[2] + tps[3] + tps[4]) / 5;
	}

	if (delete_count > 0) {
		memset (pRandMark, 0, row_count);
		ts = timestamp_us();
		route.mask		= 24;
		for (i = 0; i < delete_count; ++i) {
			if (!sequential) {
				route.prefix = prefix + get_next_rand(&rand, pRandMark, row_count);
			} else {
				route.prefix = prefix + (i % row_count);
			}
			db_delete (&route);
		}
		ts = timestamp_us() - ts;
		delete_tps = delete_count*1000000LL/ts;
		if (!s_quiet) {
			printf ("\nDelete: %d\tUse Time: %uus\tTPS: %d\n", i, (uint32_t)ts, delete_tps);
		}
	}

	if (!s_quiet) {
		printf ("\n");
		printf ("%s Benchmark Result: %s Access=%s Key=%s Rows=%d\n", 
						db_name, type_str[type], sequential?"Sequential":"Random", keyTypeStr(key), row_count);
	}
	if (!s_quiet || !header) {
		printf ("%16s\t%8s\t%8s\t%8s\t%8s\t%8s\n", "DB", "Rows", insert_tps?"Insert":"Load", "Query", "Update", "Delete");
	}
	printf ("%16s\t%8d\t%8d\t%8d\t%8d\t%8d\n", dbname, row_count, insert_tps+load_tps, query_qps, update_tps, delete_tps);

	db_close ();

	return 0;
}

