#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "hirlite_ext.h"

VALUE mod_hirlite;
VALUE mod_ext;
void Init_hirlite_ext() {
    mod_hirlite = rb_define_module("Hirlite");
    mod_ext = rb_define_module_under(mod_hirlite,"Ext");
    rb_global_variable(&mod_hirlite);
    rb_global_variable(&mod_ext);
    InitRlite(mod_ext);
}
