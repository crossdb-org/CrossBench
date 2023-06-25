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
g++ stlmap-bench.cpp -Wall -o stlmap-bench -lpthread -O3
*/

#include "benchmark.h"

#include <iostream>
#include <map>


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
};

static map<RouteKey,route_t> RouteTbl;
static bool s_inLock = false;


const char* stlmap_create (char *dbname, dbtype_e type, keytype_e key, uint32_t flags) 
{
	if (!s_quiet && (DB_INMEM != type)) {
		printf ("*** Warning: only support INMEM, use INMEM to do test\n");
	}

	return "STL Map";
}

void stlmap_close () 
{
}

void stlmap_insert (route_t* pRoute)
{
    RouteKey key(pRoute->prefix, pRoute->mask);
	
	if(!s_inLock)	rt_wrLock ();
	
	RouteTbl[key] = *pRoute;
	
	if(!s_inLock)	rt_unLock ();
}

bool stlmap_query (route_t* pRoute)
{
	bool found;
    RouteKey key(pRoute->prefix, pRoute->mask);
	
	if(!s_inLock)	rt_rdLock ();
	
    auto iter = RouteTbl.find (key);
	if (iter != RouteTbl.end()) {
		found = true;
		*pRoute = iter->second;
	} else {
		printf ("hashmap error: no row found\n");
		found = false;
	}
	
	if(!s_inLock)	rt_unLock ();
	return found;
}

void stlmap_delete (route_t* pRoute)
{
    RouteKey key(pRoute->prefix, pRoute->mask);
	if(!s_inLock)	rt_wrLock ();
    RouteTbl.erase(key);
	if(!s_inLock)	rt_unLock ();
}

bool stlmap_update (route_t* pRoute)
{
	bool found;
    RouteKey key(pRoute->prefix, pRoute->mask);

	if(!s_inLock)	rt_wrLock ();

	auto iter = RouteTbl.find (key);
	if (iter != RouteTbl.end()) {
		found = true;
		iter->second.birth = pRoute->birth;
 	} else {
		printf ("hashmap error: no row found\n");
		found = false;
	}

	if(!s_inLock)	rt_unLock ();
	return found;
}

void stlmap_begin ()
{
	rt_wrLock ();
	s_inLock = true;
}

void stlmap_commit ()
{
	rt_unLock ();
	s_inLock = false;
}


#define db_name		"STLMap"
#define db_create	stlmap_create
#define db_close	stlmap_close
#define db_insert	stlmap_insert
#define db_query	stlmap_query
#define db_update	stlmap_update
#define db_delete	stlmap_delete
#define db_begin	stlmap_begin
#define db_commit	stlmap_commit

#include "benchmark.c"

