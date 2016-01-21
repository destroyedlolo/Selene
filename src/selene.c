/* Selene : DirectFB framework using Lua
 *
 * 12/04/2015 LF : First version
 * 25/04/2015 LF : Use loadfile() for script
 * 07/06/2015 LF : bump to v0.01 as MQTT is finalized
 * 02/07/2015 LF : add Sleep()
 * 03/07/2015 LF : add WaitFor()
 * 18/09/2015 LF : add SELENE_SCRIPT_* global variables
 * 28/09/2015 LF : v0.03.0 - Add Collection
 * 03/10/2015 LF : v0.04.0 - Subscrition function is not mandatory anymore
 * 24/10/2015 LF : v0.05.0 - Add Layer
 * 26/10/2015 LF : v0.06.0 - Add Window
 * 26/10/2015 LF : v0.07.0 - Add TaskOnce different value
 */

#define _POSIX_C_SOURCE 199309	/* Otherwise some defines/types are not defined with -std=c99 */
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>
#include <assert.h>
#include <libgen.h>
#include <errno.h>
#include <signal.h>

#include "selene.h"
#include "SelShared.h"
#include "directfb.h"
#include "SelTimer.h"
#include "SelMQTT.h"
#include "SelCollection.h"

#define VERSION 0.0701	/* major, minor, sub */

	/*
	 * Utility function
	 */
char *mystrdup(const char *as){	/* as strdup() is missing within C99, grrr ! */
	char *s;
	assert(as);
	assert(s = malloc(strlen(as)+1));
	strcpy(s, as);
	return s;
}

	/*****
	 * Transcodification
	 *****/

int findConst( lua_State *L, const struct ConstTranscode *tbl ){
	const char *arg = luaL_checkstring(L, 1);	/* Get the constant name to retreave */

	for(unsigned int i=0; tbl[i].name; i++){
		if(!strcmp(arg, tbl[i].name)){
			lua_pushnumber(L, tbl[i].value);
			return 1;
		}
	}

	lua_pushnil(L);
	lua_pushstring(L, arg);
	lua_pushstring(L," : Unknown constant");
	lua_concat(L, 2);
	return 2;
}

int rfindConst( lua_State *L, const struct ConstTranscode *tbl ){
	int arg = luaL_checkinteger(L, 1);	/* Get the integer to retrieve */

	for(unsigned int i=0; tbl[i].name; i++){
		if( arg == tbl[i].value ){
			lua_pushstring(L, tbl[i].name);
			return 1;
		}
	}

	lua_pushnil(L);
	lua_pushinteger(L, arg);
	lua_tostring(L, -1);
	lua_pushstring(L," : Unknown constant");
	lua_concat(L, 2);
	return 2;
}

	/*
	 *  Lua stuffs
	 */
lua_State *L;

void clean_lua(void){
	lua_close(L);
}

void *luaL_checkuserdata(lua_State *L, int n){
	if(lua_isuserdata(L, n))
		return lua_touserdata(L, n);
	else {
		luaL_error(L, "parameter %d expected to be a userdata");
		return NULL;
	}
}

#ifdef DEBUG
void dumpstack(lua_State *L){
	int i;
	int top = lua_gettop(L);

	puts("*D* stack");
	for (i = 1; i <= top; i++){
		int t = lua_type(L, i);
		switch(t){
		case LUA_TSTRING:
			printf("`%s'", lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:
			printf(lua_toboolean(L, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:
			printf("%g", lua_tonumber(L, i));
			break;
		default:
			printf("%s", lua_typename(L, t));
			break;
		}
		printf("  ");
	}
	printf("\n");
}
#endif

int SelSleep( lua_State *L ){
	struct timespec ts;
	lua_Number lenght = luaL_checknumber(L, 1);
	ts.tv_sec = (time_t)lenght;
	ts.tv_nsec = (unsigned long int)((lenght - (time_t)lenght) * 1e9);

	nanosleep( &ts, NULL );
	return 0;
}

#ifdef DEBUG
static int _handleToDoList
#else
static int handleToDoList
#endif
( lua_State *L ){ /* Execute functions in the ToDo list */
	for(;;){
		int taskid;
		pthread_mutex_lock( &SharedStuffs.mutex_tl );
		if(SharedStuffs.ctask == SharedStuffs.maxtask){	/* No remaining waiting task */
			pthread_mutex_unlock( &SharedStuffs.mutex_tl );
			break;
		}
		taskid = SharedStuffs.todo[SharedStuffs.ctask++ % SO_TASKSSTACK_LEN];
		pthread_mutex_unlock( &SharedStuffs.mutex_tl );

		if( taskid == LUA_REFNIL)	/* Deleted task */
			continue;

/*		This one is called on the 'main' thread, so there is no need
 *		to protect it
 *
 * 		pthread_mutex_lock( &lua_mutex );
 */
#ifdef DEBUG
printf("*D* todo : %d/%d, tid : %d, stack : %d ",SharedStuffs.ctask,SharedStuffs.maxtask , taskid, lua_gettop(L));
#endif
		lua_rawgeti( L, LUA_REGISTRYINDEX, taskid);
#ifdef DEBUG
printf("-> %d (%d : %d)\n", lua_gettop(L), taskid, lua_type(L, -1) );
#endif
		if(lua_pcall( L, 0, 0, 0 )){	/* Call the trigger without arg */
			fprintf(stderr, "*E* (ToDo) %s\n", lua_tostring(L, -1));
			lua_pop(L, 1); /* pop error message from the stack */
			lua_pop(L, 1); /* pop NIL from the stack */
		}
/*		pthread_mutex_unlock( &lua_mutex ); */
	}

	return 0;
}

#ifdef DEBUG
static int handleToDoList( lua_State *L ){
	puts("handleToDoList ...");
	int ret = _handleToDoList(L);
	puts("handleToDoList : ok");
	return ret;
}
#endif

int SelWaitFor( lua_State *L ){
	unsigned int nsup=0;	/* Number of supervised object (used as index in the table) */
	int nre;				/* Number of received event */
	struct pollfd ufds[WAITMAXFD];
	int maxarg = lua_gettop(L);

	for(int j=1; j <= lua_gettop(L); j++){	/* Stacks SelTimer arguments */
		struct SelTimer *r = luaL_checkudata(L, j, "SelTimer");

		if(nsup == WAITMAXFD){
			lua_pushnil(L);
			lua_pushstring(L, "Exhausting number of waiting FD, please increase WAITMAXFD");
			return 2;
		}

		if(r){	/* We got a SelTimer */
			ufds[nsup].fd = r->fd;
			ufds[nsup++].events = POLLIN;
		}
	}

		/* at least, we have to supervise SharedStuffs' todo list */
	if(nsup == WAITMAXFD){
		lua_pushnil(L);
		lua_pushstring(L, "Exhausting number of waiting FD, please increase WAITMAXFD");
		return 2;
	}

	ufds[nsup].fd = SharedStuffs.tlfd;	/* Push todo list's fd */
	ufds[nsup].events = POLLIN;

	nsup++;

		/* Waiting */
	pthread_mutex_unlock( &lua_mutex );	/* Release parallel Lua tasks */
	if((nre = poll(ufds, nsup, -1)) == -1){	/* Waiting for events */
		pthread_mutex_lock( &lua_mutex );
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	pthread_mutex_lock( &lua_mutex );

	for(int i=0; i<nsup; i++){
		if( ufds[i].revents ){	/* This one has data */
			if( ufds[i].fd == SharedStuffs.tlfd ){ /* Todo list's evenfd */
				uint64_t v;
				if(read( ufds[i].fd, &v, sizeof( uint64_t )) != sizeof( uint64_t ))
					perror("read(eventfd)");
				lua_pushcfunction(L, &handleToDoList);	/*  Push the function to handle the todo list */
			} else for(int j=1; j <= maxarg; j++){
				struct SelTimer *r = luaL_checkudata(L, j, "SelTimer");
				if(r &&  ufds[i].fd == r->fd){
					uint64_t v;
					if(read( ufds[i].fd, &v, sizeof( uint64_t )) != sizeof( uint64_t ))
						perror("read(timerfd)");
					if(r->ifunc != LUA_REFNIL){	/* Immediate function to be executed */
						lua_rawgeti( L, LUA_REGISTRYINDEX, r->ifunc);
						if(lua_pcall( L, 0, 0, 0 )){	/* Call the trigger without arg */
							fprintf(stderr, "*E* (ToDo) %s\n", lua_tostring(L, -1));
							lua_pop(L, 1); /* pop error message from the stack */
							lua_pop(L, 1); /* pop NIL from the stack */
						}
					}
					if(r->task != LUA_REFNIL){	/* Function to be pushed in todo list */
						if( pushtask( r->task, r->once ) ){
							lua_pushstring(L, "Waiting task list exhausted : enlarge SO_TASKSSTACK_LEN");
							lua_error(L);
							exit(EXIT_FAILURE);	/* Code never reached */
						}
					}
				}
			}
		}
	}

	return lua_gettop(L)-maxarg;	/* Number of stuffs to proceed */
}

	/*
	 * Main loop
	 */
static const struct luaL_reg seleneLib[] = {
	{"Sleep", SelSleep},
	{"WaitFor", SelWaitFor},
#ifdef USE_DIRECTFB
		/* Notez-bien : functions bellow have to be generic enough
		 * to be always implemented (and have to use the same API)
		 * whatever the graphical stacks is.
		 */
	{"CooperativeConst", CooperativeConst},
	{"GetDeviceDescription", GetDeviceDescription}, 
	{"SetCooperativeLevel", SetCooperativeLevel},
	{"init", SetCooperativeLevel},	/* Alias for SetCooperativeLevel */
#endif
	{NULL, NULL}    /* End of definition */
};

int main (int ac, char **av){
	char l[1024];

	L = lua_open();		/* opens Lua */
	luaL_openlibs(L);	/* and it's libraries */
	atexit(clean_lua);
	luaL_openlib(L,"Selene", seleneLib, 0);	/* Declare Selene's functions */

	init_shared(L);
	init_SelTimer(L);
	init_SelCollection(L);
#ifdef USE_DIRECTFB
	init_directfb(L, &ac, &av);
#endif
#ifdef USE_MQTT
	init_mqtt(L);
#endif
	lua_pushnumber(L, VERSION);		/* Expose version to lua side */
	lua_setglobal(L, "SELENE_VERSION");

	if(ac > 1){
		char *t = strdup( av[1] );
		assert(t);
		lua_pushstring(L, dirname(t) );
		lua_setglobal(L, "SELENE_SCRIPT_DIR");
		strcpy(t, av[1]);
		lua_pushstring(L, basename(t) );
		lua_setglobal(L, "SELENE_SCRIPT_NAME");

		pthread_mutex_lock( &lua_mutex );
		int err = luaL_loadfile(L, av[1]) || lua_pcall(L, 0, 0, 0);
		if(err){
			fprintf(stderr, "%s", lua_tostring(L, -1));
			lua_pop(L, 1);  /* pop error message from the stack */
			exit(EXIT_FAILURE);
		}
		pthread_mutex_unlock( &lua_mutex );
	} else while(fgets(l, sizeof(l), stdin) != NULL){
		pthread_mutex_lock( &lua_mutex );
		int err = luaL_loadbuffer(L, l, strlen(l), "line") || lua_pcall(L, 0, 0, 0);
		if(err){
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);  /* pop error message from the stack */
		}
		pthread_mutex_unlock( &lua_mutex );
	}

	exit(EXIT_SUCCESS);
}

