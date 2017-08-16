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
 * 24/01/2016 LF : v0.08.0 - Add watchdog to MQTT subscriptions
 * 12/04/2016 LF : switch to v1.0.0
 * 16/04/2016 LF : switch to v2.0.0 - DirectFB is now a plugin
 * 22/04/2016 LF : Remove lua_mutex (not used as MQTT's function use their own state)
 * 04/05/2016 LF : Add Detach()
 * 05/09/2016 LF : switch to v3.0.0 - Add Curses plugin
 * 19/09/2016 LF : v3.1.0 - WaitFor can wait for io stream
 * 03/12/2016 LF : v3.2.0 - SelLog can log on stdout as well
 * 14/12/2016 LF : v3.3.0 - can use unset timer
 * 31/12/2016 LF : v3.4.0 - DFB : Add toSurface() to Image
 * 			  LF : v3.5.0 - DFB : Add blitting
 * 01/12/2016 LF : v3.6.0 - DFB : add clone()/restore()
 * 			  LF : v3.7.0 - DFB : add SetClip() and SurfaceTileBlitClip()
 * 04/02/2017 LF : v3.8.0 - DFB : add DrawCircle()
 * 08/02/2017 LF : v3.9.0 - DFB : add PixelFormat()
 * 11/03/2017 LF : v3.10.0 - DFB : add FillGradient()
 * 24/03/2017 LF : v3.11.0 - Collection : add HowMany()
 * 25/03/2017 LF : v3.12.0 - DFB : Add SetRenderOptions()
 * 06/04/2017 LF : v3.13.0 - DFB : Add GetAfter() & GetBelow()
 * 10/04/2017 LF : v3.14.0 - Add SelTimedCollection
 * 15/04/2017 LF : v3.15.0 - Add SigIntTask()
 * 15/04/2017 LF : v3.16.0 - Add Save() and Load() to SelTimedCollection
 * 						   - SigIntTask() handles SIGUSR1 as well
 * 16/04/2017 LF : v3.17.0 - DFB : add PixelFormat to windows
 * 24/04/2017 LF : v3.18.0 - Add SelEvent
 * 05/06/2017 LF : v3.19.0 - Add SelTimedWindowCollection
 * 16/06/2017 LF : v3.20.0 - Add SelFIFO
 * 16/08/2017 LF : v3.21.0 - Create arg array as Lua is doing
 */

#define _POSIX_C_SOURCE 199309	/* Otherwise some defines/types are not defined with -std=c99 */
#define _XOPEN_SOURCE 500	/* Otherwise gethostname() is not defined */

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
#include <stdbool.h>
#include <inttypes.h>	/* uint64_t */
#include <dlfcn.h>		/* dlopen(), ... */

#include "selene.h"
#include "SelShared.h"
#include "SelPlugins/DirectFB/directfb.h"
#include "SelPlugins/Curses/curses.h"
#include "SelTimer.h"
#include "SelMQTT.h"
#include "SelCollection.h"
#include "SelTimedCollection.h"
#include "SelTimedWindowCollection.h"
#include "SelFIFO.h"
#include "SelLog.h"
#include "SelEvent.h"

#define VERSION 3.2100	/* major, minor, sub */

#ifndef PLUGIN_DIR
#	define PLUGIN_DIR	"/usr/local/lib/Selene"
#endif

pthread_attr_t thread_attr;

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

int hash( const char *s ){	/* Calculate the hash code of a string */
	int r = 0;
	for(; *s; s++)
		r += *s;
	return r;
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
	 * Signal handling
	 */

static int sigfunc = LUA_REFNIL;	/* Function to call in case of SIG_INT */

static void sighandler(){
	if( sigfunc != LUA_REFNIL )
		pushtask( sigfunc, true );
}

static int SelSigIntTask(lua_State *L){
	if( lua_type(L, -1) == LUA_TFUNCTION ){
		sigfunc = findFuncRef(L,lua_gettop(L));

		signal(SIGINT, sighandler);
		signal(SIGUSR1, sighandler);
	}
	return 0;
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

static void *checkUData(lua_State *L, int ud, const char *tname){
/* Like luaL_checkudata() but w/o crashing if doesn't march
 * From luaL_checkudata() source code
 */
	void *p = lua_touserdata(L, ud);
	if(p){
		if(lua_getmetatable(L, ud)){  /* does it have a metatable? */
			lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
			if(!lua_rawequal(L, -1, -2))  /* does it have the correct mt ? */
				p = NULL;	/* No */
			lua_pop(L, 2);  /* remove both metatables */
			return p;
		}
	}
	return NULL;	/* Not an user data */
}

static int SelWaitFor( lua_State *L ){
	unsigned int nsup=0;	/* Number of supervised object (used as index in the table) */
	int nre;				/* Number of received event */
	struct pollfd ufds[WAITMAXFD];
	int maxarg = lua_gettop(L);

	for(int j=1; j <= lua_gettop(L); j++){	/* Stacks SelTimer arguments */
		if(nsup == WAITMAXFD){
			lua_pushnil(L);
			lua_pushstring(L, "Exhausting number of waiting FD, please increase WAITMAXFD");
			return 2;
		}

		void *r;
		if((r = checkUData(L, j, "SelTimer"))){	/* We got a SelTimer */
			ufds[nsup].fd = ((struct SelTimer *)r)->fd;
			ufds[nsup++].events = POLLIN;
		} else if(( r = checkUData(L, j, "SelEvent"))){
			ufds[nsup].fd = ((struct SelEvent *)r)->fd;
			ufds[nsup++].events = POLLIN;
		} else if(( r = checkUData(L, j, LUA_FILEHANDLE))){	/* We got a file */
			ufds[nsup].fd = fileno(*((FILE **)r));
			ufds[nsup++].events = POLLIN;
		} else {
			lua_pushnil(L);
			lua_pushstring(L, "Unsupported type for WaitFor()");
			return 2;
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
	if((nre = poll(ufds, nsup, -1)) == -1){	/* Waiting for events */
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	for(int i=0; i<nsup; i++){
		if( ufds[i].revents ){	/* This one has data */
			if( ufds[i].fd == SharedStuffs.tlfd ){ /* Todo list's evenfd */
				uint64_t v;
				if(read( ufds[i].fd, &v, sizeof( uint64_t )) != sizeof( uint64_t ))
					perror("read(eventfd)");
				lua_pushcfunction(L, &handleToDoList);	/*  Push the function to handle the todo list */
			} else for(int j=1; j <= maxarg; j++){
				void *r;
				if((r=checkUData(L, j, "SelTimer"))){
					if(ufds[i].fd == ((struct SelTimer *)r)->fd){
						uint64_t v;
						if(read( ufds[i].fd, &v, sizeof( uint64_t )) != sizeof( uint64_t ))
							perror("read(timerfd)");
						if(((struct SelTimer *)r)->ifunc != LUA_REFNIL){	/* Immediate function to be executed */
							lua_rawgeti( L, LUA_REGISTRYINDEX, ((struct SelTimer *)r)->ifunc);
							if(lua_pcall( L, 0, 0, 0 )){	/* Call the trigger without arg */
								fprintf(stderr, "*E* (SelTimer ifunc) %s\n", lua_tostring(L, -1));
								lua_pop(L, 1); /* pop error message from the stack */
								lua_pop(L, 1); /* pop NIL from the stack */
							}
						}
						if(((struct SelTimer *)r)->task != LUA_REFNIL){	/* Function to be pushed in todo list */
							if( pushtask( ((struct SelTimer *)r)->task, ((struct SelTimer *)r)->once ) ){
								lua_pushstring(L, "Waiting task list exhausted : enlarge SO_TASKSSTACK_LEN");
								lua_error(L);
								exit(EXIT_FAILURE);	/* Code never reached */
							}
						}
					}
				} else if((r=checkUData(L, j, "SelEvent"))){
					if(ufds[i].fd == ((struct SelEvent *)r)->fd){
						if( pushtask( ((struct SelEvent *)r)->func, false) ){
							lua_pushstring(L, "Waiting task list exhausted : enlarge SO_TASKSSTACK_LEN");
							lua_error(L);
							exit(EXIT_FAILURE);	/* Code never reached */
						}
					}
				} else if(( r = checkUData(L, j, LUA_FILEHANDLE))){
					if(ufds[i].fd == fileno(*((FILE **)r)))
						lua_pushvalue(L, j);
				}
			}
		}
	}

	return lua_gettop(L)-maxarg;	/* Number of stuffs to proceed */
}

static void *launchfunc(void *arg){
	if(lua_pcall( (lua_State *)arg, 0, 1, 0))
		fprintf(stderr, "*E* (launch) %s\n", lua_tostring((lua_State *)arg, -1));
	
	lua_close((lua_State *)arg);
	return NULL;
}

int SelDetach( lua_State *L ){
	if(lua_type(L, 1) != LUA_TFUNCTION ){
		lua_pushnil(L);
		lua_pushstring(L, "Task needed as 1st argument of Selene.Detach()");
		return 2;
	}

	pthread_t tid;	/* No need to be kept */
	lua_State *tstate = luaL_newstate(); /* Initialise new state for the thread */
	assert(tstate);
	luaL_openlibs( tstate );
	init_shared_Lua( tstate );
	init_SelFIFO( tstate );
	lua_xmove( L, tstate, 1 );

	if(pthread_create( &tid, &thread_attr, launchfunc,  tstate) < 0){
		fprintf(stderr, "*E* Can't create a new thread : %s\n", strerror(errno));
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	return 0;
}

int SelHostname( lua_State *L ){
	char n[HOST_NAME_MAX];
	gethostname(n, HOST_NAME_MAX);

	lua_pushstring(L, n);
	return 1;
}

	/*
	 * Dynamically add Pluggins
	 */
#ifdef USE_DIRECTFB
int UseDirectFB( lua_State *L ){
	void *pgh;
	void (*func)( lua_State * );

	if(!(pgh = dlopen(PLUGIN_DIR "/SelDirectFB.so", RTLD_LAZY))){
		fprintf(stderr, "Can't load plug-in : %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror(); /* Clear any existing error */

	if(!(func = dlsym( pgh, "init_directfb" ))){
		fprintf(stderr, "Can't find plug-in init function : %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	(*func)( L );

	return 0;
}
#endif

#ifdef USE_CURSES
	int UseCurses( lua_State *L ){
	void *pgh;
	void (*func)( lua_State * );

	if(!(pgh = dlopen(PLUGIN_DIR "/SelCurses.so", RTLD_LAZY))){
		fprintf(stderr, "Can't load plug-in : %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror(); /* Clear any existing error */

	if(!(func = dlsym( pgh, "init_curses" ))){
		fprintf(stderr, "Can't find plug-in init function : %s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	(*func)( L );

	return 0;
}
#endif
	/*
	 * Main loop
	 */
static const struct luaL_reg seleneLib[] = {
	{"Sleep", SelSleep},
	{"WaitFor", SelWaitFor},
	{"Detach", SelDetach},
	{"SigIntTask", SelSigIntTask},
#ifdef USE_DIRECTFB
	{"UseDirectFB", UseDirectFB},
#endif
#ifdef USE_CURSES
	{"UseCurses", UseCurses},
#endif
	{"Hostname", SelHostname},
	{NULL, NULL}    /* End of definition */
};

int main (int ac, char **av){
	char l[1024];

		/* Creates threads as detached in order to save
		 * some resources when quiting
		 */
	assert(!pthread_attr_init (&thread_attr));
	assert(!pthread_attr_setdetachstate (&thread_attr, PTHREAD_CREATE_DETACHED));

	L = lua_open();		/* opens Lua */
	luaL_openlibs(L);	/* and it's libraries */
	atexit(clean_lua);
	luaL_openlib(L,"Selene", seleneLib, 0);	/* Declare Selene's functions */

	init_shared(L);
	init_SelTimer(L);
	init_SelCollection(L);
	init_SelTimedCollection(L);
	init_SelTimedWindowCollection(L);
	init_SelFIFO(L);
	init_log(L);
	init_SelEvent(L);
#ifdef USE_MQTT
	init_mqtt(L);
#endif

	lua_pushnumber(L, VERSION);		/* Expose version to lua side */
	lua_setglobal(L, "SELENE_VERSION");

	if(ac > 1){
		if(ac > 2){ /* Handle script's arguments */
			luaL_checkstack(L, ac-1, "too many arguments to script");	/* Place for args (ac-2) + the table itself */
			lua_createtable(L, ac-2, 0);
			for(int i=2; i<ac; i++){
				lua_pushstring(L, av[i]);
				lua_rawseti(L, -2, i-1);
			}
			lua_setglobal(L, "arg");
		}

		char *t = strdup( av[1] );	/* Launching script */
		assert(t);
		lua_pushstring(L, dirname(t) );
		lua_setglobal(L, "SELENE_SCRIPT_DIR");
		strcpy(t, av[1]);
		lua_pushstring(L, basename(t) );
		lua_setglobal(L, "SELENE_SCRIPT_NAME");

		int err = luaL_loadfile(L, av[1]) || lua_pcall(L, 0, 0, 0);
		if(err){
			fprintf(stderr, "%s", lua_tostring(L, -1));
			lua_pop(L, 1);  /* pop error message from the stack */
			exit(EXIT_FAILURE);
		}
	} else while(fgets(l, sizeof(l), stdin) != NULL){
		int err = luaL_loadbuffer(L, l, strlen(l), "line") || lua_pcall(L, 0, 0, 0);
		if(err){
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);  /* pop error message from the stack */
		}
	}

	exit(EXIT_SUCCESS);
}

