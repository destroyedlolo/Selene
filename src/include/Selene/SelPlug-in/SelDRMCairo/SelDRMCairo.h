/* SelDRMCairo.h
 *
 * Framebuffer graphics using DRM/KMS and Cairo.
 * DRMCairo is a lightweight framework to create decent graphics without the
 * need to install X.
 *
 * Depends on industry standard Cairo library
 *
 * Have a look and respect Selene Licence.
 */

#ifndef SELDRMCAIRO_VERSION

#ifdef __cplusplus
extern "C"
{
#endif

#include <Selene/libSelene.h>
#include <Selene/SelLua.h>

#if __has_include(<cairo.h>)
#	include <cairo.h>
#elif __has_include(<cairo/cairo.h>)
#	include <cairo/cairo.h>
#else
#	error "Cairo not found"
#endif

/* *********** 
 * /!\ CAUTION : BUMP THIS VERSION AT EVERY CHANGE INSIDE GLUE STRUCTURE
 * ***********/
#define SELDRMCAIRO_VERSION 2

struct SelDRMCairo {
	struct SelModule module;
};

#ifdef __cplusplus
}
#endif

#endif
