#include "libSelene.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

char *SelL_strdup( const char *as ){
	char *s;
	assert(as);
	assert( (s = malloc(strlen(as)+1)) );
	strcpy(s, as);
	return s;
}

int hash( const char *s ){	/* Calculate the hash code of a string */
	int r = 0;
	for(; *s; s++)
		r += *s;
	return r;
}
