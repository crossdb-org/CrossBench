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
gcc -o crossdb-bench crossdb-bench.c ./libcrossdb.so -O3 -Wall
*/

#include "benchmark.h"
#include "crossdb.h"

cross_db_h		hDb;
cross_tbl_h 	hRtTbl;

#define CHECK(ret,str)		if (ret < 0) {	printf (str": %s\n", cross_errMsg(ret)); exit(-1); }

#undef	CROSS_STRUCT_NAME
#define	CROSS_STRUCT_NAME	route_t
cross_field_t	route_schema[] = {
	CROSS_FIELD (prefix,	UINT,	IPv4, 0),
	CROSS_FIELD (mask,		UINT,	DFT,  0),
	CROSS_FIELD (nexthop,	UINT,	IPv4, 0),
	CROSS_FIELD (metric,	UINT,	DFT,  0),
	CROSS_FIELD (intf,		CHAR,	DFT,  0),
	CROSS_FIELD (birth, 	UINT,	TS,   0),
	CROSS_FIELD (flags, 	UINT,	HEX,  0),
	CROSS_END (route_t)
};

cross_fields_h hUpdBirthFlds = NULL;

const char* crossdb_create (char *dbname, dbtype_e type, keytype_e key, uint32_t flags) 
{
	cross_ret 		ret;

	uint32_t dbflags = (DB_ONDISK == type) ? CROSS_ONDISK : 
						((DB_INMEM == type) ? CROSS_INMEM : CROSS_RAMDISK);
	ret = cross_dbCreate (&hDb, dbname, dbflags);
	CHECK (ret, "Failed to create DB: tutorial");

	ret = cross_dbTblCreate (hDb, &hRtTbl, "route", route_schema, "prefix,mask", KEY_TREE == key ? CROSS_RBTREE : 0);
	CHECK (ret, "Failed to create table: route");

	//cross_dbDeleteRows (hRtTbl, NULL, NULL, 0);

	ret = cross_fieldsCreate (hRtTbl, &hUpdBirthFlds, "birth", 0);
	CHECK (ret, "Failed to create fields: birth");
	
	return KEY_TREE == key ? "CrossDB RBTree" : "CrossDB";
}

void crossdb_begin ()
{
	cross_ret ret = cross_dbTransBegin (hDb, 0);
	CHECK (ret, "Failed to begin trans");	
}

void crossdb_commit ()
{
	cross_ret ret = cross_dbTransCommit (hDb, 0);
	CHECK (ret, "Failed to commit trans");
}

void crossdb_close ()
{
	cross_fieldsFree (hUpdBirthFlds);
	cross_ret ret = cross_dbClose (hDb, 0);
	CHECK (ret, "Failed to close DB");
}

void crossdb_insert (route_t *pRoute)
{
	cross_ret ret = cross_dbInsertRow (hRtTbl, pRoute, 0); 
	CHECK (ret, "Failed to insert route");
}

bool crossdb_query (route_t *pRoute)
{
	cross_ret ret = cross_dbGetRowByPK (hRtTbl, pRoute, pRoute, 0); 
	if (CROSS_OK != ret) {
		printf ("crossdb error: status = %d, no row found\n", ret);
		DUMP_RTKEY (*pRoute);
		return false;
	}
	return true;
}

void crossdb_update (route_t *pRoute)
{
	cross_ret ret = cross_dbUpdRowByPK (hRtTbl, pRoute, hUpdBirthFlds, pRoute, 0); 
	CHECK (ret, "Failed to update route");	
}

void crossdb_delete (route_t *pRoute)
{
	cross_ret ret = cross_dbDelRowByPK (hRtTbl, pRoute, 0); 
	CHECK (ret, "Failed to delete route");	
}

#define db_name		"CrossDB"
#define db_create	crossdb_create
#define db_close	crossdb_close
#define db_insert	crossdb_insert
#define db_query	crossdb_query
#define db_update	crossdb_update
#define db_delete	crossdb_delete
#define db_begin	crossdb_begin
#define db_commit	crossdb_commit

#include "benchmark.c"

