#ifndef __HIRLITE_EXT_H
#define __HIRLITE_EXT_H

/* Defined for Rubinius. This indicates a char* obtained
 * through RSTRING_PTR is never modified in place. With this
 * define Rubinius can disable the slow copy back mechanisms
 * to make sure strings are updated at the Ruby side.
 */
#define RSTRING_NOT_MODIFIED

#include "hirlite.h"
#include "ruby.h"

/* Defined in hirlite_ext.c */
extern VALUE mod_hirlite;

/* Defined in rlite.c */
extern VALUE klass_rlite;
extern void InitRlite(VALUE module);

/* Borrowed from Nokogiri */
#ifndef RSTRING_PTR
#define RSTRING_PTR(s) (RSTRING(s)->ptr)
#endif

#ifndef RSTRING_LEN
#define RSTRING_LEN(s) (RSTRING(s)->len)
#endif

#ifndef RARRAY_PTR
#define RARRAY_PTR(a) RARRAY(a)->ptr
#endif

#ifndef RARRAY_LEN
#define RARRAY_LEN(a) RARRAY(a)->len
#endif

#endif
