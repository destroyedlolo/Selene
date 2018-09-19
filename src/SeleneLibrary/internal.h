/* internal.h
 *
 * All internal shared stuffs that are not part of another include
 */

#ifndef INTERNAL_H
#define INTERNAL_H

extern void initG_Selene();
extern void initG_SelLog();
extern void initG_SeleMQTT();

	/* SelMQTT's shared with SeleMQTT */
extern int smq_QoSConst( lua_State * );
extern int smq_ErrCodeConst( lua_State * );
extern int smq_StrError( lua_State * );

#endif
