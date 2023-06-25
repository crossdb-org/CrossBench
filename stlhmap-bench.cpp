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
g++ stlhmap-bench.cpp -Wall -o stlhmap-bench -lpthread -O3
*/

#include "benchmark.h"

#include <iostream>
#include <unordered_map>


using namespace std;

class RouteKey {
public:
    uint32_t prefix;
    uint8_t mask;

public:
    RouteKey (uint32_t _prefix, uint8_t _mask){
        prefix = _prefix;
        mask = _mask;
    }
    ~RouteKey(){};
    bool operator <(const RouteKey& rt) const
	{
		if ((this->prefix < rt.prefix) || ((this->prefix == rt.prefix) && (this->mask < rt.mask))) {
			return true;
		} else {
			return false;
		}
	}
    bool operator ==(const RouteKey& rt) const
	{
		return (this->prefix == rt.prefix) && (this->mask == rt.mask);
	}
};

class MyHashFunction
{
public:
    // Use sum of lengths of first and last names
    // as hash function.
    size_t operator()(const RouteKey& rt) const
    {
        return rt.prefix ^ rt.mask;
    }
};

static unordered_map<RouteKey,route_t,MyHashFunction> RouteTblHash;
static bool s_inLock = false;


const char* stlhmap_create (char *dbname, dbtype_e type, keytype_e key, uint32_t flags) 
{
	if (!s_quiet && (DB_INMEM != type)) {
		printf ("*** Warning: only support INMEM, use INMEM to do test\n");
	}

	return "STL HashMap";
}

void stlhmap_close () 
{
}

void stlhmap_insert (route_t* pRoute)
{
    RouteKey key(pRoute->prefix, pRoute->mask);
	if(!s_inLock)	rt_wrLock ();
	RouteTblHash[key] = *pRoute;
	if(!s_inLock)	rt_unLock ();
}

bool stlhmap_query (route_t* pRoute)
{
	bool found;
    RouteKey key(pRoute->prefix, pRoute->mask);

	if(!s_inLock)	rt_rdLock ();

    auto iter = RouteTblHash.find (key);
	if (iter != RouteTblHash.end()) {
		found = true;
		*pRoute = iter->second;
	} else {
		printf ("hashmap error: no row found\n");
		found = false;
	}

	if(!s_inLock)	rt_unLock ();
	return found;
}

void stlhmap_delete (route_t* pRoute)
{
    RouteKey key(pRoute->prefix, pRoute->mask);
	if(!s_inLock)	rt_wrLock ();

    RouteTblHash.erase(key);

	if(!s_inLock)	rt_unLock ();
}

bool stlhmap_update (route_t* pRoute)
{
	bool found;
    RouteKey key(pRoute->prefix, pRoute->mask);

	if(!s_inLock)	rt_wrLock ();

	auto iter = RouteTblHash.find (key);
	if (iter != RouteTblHash.end()) {
		found = true;
		iter->second.birth = pRoute->birth;
 	} else {
		printf ("hashmap error: no row found\n");
		found = false;
	}

	if(!s_inLock)	rt_unLock ();
	return found;
}

void stlhmap_begin ()
{
	rt_wrLock ();
	s_inLock = true;
}

void stlhmap_commit ()
{
	rt_unLock ();
	s_inLock = false;
}


#define db_name		"STLHashMap"
#define db_create	stlhmap_create
#define db_close	stlhmap_close
#define db_insert	stlhmap_insert
#define db_query	stlhmap_query
#define db_update	stlhmap_update
#define db_delete	stlhmap_delete
#define db_begin	stlhmap_begin
#define db_commit	stlhmap_commit

#include "benchmark.c"

