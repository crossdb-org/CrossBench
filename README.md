---
template: overrides/blog.html
---

# CrossDB Benchmark

[**CrossBench**](https://github.com/crossdb-org/CrossBench) is an utility designed for evaluating performance of embedded databases.
The goal of this project is to provide a standart and simple in use instrument for benchmarking, so any database developer or user can reference to or repeat obtained results.

- Storage modes: `On-Disk`, `In-Memory`, `On-Ramdisk`
- Benchmarking methods: `insert`, `load`, `query`(top 3 avg), `update`(top 5 avg), `delete`
- Access Sequence: `random`, `sequential`
- Thread: Single thread now, you can bind cpu core to do test
- Supported databases: `crossdb`, `sqlite3`, `lmdb`, `STL map`, `STL hashmap(unordered_map)`


## Usage

```
./crossdb-bench.bin -h
Usage: {count unit can be k or K (*1000)  m or M (*1000000)}
  -h                        show this help
  -s <db storage>           ramdisk | inmem | ondisk, default is ramdisk
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
  -S                        sequential, defaut is random
  -V                        verify driver basic CRUD operations
  -Q                        quiet mode, only show last result
  -H                        show header, if -Q is set, then don't show header

Example:
  default: do ramdisk random mode test, insert 1M rows, query 1M times, update 1M times, delete 1M rows
     -s m: do inmem random mode test,   insert 1M rows, query 1M times, update 1M times, delete 1M rows
     -s d: do ondisk random mode test,  insert 1M rows, query 1M times, update 1M times, delete 1M rows
     -i 500k -q 5m:            do ramdisk random mode test, insert 500K rows, query 5M times, updat 1M times, delete 500k rows
     -i 10k -q 0 -u 0 -d 0:    do ramdisk random mode test, insert 10K rows
     -r 10k -q 100k -u 0 -d 0: do ramdisk random mode test, query  100K times
     -r 10k -q 0 -u 100k -d 0: do ramdisk random mode test, update 100K times
     -r 10k -q 0 -u 0 -d 10k:  do ramdisk random mode test, delete 10K rows
     -V:  verify db driver is ok
```

> **Note**
> `ondisk` DB is stored in current folder `_benchmarkdb`, `ramdisk` DB is stored in `/tmp/_benchmarkdb`.
> If you don't delete all rows in your test, you'd better remember to delete manually or do a simple default test or `./build.sh clean`.


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

## Output Example
```
./crossdb-bench.bin
CrossDB Database Benchmark

Location: /tmp/_benchmarkdb/CrossDB
Type: RAMDISK
Key: Default
Access: Random

Insert rows: 1000000    Use time: 272466us    TPS: 3670182

Query Test: 1000000
  Round: 1      Use Time: 231543us      QPS: 4318852
  Round: 2      Use Time: 244874us      QPS: 4083732
  Round: 3      Use Time: 240435us      QPS: 4159128
  Round: 4      Use Time: 223147us      QPS: 4481350
  Round: 5      Use Time: 221954us      QPS: 4505438
  Round: 6      Use Time: 243468us      QPS: 4107315
  Round: 7      Use Time: 240375us      QPS: 4160166
  Round: 8      Use Time: 240878us      QPS: 4151479
  Round: 9      Use Time: 229140us      QPS: 4364144
  Round: 10     Use Time: 237540us      QPS: 4209817
Update Test: 1000000
  Round: 1      Use Time: 266616us      TPS: 3750712
  Round: 2      Use Time: 281096us      TPS: 3557503
  Round: 3      Use Time: 274047us      TPS: 3649009
  Round: 4      Use Time: 265904us      TPS: 3760755
  Round: 5      Use Time: 260426us      TPS: 3839862
  Round: 6      Use Time: 265945us      TPS: 3760175
  Round: 7      Use Time: 254995us      TPS: 3921645
  Round: 8      Use Time: 274223us      TPS: 3646667
  Round: 9      Use Time: 274394us      TPS: 3644394
  Round: 10     Use Time: 290548us      TPS: 3441772

Delete: 1000000 Use Time: 401207us      TPS: 2492478

CrossDB Benchmark Result: RAMDISK Access=Random Key=Default Rows=1000000
              DB            Rows          Insert           Query          Update          Delete
         CrossDB         1000000         3670182         4450310         3840754         2492478
```

## Benchmark Scripts

You can use following scripts to do benchmark test. The parameter is the cpu core to test which is optional, but to make test reliable, you'd better do test or low load system.

```
./crossdb-benchmark.sh 7
./sqlite-benchmark.sh 7
./lmdb-benchmark.sh 7
./stlmap-benchmark.sh 7
./stlhmap-benchmark.sh 7
```

Output example
```
On-Disk Benchmark Test
              DB            Rows          Insert           Query          Update          Delete
         CrossDB            1000          351741        22158982         2441267         2785515
         CrossDB            1000          651465        22109811         2453643         2865329
         CrossDB            1000          643500        22131423         2436465         2770083
         CrossDB           10000         1523229        19490360         2385504         3036744
         CrossDB           10000         1671402        19781879         2402664         3087372
         CrossDB           10000         1538224        19753758         2383397         3049710
         CrossDB          100000         1923409        14145718         2157846         2595178
         CrossDB          100000         1833483        14079026         2152338         2472615
         CrossDB          100000         1895123        13783290         2145765         1340213
         CrossDB         1000000         1054961         6716753         1471987         1721339
         CrossDB         1000000         1058803         6712856         1464983         1725965
         CrossDB         1000000         1051711         6729894         1469273         1749135
         CrossDB        10000000          598117         4020796         1202939         1172451
         CrossDB        10000000          595118         3992847         1196629         1133848
         CrossDB        10000000          596664         4025642         1189165         1119912
         CrossDB        100000000         429711         3052691          924709          514820
         CrossDB        100000000         394050         2984995         1081168          552174
         CrossDB        100000000         372579         3002053         1084742          630738
On-RamDisk Benchmark Test
              DB            Rows          Insert           Query          Update          Delete
         CrossDB            1000         3134796        22019752        15952443         5076142
         CrossDB            1000         1577287        22070616        16006938         5208333
         CrossDB            1000         1623376        22059753        15972537         5128205
         CrossDB           10000         3095975        19691196        14472099         6165228
         CrossDB           10000         3454231        19662306        14318556         6265664
         CrossDB           10000         2882675        19533775        13952115         6042296
         CrossDB          100000         3547357        14193827        10452799         4989770
         CrossDB          100000         3598934        13253040        10448443         5093205
         CrossDB          100000         3744196        14170861        10174198         5261496
         CrossDB         1000000         3740750         6740538         5310981         3029302
         CrossDB         1000000         3708868         6753996         5488757         3017146
         CrossDB         1000000         3682956         6748722         5491734         2986920
         CrossDB        10000000         1936138         4006575         3570695         1803817
         CrossDB        10000000         2331095         4023692         3568680         1858624
         CrossDB        10000000         2271060         4018655         3566541         2015747
         CrossDB        100000000        1466629         3007293         2734737          802871
         CrossDB        100000000        1467744         2967245         2714797          815151
         CrossDB        100000000        1442414         3004026         2734427          747224
In-Memory Benchmark Test
              DB            Rows          Insert           Query          Update          Delete
         CrossDB            1000         4016064        21937928        15225355         7812500
         CrossDB            1000         1838235        22083004        15972435         7575757
         CrossDB            1000         1461988        22109461        16017004         8333333
         CrossDB           10000         5356186        19717491        14388566         7163323
         CrossDB           10000         5254860        19530617        14567100         7385524
         CrossDB           10000         3285151        19605321        14355210         7047216
         CrossDB          100000         4159733        13979051        10180695         5471656
         CrossDB          100000         3981367        14143811        10259367         5474052
         CrossDB          100000         4039098        13841855        10042771         5144562
         CrossDB         1000000         3907868         6742926         5475764         3044659
         CrossDB         1000000         3972399         6741476         5378830         3184835
         CrossDB         1000000         3963362         6761225         5484965         3167213
         CrossDB        10000000         2489824         3997149         3556671         2038462
         CrossDB        10000000         2480423         4013349         3555828         2070516
         CrossDB        10000000         2486211         3913256         3449931         1956951
         CrossDB        100000000        1696429         2807183         2546983         1468625
         CrossDB        100000000        1789488         2903683         2613381         1480978
         CrossDB        100000000        1783067         2886531         2627970         1492552
```


## New Driver

To make a new xxxdb driver, refer the driver template below.

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
./xxxdb-bench.bin -V
```
