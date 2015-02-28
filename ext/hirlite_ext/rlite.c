/**
 *
 * Copyright (c) 2010-2012, Pieter Noordhuis
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of Redis nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is based on these files:
 * https://github.com/redis/hiredis-rb/blob/master/ext/hiredis_ext/connection.c
 * https://github.com/redis/hiredis-rb/blob/master/ext/hiredis_ext/reader.c
 *
 */
#include <errno.h>
#include "hirlite_ext.h"

/* Force encoding on new strings? */
static VALUE enc_klass;
static ID enc_default_external = 0;
static ID str_force_encoding = 0;

typedef struct rliteParentContext {
    rliteContext *context;
} rliteParentContext;

static void parent_context_try_free_context(rliteParentContext *pc) {
    if (pc->context) {
        rliteFree(pc->context);
        pc->context = NULL;
    }
}

static void parent_context_try_free(rliteParentContext *pc) {
    parent_context_try_free_context(pc);
}

static void parent_context_mark(rliteParentContext *pc) {
}

static void parent_context_free(rliteParentContext *pc) {
    parent_context_try_free(pc);
    free(pc);
}

static void parent_context_raise(rliteParentContext *pc) {
    int err;
    char errstr[1024];

    /* Copy error and free context */
    err = pc->context->err;
    snprintf(errstr,sizeof(errstr),"%s",pc->context->errstr);
    parent_context_try_free(pc);

    switch(err) {
    case RLITE_ERR_IO:
        /* Raise native Ruby I/O error */
        rb_sys_fail(0);
        break;
    case RLITE_ERR_EOF:
        /* Raise native Ruby EOFError */
        rb_raise(rb_eEOFError,"%s",errstr);
        break;
    default:
        /* Raise something else */
        rb_raise(rb_eRuntimeError,"%s",errstr);
    }
}

static VALUE rlite_parent_context_alloc(VALUE klass) {
    rliteParentContext *pc = malloc(sizeof(*pc));
    pc->context = NULL;
    return Data_Wrap_Struct(klass, parent_context_mark, parent_context_free, pc);
}

static VALUE rlite_generic_connect(VALUE self, rliteContext *c, VALUE arg_timeout) {
    rliteParentContext *pc;

    Data_Get_Struct(self,rliteParentContext,pc);

    if (c->err) {
        char buf[1024];
        int err;

        /* Copy error and free context */
        err = c->err;
        snprintf(buf,sizeof(buf),"%s",c->errstr);
        rliteFree(c);

        if (err == RLITE_ERR_IO) {
            /* Raise native Ruby I/O error */
            rb_sys_fail(0);
        } else {
            /* Raise something else */
            rb_raise(rb_eRuntimeError,"%s",buf);
        }
    }

    parent_context_try_free_context(pc);
    pc->context = c;
    return Qnil;
}

static VALUE rlite_connect(int argc, VALUE *argv, VALUE self) {
    rliteContext *c;
    VALUE arg_host = Qnil;
    VALUE arg_port = Qnil;
    VALUE arg_timeout = Qnil;

    if (argc == 2 || argc == 3) {
        arg_host = argv[0];
        arg_port = argv[1];

        if (argc == 3) {
            arg_timeout = argv[2];

            /* Sanity check */
            if (NUM2INT(arg_timeout) <= 0) {
                rb_raise(rb_eArgError, "timeout should be positive");
            }
        }
    } else {
        rb_raise(rb_eArgError, "invalid number of arguments");
    }

    c = rliteConnectNonBlock(StringValuePtr(arg_host), NUM2INT(arg_port));
    return rlite_generic_connect(self,c,arg_timeout);
}

static VALUE rlite_connect_unix(int argc, VALUE *argv, VALUE self) {
    rliteContext *c;
    VALUE arg_path = Qnil;
    VALUE arg_timeout = Qnil;

    if (argc == 1 || argc == 2) {
        arg_path = argv[0];

        if (argc == 2) {
            arg_timeout = argv[1];

            /* Sanity check */
            if (NUM2INT(arg_timeout) <= 0) {
                rb_raise(rb_eArgError, "timeout should be positive");
            }
        }
    } else {
        rb_raise(rb_eArgError, "invalid number of arguments");
    }

    c = rliteConnectUnixNonBlock(StringValuePtr(arg_path));
    return rlite_generic_connect(self,c,arg_timeout);
}

static VALUE rlite_is_connected(VALUE self) {
    rliteParentContext *pc;
    Data_Get_Struct(self,rliteParentContext,pc);
    if (pc->context && !pc->context->err)
        return Qtrue;
    else
        return Qfalse;
}

static VALUE rlite_disconnect(VALUE self) {
    rliteParentContext *pc;
    Data_Get_Struct(self,rliteParentContext,pc);
    if (!pc->context)
        rb_raise(rb_eRuntimeError,"%s","not connected");
    parent_context_try_free(pc);
    return Qnil;
}

static VALUE rlite_write(VALUE self, VALUE command) {
    rliteParentContext *pc;
    int argc;
    char **argv = NULL;
    size_t *alen = NULL;
    int i;

    /* Commands should be an array of commands, where each command
     * is an array of string arguments. */
    if (TYPE(command) != T_ARRAY)
        rb_raise(rb_eArgError,"%s","not an array");

    Data_Get_Struct(self,rliteParentContext,pc);
    if (!pc->context)
        rb_raise(rb_eRuntimeError,"%s","not connected");

    argc = (int)RARRAY_LEN(command);
    argv = malloc(argc*sizeof(char*));
    alen = malloc(argc*sizeof(size_t));
    for (i = 0; i < argc; i++) {
        /* Replace arguments in the arguments array to prevent their string
         * equivalents to be garbage collected before this loop is done. */
        VALUE entry = rb_obj_as_string(rb_ary_entry(command, i));
        rb_ary_store(command, i, entry);
        argv[i] = RSTRING_PTR(entry);
        alen[i] = RSTRING_LEN(entry);
    }
    rliteAppendCommandArgv(pc->context,argc,argv,alen);
    free(argv);
    free(alen);
    return Qnil;
}

static VALUE rlite_flush(VALUE self) {
    return Qnil;
}

static VALUE createDecodedString(char *str, size_t len) {
    VALUE v, enc;
    v = rb_str_new(str,len);

    /* Force default external encoding if possible. */
    if (enc_default_external) {
        enc = rb_funcall(enc_klass,enc_default_external,0);
        v = rb_funcall(v,str_force_encoding,1,enc);
    }

    return v;
}

static VALUE reply_to_value(rliteReply *reply) {
    if (reply->type == RLITE_REPLY_STATUS || reply->type == RLITE_REPLY_STRING) {
        if (reply->type == RLITE_REPLY_STATUS && reply->len == 2 && memcmp(reply->str, "OK", 2) == 0) {
            return Qtrue;
        }
        return createDecodedString(reply->str, reply->len);
    }
    if (reply->type == RLITE_REPLY_NIL) {
        return Qnil;
    }
    if (reply->type == RLITE_REPLY_INTEGER) {
        return LL2NUM(reply->integer);
    }
    if (reply->type == RLITE_REPLY_ERROR) {
        VALUE obj = createDecodedString(reply->str, reply->len);
        return rb_funcall(rb_eRuntimeError,rb_intern("new"),1,obj);
    }
    if (reply->type == RLITE_REPLY_ARRAY) {
        VALUE v, element;
        size_t i;
        v =  rb_ary_new2(reply->elements);
        for (i = 0; i < reply->elements; i++) {
            element = reply_to_value(reply->element[i]);
            rb_ary_store(v, i, element);
        }
        return v;
    }
    return Qnil;
}

static int __get_reply(rliteParentContext *pc, VALUE *reply) {
    rliteContext *c = pc->context;
    void *aux = NULL;

    /* Try to read pending replies */
    if (rliteGetReply(c,&aux) == RLITE_ERR) {
        /* Protocol error */
        return -1;
    }

    /* Set reply object */
    if (reply && aux) {
        *reply = reply_to_value(aux);
    }

    return 0;
}

static VALUE rlite_read(VALUE self) {
    rliteParentContext *pc;
    VALUE reply = Qnil;

    Data_Get_Struct(self,rliteParentContext,pc);
    if (!pc->context)
        rb_raise(rb_eRuntimeError, "not connected");

    if (__get_reply(pc,&reply) == -1)
        parent_context_raise(pc);

    return reply;
}

static VALUE rlite_set_timeout(VALUE self, VALUE usecs) {
    if (NUM2INT(usecs) < 0) {
        rb_raise(rb_eArgError, "timeout cannot be negative");
    }

    return Qnil;
}

static VALUE rlite_fileno(VALUE self) {
    rliteParentContext *pc;

    Data_Get_Struct(self,rliteParentContext,pc);

    if (!pc->context)
        rb_raise(rb_eRuntimeError, "not connected");

    return INT2NUM(0);
}

VALUE klass_rlite;
void InitRlite(VALUE mod) {
    klass_rlite = rb_define_class_under(mod, "Rlite", rb_cObject);
    rb_global_variable(&klass_rlite);
    rb_define_alloc_func(klass_rlite, rlite_parent_context_alloc);
    rb_define_method(klass_rlite, "connect", rlite_connect, -1);
    rb_define_method(klass_rlite, "connect_unix", rlite_connect_unix, -1);
    rb_define_method(klass_rlite, "connected?", rlite_is_connected, 0);
    rb_define_method(klass_rlite, "disconnect", rlite_disconnect, 0);
    rb_define_method(klass_rlite, "timeout=", rlite_set_timeout, 1);
    rb_define_method(klass_rlite, "fileno", rlite_fileno, 0);
    rb_define_method(klass_rlite, "write", rlite_write, 1);
    rb_define_method(klass_rlite, "flush", rlite_flush, 0);
    rb_define_method(klass_rlite, "read", rlite_read, 0);

    /* If the Encoding class is present, #default_external should be used to
     * determine the encoding for new strings. The "enc_default_external"
     * ID is non-zero when encoding should be set on new strings. */
    if (rb_const_defined(rb_cObject, rb_intern("Encoding"))) {
        enc_klass = rb_const_get(rb_cObject, rb_intern("Encoding"));
        enc_default_external = rb_intern("default_external");
        str_force_encoding = rb_intern("force_encoding");
        rb_global_variable(&enc_klass);
    } else {
        enc_default_external = 0;
    }
}
