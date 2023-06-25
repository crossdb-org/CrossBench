**CrossBench** is an utility designed for evaluating performance of embedded databases.
The goal of this project is to provide a standart and simple in use instrument for benchmarking, so any database developer or user can reference to or repeat obtained results.

Storage modes: `on disk`, `in memory`, `on ramdisk`

Benchmarking methods: `insert`, `load`, `query`, `update`, `delete`

Access Sequence: `random`, `sequential`

Supported databases: `crosdb`, `sqlite3`, `lmdb`, `STL map`, `STL hashmap(unordered_map)`


## Usage

```
./crossdb-bench.bin -h
Usage: {count unit can be k or K (*1000)  m or M (*1000000)}
  -h                        show this help
  -t <db type>              ramdisk | inmem | ondisk, default is ramdisk
                            shortcut r: ramdisk  m: inmem    d: ondisk
                            ondisk store db in current folder crossbench
  -i <insert count>         default 1000000, do insert test, conflict with -l load
  -r <row count>            open db which already has these rows
  -q <query count>          default 1000000, do query test
  -u <update count>         default 1000000, do update test
  -d <delete count>         default 1000000, do delete test
  -c <cpu core>             bind cpu core
  -l <load count>           quick load data, conflict with -i insert
  -k <key type>             default | hash | tree
                            shortcut h: hash  t: tree    d: default
  -s                        sequential, defaut is random
  -V                        verify driver basic CRUD operations
  -Q                        quiet mode, only show last result

Example:
  default: do ramdisk random mode test, insert 1M rows, query 1M times, update 1M times, delete 1M rows
     -t m: do inmem random mode test,   insert 1M rows, query 1M times, update 1M times, delete 1M rows
     -t d: do ondisk random mode test,  insert 1M rows, query 1M times, update 1M times, delete 1M rows
     -i 500k -q 5m:            do ramdisk random mode test, insert 500K rows, query 5M times, updat 1M times, delete 500k rows
     -i 10k -q 0 -u 0 -d 0:    do ramdisk random mode test, insert 10K rows
     -r 10k -q 100k -u 0 -d 0: do ramdisk random mode test, query  100K times
     -r 10k -q 0 -u 100k -d 0: do ramdisk random mode test, update 100K times
     -r 10k -q 0 -u 0 -d 10k:  do ramdisk random mode test, delete 10K rows
     -V:  verify db driver is ok
```


## Build

Install `sqlite3` and `lmdb`
```
sudo apt install libsqlite3-dev
sudo apt install liblmdb-dev
```
Install [CrossDB Library](https://github.com/xxxdb-org/CrossDB/releases)
```
sudo cp libxxxdb.so /usr/lib/
```
Build
```
./build.sh
```


## New Driver

To make a new xxxdb driver, following the simple template below.

```c
#include "benchmark.h"

const char* xxxdb_create (char *dbname, dbtype_e type, keytype_e key, uint32_t flags) 
{
	// credata database or table, Primary Key: prefix+mask
}

void xxxdb_close ()
{
	// close database
}

void xxxdb_begin ()
{
	// begin transaction
}

void xxxdb_commit ()
{
	// commit transaction
}

void xxxdb_insert (route_t *pRoute)
{
	// insert route
}

bool xxxdb_query (route_t *pRoute)
{
	// query route by Primary Key
}

void xxxdb_update (route_t *pRoute)
{
	// update route set birth by Primary Key
}

void xxxdb_delete (route_t *pRoute)
{
	// delete route by Primary Key
}

#define db_name		"xxxDB"
#define db_create	xxxdb_create
#define db_close	xxxdb_close
#define db_insert	xxxdb_insert
#define db_query	xxxdb_query
#define db_update	xxxdb_update
#define db_delete	xxxdb_delete
#define db_begin	xxxdb_begin
#define db_commit	xxxdb_commit

#include "benchmark.c"

```

Verify the driver can work
```
./xxxdb-bench -V
```
