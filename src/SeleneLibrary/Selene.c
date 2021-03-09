/* Selene object */

#include <unistd.h>	     /* gethostname(), ... */
#include <time.h>
#include <errno.h>
#include <sys/poll.h>
#include <stdlib.h>		/* exit(), ... */
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <inttypes.h>

#include "libSelene.h"
#include "configuration.h"
#include "SelTimer.h"
#include "SelShared.h"
#include "SelEvent.h"

#if LUA_VERSION_NUM <= 501
void *luaL_testudata(lua_State *L, int ud, const char *tname){
/* Like luaL_checkudata() but w/o crashing if doesn't march
 * From luaL_checkudata() source code
 * This function appeared with 5.2 so it's a workaround for 5.1
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
#endif

#ifdef DEBUG
static int _handleToDoList
#else
static int handleToDoList
#endif
( lua_State *L ){ /* Execute functions in the ToDo list */
	for(;;){
		int taskid;
		sel_shareable_lock( &SharedStuffs.mutex_tl );
		if(SharedStuffs.ctask == SharedStuffs.maxtask){	/* No remaining waiting task */
			sel_shareable_unlock( &SharedStuffs.mutex_tl );
			break;
		}
		taskid = SharedStuffs.todo[SharedStuffs.ctask++ % SO_TASKSSTACK_LEN];
		sel_shareable_unlock( &SharedStuffs.mutex_tl );

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

static int SelWaitFor( lua_State *L ){
	unsigned int nsup=0;	/* Number of supervised object (used as index in the table) */
	int nre;				/* Number of received event */
	struct pollfd ufds[WAITMAXFD];
	int maxarg = lua_gettop(L);
	int i,j;

	for(j=1; j <= lua_gettop(L); j++){	/* Stacks SelTimer arguments */
		if(nsup == WAITMAXFD){
			lua_pushnil(L);
			lua_pushstring(L, "Exhausting number of waiting FD, please increase WAITMAXFD");
			return 2;
		}

		void *r;
		if((r = luaL_testudata(L, j, "SelTimer"))){	/* We got a SelTimer */
			ufds[nsup].fd = ((struct SelTimer *)r)->fd;
			ufds[nsup++].events = POLLIN;
		} else if(( r = luaL_testudata(L, j, "SelEvent"))){
			ufds[nsup].fd = ((struct SelEvent *)r)->fd;
			ufds[nsup++].events = POLLIN;
		} else if(( r = luaL_testudata(L, j, LUA_FILEHANDLE))){	/* We got a file */
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

	for(i=0; i<nsup; i++){
		if( ufds[i].revents ){	/* This one has data */
			if( ufds[i].fd == SharedStuffs.tlfd ){ /* Todo list's evenfd */
				uint64_t v;
				if(read( ufds[i].fd, &v, sizeof( uint64_t )) != sizeof( uint64_t ))
					perror("read(eventfd)");
				lua_pushcfunction(L, &handleToDoList);	/*  Push the function to handle the todo list */
			} else for(j=1; j <= maxarg; j++){
				void *r;
				if((r=luaL_testudata(L, j, "SelTimer"))){
					if(ufds[i].fd == ((struct SelTimer *)r)->fd && !((struct SelTimer *)r)->disable){
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
				} else if((r=luaL_testudata(L, j, "SelEvent"))){
					if(ufds[i].fd == ((struct SelEvent *)r)->fd){
						if( pushtask( ((struct SelEvent *)r)->func, false) ){
							lua_pushstring(L, "Waiting task list exhausted : enlarge SO_TASKSSTACK_LEN");
							lua_error(L);
							exit(EXIT_FAILURE);	/* Code never reached */
						}
					}
				} else if(( r = luaL_testudata(L, j, LUA_FILEHANDLE))){
					if(ufds[i].fd == fileno(*((FILE **)r)))
						lua_pushvalue(L, j);
				}
			}
		}
	}

	return lua_gettop(L)-maxarg;	/* Number of stuffs to proceed */
}

static int SelSleep( lua_State *L ){
	struct timespec ts;
	lua_Number lenght = luaL_checknumber(L, 1);
	ts.tv_sec = (time_t)lenght;
	ts.tv_nsec = (unsigned long int)((lenght - (time_t)lenght) * 1e9);

	nanosleep( &ts, NULL );
	return 0;
}

static int SelHostname( lua_State *L ){
	char n[HOST_NAME_MAX];
	gethostname(n, HOST_NAME_MAX);

	lua_pushstring(L, n);
	return 1;
}

static int SelgetPID( lua_State *L ){
	lua_pushinteger(L, getpid());
	return 1;
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
	 * Multithreading
	 */
pthread_attr_t thread_attr;
void *SalveInitFunctionsList;

struct launchargs {
	lua_State *L;	/* New thread Lua state */
	int nargs;		/* Number of arguments for the function */
	int nresults;	/* Number of results */
	int triggerid;	/* Trigger to add in todo list if return true */
	enum TaskOnce trigger_once;
};

static void *launchfunc(void *a){
	struct launchargs *arg = a;	/* To avoid further casting */

	if(lua_pcall( arg->L, arg->nargs, arg->nresults, 0))
		fprintf(stderr, "*E* (launch) %s\n", lua_tostring(arg->L, -1));
	else {
		if( arg->triggerid != LUA_REFNIL){
			if(lua_toboolean(arg->L, -1))
				pushtask( arg->triggerid, arg->trigger_once );
		}
	}

	lua_close(arg->L);
	free(arg);
	return NULL;
}

lua_State *createslavethread( void ){
/* Create and initialize (for our objects) a new state
 * for a slave thread.
 */
	lua_State *tstate = luaL_newstate();
	assert(tstate);

	luaL_openlibs( tstate );
	libSel_ApplyStartupFunc( SalveInitFunctionsList, tstate );
	initSelTimedCollection( tstate );

	return tstate;
}

bool loadandlaunch( lua_State *L, lua_State *newL, struct elastic_storage *storage, int nargs, int nresults, int trigger, enum TaskOnce trigger_once){
/* load and then launch a stored function in a slave thread
 * -> L : master thread (for error reporting, may be NULL)
 *    newL : slave thread
 *    storage : storage of the function
 *    nargs : number of arguments to the functions
 *    trigger : if not LUA_REFNIL, add this trigger_id in the task's list
 * <- success or not
 */

		/* It's needed because this structure has to survive until
		 * slave function is over.
		 * It will be cleared in launchfunc()
		 */
	struct launchargs *arg = malloc( sizeof(struct launchargs) );
	assert(arg);
	arg->L = newL;
	arg->nargs = nargs;
	arg->nresults = nresults;
	arg->triggerid = trigger;

	int err;
	if( (err = loadsharedfunction(newL, storage)) ){
		if(L){
			lua_pushnil(L);
			lua_pushstring(L, (err == LUA_ERRSYNTAX) ? "Syntax error" : "Memory error");
		} else 
			fprintf(stderr, "*E* Can't create a new thread : %s\n", (err == LUA_ERRSYNTAX) ? "Syntax error" : "Memory error" );
		return false;
	}

	if(nargs)	/* Move the function before its arguments */
		lua_insert(newL, -1 - nargs);

	pthread_t tid;	/* No need to be kept */
	if(pthread_create( &tid, &thread_attr, launchfunc,  arg) < 0){
		fprintf(stderr, "*E* Can't create a new thread : %s\n", strerror(errno));
		if(L){
			lua_pushnil(L);
			lua_pushstring(L, strerror(errno));
		}
		return false;
	}
	return true;
}

static bool newthreadfunc( lua_State *L, struct elastic_storage *storage ){
/* Launch a function in a new thread
 * -> 	L : master thread (optional)
 * 		storage : storage of the function
 * <- is the function successful ?
 */
	lua_State *tstate = createslavethread();
	return( loadandlaunch( L, tstate, storage, 0, 0, LUA_REFNIL, TO_MULTIPLE ) );
}

static int SelDetach( lua_State *L ){
	struct elastic_storage **r;

	if(lua_type(L, 1) == LUA_TFUNCTION ){
		struct elastic_storage storage;
		assert( EStorage_init( &storage ) );

		if(lua_dump(L, ssfc_dumpwriter, &storage
#if LUA_VERSION_NUM > 501
			,1
#endif
		) != 0){
			EStorage_free( &storage );
			return luaL_error(L, "unable to dump given function");
		}
		lua_pop(L,1);	/* remove the function from the stack */

		bool ret = newthreadfunc(L, &storage);
		EStorage_free( &storage );

		return( ret ? 0 : 2 );
	} else if( (r = luaL_testudata(L, 1, "SelSharedFunc")) ){
		return( newthreadfunc(L, *r) ? 0 : 2 );
	} else {
		lua_pushnil(L);
		lua_pushstring(L, "Function or shared function needed as 1st argument of Selene.Detach()");
		return 2;
	}

	return 0;
}


/* Selene own functions */
static const struct luaL_Reg seleneExtLib[] = {	/* Extended ones */
	{"WaitFor", SelWaitFor},
	{"SigIntTask", SelSigIntTask},
	{"Detach", SelDetach},
	{NULL, NULL} /* End of definition */
};

static const struct luaL_Reg seleneLib[] = {	/* Ones for every applications */
	{"Sleep", SelSleep},
	{"Hostname", SelHostname},
	{"getHostname", SelHostname},
	{"getPid", SelgetPID},
	{NULL, NULL} /* End of definition */
};


void initG_Selene(){
		/* Creates threads as detached in order to save
		 * some resources when quiting
		 */
	assert(!pthread_attr_init (&thread_attr));
	assert(!pthread_attr_setdetachstate (&thread_attr, PTHREAD_CREATE_DETACHED));

		/* Define shared objects that must be known by all slaves */
	SalveInitFunctionsList = libSel_AddStartupFunc(NULL, initSelShared);
	SalveInitFunctionsList = libSel_AddStartupFunc(SalveInitFunctionsList, initSelFIFO);
}

int initReducedSelene( lua_State *L ){
	libSel_libFuncs( L, "Selene", seleneLib );
	return 1;
}

int initSelene( lua_State *L ){
	libSel_libFuncs( L, "Selene", seleneLib );
	libSel_libFuncs( L, "Selene", seleneExtLib );

	return 1;
}

