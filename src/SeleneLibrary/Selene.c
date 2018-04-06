/* Selene object */

#include <unistd.h>	     /* gethostname(), ... */
#include <time.h>
#include <errno.h>
#include <sys/poll.h>
#include <stdlib.h>		/* exit(), ... */
#include <stdbool.h>
#include <string.h>

#include "libSelene.h"
#include "configuration.h"
#include "SelTimer.h"
#include "SelShared.h"

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
#ifdef NOT_YET
		} else if(( r = checkUData(L, j, "SelEvent"))){
			ufds[nsup].fd = ((struct SelEvent *)r)->fd;
			ufds[nsup++].events = POLLIN;
#endif
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
			} else 
			for(int j=1; j <= maxarg; j++){
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
#if NOT_YET
					if(ufds[i].fd == ((struct SelEvent *)r)->fd){
						if( pushtask( ((struct SelEvent *)r)->func, false) ){
							lua_pushstring(L, "Waiting task list exhausted : enlarge SO_TASKSSTACK_LEN");
							lua_error(L);
							exit(EXIT_FAILURE);	/* Code never reached */
						}
					}
#endif
				} else if(( r = checkUData(L, j, LUA_FILEHANDLE))){
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

/* Selene own functions */
static const struct luaL_Reg seleneLib[] = {
	{"Sleep", SelSleep},
	{"WaitFor", SelWaitFor},
	{"Hostname", SelHostname},
	{NULL, NULL} /* End of definition */
};


int initSelene( lua_State *L ){
	libSel_libFuncs( L, "Selene", seleneLib );

	return 1;
}

