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
gcc -o sqlite3-bench sqlite3-bench.c -lsqlite3 -O3 -Wall
*/

#include "benchmark.h"
#include <sqlite3.h>

static void error_check(int status) {
  if (status != SQLITE_OK) {
    printf("sqlite3 error: status = %d\n", status);
    exit(1);
  }
}

static void exec_error_check(int status, char *err_msg) {
  if (status != SQLITE_OK) {
    printf("SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
    exit(1);
  }
}

static void step_error_check(int status) {
  if (status != SQLITE_DONE) {
    printf("SQL step error: status = %d\n", status);
    exit(1);
  }
}

static sqlite3* pDb = NULL;

static sqlite3_stmt *insert_stmt, *query_stmt, *update_stmt, *delete_stmt, *begin_trans_stmt, *end_trans_stmt;

const char* sqlite_create (char *dbname, dbtype_e type, keytype_e key, uint32_t flags) 
{
	int status;
	char* err_msg = NULL;
	char name[512];

	if (!s_quiet && (KEY_DEFAULT != key) && (KEY_TREE != key)) {
		printf ("*** Warning: Doesn't support key type %s, use default to do test\n", keyTypeStr(key));
	}

	if (DB_INMEM == type) {
		dbname = ":memory:";
	} else {
		sprintf (name, "%s/%s", dbname, "sqlite3bench.db");
		dbname = name;
	}

	status = sqlite3_open(dbname, &pDb);
	if (status) {
		printf("open error: %s\n", sqlite3_errmsg(pDb));
		exit(1);
	}
	char * sqlite3_cfg[] = {
		"PRAGMA synchronous = NORMAL",
		"PRAGMA journal_mode = WAL",
		"PRAGMA temp_store = memory",
		"PRAGMA optimize",
		NULL
	};
	if (DB_ONDISK != type) {
		sqlite3_cfg[0] = "PRAGMA synchronous = OFF";
		if (DB_INMEM == type) {
			sqlite3_cfg[1] = "PRAGMA journal_mode = OFF";
		}
	}

	for (int i = 0; NULL != sqlite3_cfg[i]; ++i) {
		if (!s_quiet) {
			printf ("  %s\n", sqlite3_cfg[i]);
		}
		status = sqlite3_exec(pDb, sqlite3_cfg[i], NULL, NULL, &err_msg);
		exec_error_check(status, err_msg);
	}

	// sqlite3_exec(pDb, "DROP TABLE route", NULL, NULL, &err_msg);
	status = sqlite3_exec(pDb, "CREATE TABLE IF NOT EXISTS route("
									"prefix INT, "
									"mask INT, "
									"nexthop INT, "
									"metric INT, "
									"intf TEXT, "
									"birth INT, "
									"flags INT, "
									"PRIMARY KEY (prefix,mask))", 
							NULL, NULL, &err_msg);
	exec_error_check(status, err_msg);

	const char* insert_str = "INSERT INTO route VALUES (?, ?, ?, ?, ?, ?, ?)";
	const char *query_str = "SELECT * FROM route WHERE prefix = ? AND mask = ?";
	const char *update_str = "UPDATE route SET birth = ? WHERE prefix = ? AND mask = ?";
	const char *delete_str = "DELETE FROM route WHERE prefix = ? AND mask = ?";

	// Preparing sqlite3 statements
	status = sqlite3_prepare_v2(pDb, insert_str, -1, &insert_stmt, NULL);
	error_check(status);
	status = sqlite3_prepare_v2(pDb, query_str, -1, &query_stmt, NULL);
	error_check(status);
	status = sqlite3_prepare_v2(pDb, update_str, -1, &update_stmt, NULL);
	error_check(status);
	status = sqlite3_prepare_v2(pDb, delete_str, -1, &delete_stmt, NULL);
	error_check(status);
	status = sqlite3_prepare_v2(pDb, "BEGIN TRANSACTION", -1, &begin_trans_stmt, NULL);
	error_check(status);
	status = sqlite3_prepare_v2(pDb, "END TRANSACTION", -1, &end_trans_stmt, NULL);
	error_check(status);

	return "SQLite";

}

void sqlite_close ()
{
	int status = sqlite3_finalize(insert_stmt);
	error_check(status);
	status = sqlite3_finalize(query_stmt);
	error_check(status);
	status = sqlite3_finalize(update_stmt);
	error_check(status);
	status = sqlite3_finalize(delete_stmt);
	error_check(status);
	status = sqlite3_finalize(begin_trans_stmt);
	error_check(status);
	status = sqlite3_finalize(end_trans_stmt);
	error_check(status);

	status = sqlite3_close(pDb);
	error_check(status);
}

void sqlite_begin ()
{
	int status;
    // Begin write transaction
	status = sqlite3_step(begin_trans_stmt);
	step_error_check(status);
	status = sqlite3_reset(begin_trans_stmt);
	error_check(status);
}

void sqlite_commit ()
{
	int status;
	// End write transaction
	status = sqlite3_step(end_trans_stmt);
	step_error_check(status);
	status = sqlite3_reset(end_trans_stmt);
	error_check(status);
}

void sqlite_insert (route_t *pRoute)
{
	int status;

	// Bind values into insert_stmt

	status = sqlite3_bind_int64(insert_stmt, 1, pRoute->prefix);
	error_check(status);
	status = sqlite3_bind_int(insert_stmt, 2, pRoute->mask);
	error_check(status);
	status = sqlite3_bind_int64(insert_stmt, 3, pRoute->nexthop);
	error_check(status);
	status = sqlite3_bind_int(insert_stmt, 4, pRoute->metric);
	error_check(status);
	status = sqlite3_bind_text(insert_stmt, 5, pRoute->intf, strlen(pRoute->intf), SQLITE_STATIC);
	error_check(status);
	status = sqlite3_bind_int64(insert_stmt, 6, pRoute->birth);
	error_check(status);
	status = sqlite3_bind_int64(insert_stmt, 7, pRoute->flags);
	error_check(status);

	// Execute insert_stmt
	status = sqlite3_step(insert_stmt);
	step_error_check(status);
	
	// Reset SQLite statement for another use
	status = sqlite3_clear_bindings(insert_stmt);
	error_check(status);
	status = sqlite3_reset(insert_stmt);
	error_check(status);
}

bool sqlite_query (route_t *pRoute)
{
	int status;

	// Bind key value into query_stmt
	status = sqlite3_bind_int64(query_stmt, 1, pRoute->prefix);
	error_check(status);

	status = sqlite3_bind_int(query_stmt, 2, pRoute->mask);
	error_check(status);

	// Execute query_stmt
	if ((status = sqlite3_step(query_stmt)) == SQLITE_ROW) {
		 pRoute->nexthop 	= sqlite3_column_int64 (query_stmt, 2);
		 pRoute->metric 	= sqlite3_column_int (query_stmt, 3);
		 strcpy (pRoute->intf, (const char*)sqlite3_column_text (query_stmt, 4));
		 pRoute->birth 		= sqlite3_column_int64 (query_stmt, 5);
		 pRoute->flags 		= sqlite3_column_int64 (query_stmt, 6);
	} else {
		printf("sqlite3 error: status = %d, no row found\n", status);
		return false;
	}

	// Reset SQLite statement for another use
	status = sqlite3_clear_bindings(query_stmt);
	error_check(status);
	status = sqlite3_reset(query_stmt);
	error_check(status);
	return true;
}

void sqlite_update (route_t *pRoute)
{
	int status;

	// Bind key value into update_stmt
	status = sqlite3_bind_int64(update_stmt, 1, pRoute->birth);
	error_check(status);
	status = sqlite3_bind_int64(update_stmt, 2, pRoute->prefix);
	error_check(status);
	status = sqlite3_bind_int(update_stmt, 3, pRoute->mask);
	error_check(status);

	// Execute update_stmt
	status = sqlite3_step(update_stmt);
	step_error_check(status);

	// Reset SQLite statement for another use
	status = sqlite3_clear_bindings(update_stmt);
	error_check(status);
	status = sqlite3_reset(update_stmt);
	error_check(status);
}

void sqlite_delete (route_t *pRoute)
{
	int status;

	// Bind key value into delete_stmt
	status = sqlite3_bind_int64(delete_stmt, 1, pRoute->prefix);
	error_check(status);
	status = sqlite3_bind_int(delete_stmt, 2, pRoute->mask);
	error_check(status);

	// Execute delete_stmt
	status = sqlite3_step(delete_stmt);
	step_error_check(status);

	// Reset SQLite statement for another use
	status = sqlite3_clear_bindings(delete_stmt);
	error_check(status);
	status = sqlite3_reset(delete_stmt);
	error_check(status);
}


#define db_name		"SQLite"
#define db_create	sqlite_create
#define db_close	sqlite_close
#define db_insert	sqlite_insert
#define db_query	sqlite_query
#define db_update	sqlite_update
#define db_delete	sqlite_delete
#define db_begin	sqlite_begin
#define db_commit	sqlite_commit

#define DFT_ROW_COUNT	100000

#include "benchmark.c"

