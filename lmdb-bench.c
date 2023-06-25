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

/*
gcc lmdb-bench.c -Wall -o lmdb-bench -llmdb -O3
*/

#include "benchmark.h"
#include "lmdb.h"

typedef struct {
	uint32_t prefix;
	uint8_t mask;
} route_key;


#define CHECK(rc,str)		if (rc != MDB_SUCCESS) {	printf (str": %s\n", mdb_strerror(rc)); exit(-1); }

static MDB_env *env = NULL;
static MDB_dbi dbi;
static MDB_txn *s_txn = NULL;

const char* lmdb_create (char *dbname, dbtype_e type, keytype_e keyType, uint32_t flags) 
{
	int rc = mdb_env_create(&env);
	MDB_txn *txn;

	if (!s_quiet && (DB_INMEM == type)) {
		printf ("*** Warning: Doesn't support INMEM, use RAMDISK to do test\n");
	}

	if (!s_quiet && (KEY_DEFAULT != keyType) && (KEY_TREE != keyType)) {
		printf ("*** Warning: Doesn't support key type %s, use default\n", keyTypeStr(keyType));
	}

	CHECK (rc, "mdb_env_create");
	rc = mdb_env_set_mapsize(env, (uint64_t)row_count * sizeof(route_t) * 32 / 10 + 1024*1024);
	CHECK (rc, "mdb_env_set_mapsize");

	unsigned modeflags = MDB_NOSYNC | MDB_NOMETASYNC | MDB_WRITEMAP | MDB_MAPASYNC | MDB_NORDAHEAD;

	rc = mdb_env_open(env, dbname, modeflags, 0644);
	CHECK (rc, "mdb_env_open");

	rc = mdb_txn_begin(env, NULL, 0, &txn);
	CHECK (rc, "mdb_txn_begin");
	rc = mdb_open(txn, NULL, 0, &dbi);
	CHECK (rc, "mdb_open");

	rc = mdb_txn_commit(txn);
	CHECK (rc, "mdb_txn_commit");
	return "LMDB";
}

void lmdb_close () 
{
	mdb_env_close(env);
}

void lmdb_insert (route_t* pRoute)
{
	MDB_txn *txn;
	MDB_val key, val;
	int	rc;

	if (NULL == s_txn) {
		rc = mdb_txn_begin(env, NULL, 0, &txn);
		CHECK (rc, "mdb_txn_begin");
	}

	route_key rtkey;
	memset (&rtkey, 0, sizeof(rtkey));
	rtkey.prefix = pRoute->prefix;
	rtkey.mask	= pRoute->mask;

	key.mv_data = &rtkey;
	key.mv_size = sizeof(rtkey);

	val.mv_data = pRoute;
	val.mv_size = sizeof (*pRoute);

	rc = mdb_put (txn, dbi, &key, &val, 0);
	CHECK (rc, "mdb_cursor_put");

	if (NULL == s_txn) {
		rc = mdb_txn_commit(txn);
		CHECK (rc, "mdb_txn_commit");
	}
}

bool lmdb_query (route_t* pRoute)
{
	MDB_txn *txn;
	MDB_val key, val;
	int	rc;
	bool	found = false;

	if (NULL == s_txn) {
		rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
		CHECK (rc, "mdb_txn_begin");
	}

	route_key rtkey;
	memset (&rtkey, 0, sizeof(rtkey));
	rtkey.prefix = pRoute->prefix;
	rtkey.mask	= pRoute->mask;

	key.mv_data = &rtkey;
	key.mv_size = sizeof(rtkey);

	rc = mdb_get (txn, dbi, &key, &val);
	if (MDB_SUCCESS == rc) {
		found = true;
	} else {
		printf ("lmdb error: status = %d, no row found\n", rc);
	}

	memcpy (pRoute, val.mv_data, sizeof(*pRoute));

	if (NULL == s_txn) {
		rc = mdb_txn_commit(txn);
		CHECK (rc, "mdb_txn_commit");
	}
	return found;
}

void lmdb_delete (route_t* pRoute)
{
	MDB_txn *txn;
	MDB_val key;
	int	rc;

	if (NULL == s_txn) {
		rc = mdb_txn_begin(env, NULL, 0, &txn);
		CHECK (rc, "mdb_txn_begin");
	}

	route_key rtkey;
	memset (&rtkey, 0, sizeof(rtkey));
	rtkey.prefix = pRoute->prefix;
	rtkey.mask	= pRoute->mask;

	key.mv_data = &rtkey;
	key.mv_size = sizeof(rtkey);

	rc = mdb_del (txn, dbi, &key, NULL);
	CHECK (rc, "mdb_del");

	if (NULL == s_txn) {
		rc = mdb_txn_commit(txn);
		CHECK (rc, "mdb_txn_commit");
	}
}

bool lmdb_update (route_t* pRoute)
{
	MDB_txn *txn;
	MDB_val key, val;
	int	rc;
	bool	found;

	if (NULL == s_txn) {
		rc = mdb_txn_begin(env, NULL, 0, &txn);
		CHECK (rc, "mdb_txn_begin");
	}

	route_key rtkey;
	memset (&rtkey, 0, sizeof(rtkey));
	rtkey.prefix = pRoute->prefix;
	rtkey.mask	= pRoute->mask;

	key.mv_data = &rtkey;
	key.mv_size = sizeof(rtkey);

	rc = mdb_get (txn, dbi, &key, &val);
	if (MDB_SUCCESS == rc) {
		found = true;
	} else {
		printf ("lmdb error: status = %d, no row found\n", rc);
	}
	route_t route;
	memcpy (&route, val.mv_data, sizeof(*pRoute));
	route.birth = pRoute->birth;

	rc = mdb_put (txn, dbi, &key, &val, 0);
	CHECK (rc, "mdb_put");
		
	if (NULL == s_txn) {
		rc = mdb_txn_commit(txn);
		CHECK (rc, "mdb_txn_commit");
	}

	return found;
}

void lmdb_begin ()
{
	int rc = mdb_txn_begin(env, NULL, 0, &s_txn);
	CHECK (rc, "mdb_txn_begin");
}

void lmdb_commit ()
{
	int rc = mdb_txn_commit(s_txn);
	CHECK (rc, "mdb_txn_commit");
	s_txn = NULL;
}


#define db_name		"LMDB"
#define db_create	lmdb_create
#define db_close	lmdb_close
#define db_insert	lmdb_insert
#define db_query	lmdb_query
#define db_update	lmdb_update
#define db_delete	lmdb_delete
#define db_begin	lmdb_begin
#define db_commit	lmdb_commit

#include "benchmark.c"

