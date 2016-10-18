#include "lazy_proxy.h"

static void wrapped_object_mark(void *p) {
  struct wrapped_object *ptr = p;
  rb_gc_mark(ptr->obj);
  rb_gc_mark(ptr->blk);
}

static size_t wrapped_object_memsize(const void *p) {
  return sizeof(struct wrapped_object);
}

static VALUE lp_alloc(VALUE klass) {
  struct wrapped_object *ptr;

  return TypedData_Make_Struct(klass, struct wrapped_object, &wrapped_object_data_type, ptr);
}

static struct wrapped_object * lp_ptr(VALUE obj) {
  struct wrapped_object *ptr;

  TypedData_Get_Struct(obj, struct wrapped_object, &wrapped_object_data_type, ptr);
  return ptr;
}

static void lp_cpy(struct wrapped_object * dst, struct wrapped_object * src) {
  memcpy(dst, src, sizeof(struct wrapped_object));
}

static void lp_set(VALUE self, VALUE obj, VALUE blk, unsigned char isblk) {
  struct wrapped_object data = {obj, blk, isblk, !isblk};
  struct wrapped_object * ptr = lp_ptr(self);

  lp_cpy(ptr, &data);
}

static void lp_resolve(struct wrapped_object * ptr) {
  VALUE blk = ptr->blk, resolved = rb_funcall(blk, id_call, 0);
  struct wrapped_object data = {resolved, blk, 1, 1};

  lp_cpy(ptr, &data);
}

static VALUE lp_get_resolv(VALUE self) {
  struct wrapped_object * ptr = lp_ptr(self);

  if (lp_is_unresolved_blk(ptr))
    lp_resolve(ptr);

  return ptr->obj;
}

static VALUE lp_initialize(int argc, VALUE* argv, VALUE self) {
  VALUE arg, blk;

  rb_check_arity(argc, 0, 1);

  arg = (argc == 1) ? *argv : Qnil;
  blk = rb_block_given_p() ? rb_block_proc() : Qnil;

  lp_set(self, arg, blk, NIL_P(arg) && !NIL_P(blk));

  return self;
}

static VALUE lp_reset(VALUE self) {
  struct wrapped_object * ptr = lp_ptr(self);

  if(!ptr->isblk)
    rb_raise(rb_eArgError, "proxy was not provided with a block");

  ptr->resolved = 0;

  return Qtrue;
}

static unsigned char lp_is_unresolved_blk(struct wrapped_object *ptr) {
  return ptr && !ptr->resolved;
};

static VALUE lp_inspect(VALUE self) {
  VALUE str, cname = rb_obj_class(self);
  struct wrapped_object * ptr = lp_ptr(self);

  if (lp_is_unresolved_blk(ptr))
    str = rb_sprintf("#<%"PRIsVALUE": %+"PRIsVALUE" (unresolved)>", rb_class_path(cname), ptr->blk);
  else
    str = rb_sprintf("#<%"PRIsVALUE": %+"PRIsVALUE">", rb_class_path(cname), ptr->obj);

  return str;
}

static VALUE lp_send(int argc, VALUE* argv, VALUE self) {
  VALUE obj;
  ID method_id;
  rb_check_arity(argc, 1, UNLIMITED_ARGUMENTS);

  obj = lp_get_resolv(self);
  method_id = rb_to_id(*argv);

  return rb_funcall2(obj, method_id, --argc, ++argv);
}

static VALUE lp_method_missing(int argc, VALUE* argv, VALUE self) {
  VALUE method_name = *argv, obj = lp_get_resolv(self);

  return rb_funcallv_public(obj, SYM2ID(method_name), --argc, ++argv);
}

static VALUE lp_respond_to_missing(int argc, VALUE* argv, VALUE self) {
  VALUE obj = lp_get_resolv(self);

  return rb_funcall2(obj, id_respond_to, argc, argv);
}

static VALUE lp_init_copy(VALUE dst, VALUE src) {
  struct wrapped_object *psrc = lp_ptr(src), *pdst;

  if (!OBJ_INIT_COPY(dst, src)) return dst;

  TypedData_Get_Struct(dst, struct wrapped_object, &wrapped_object_data_type, pdst);
  if (!pdst)
    rb_raise(rb_eArgError, "unallocated proxy");

  lp_cpy(pdst, psrc);
  return dst;
}

void Init_lazy_proxy() {
  id_call = rb_intern("call");
  id_respond_to = rb_intern("respond_to?");

  rb_cLazyProxy = rb_define_class("LazyProxy", rb_cObject);

  rb_define_alloc_func(rb_cLazyProxy, lp_alloc);

  rb_define_method(rb_cLazyProxy, "__setobj__", lp_initialize, -1);
  rb_define_method(rb_cLazyProxy, "__getobj__", lp_get_resolv, 0);
  rb_define_method(rb_cLazyProxy, "__reset__", lp_reset, 0);
  rb_define_method(rb_cLazyProxy, "send", lp_send, -1);
  rb_define_method(rb_cLazyProxy, "inspect", lp_inspect, 0);

  rb_define_private_method(rb_cLazyProxy, "initialize", lp_initialize, -1);
  rb_define_private_method(rb_cLazyProxy, "initialize_copy", lp_init_copy, 1);
  rb_define_private_method(rb_cLazyProxy, "method_missing", lp_method_missing, -1);
  rb_define_private_method(rb_cLazyProxy, "respond_to_missing?", lp_respond_to_missing, -1);

  ATTACH_FORW_FUNC("enum_for", enum_for);
  ATTACH_FORW_FUNC("to_enum", to_enum);
  ATTACH_FORW_FUNC("to_s", to_s);
  ATTACH_FORW_FUNC("!", fnot);
  ATTACH_FORW_FUNC("!=", noteql);
  ATTACH_FORW_FUNC("==", eqleql);
  ATTACH_FORW_FUNC("===", eqleqleql);
  ATTACH_FORW_FUNC("=~", eqltld);
  ATTACH_FORW_FUNC("nil?", isnil);
}

DEFINE_FORW_FUNC("enum_for", enum_for);
DEFINE_FORW_FUNC("to_enum", to_enum);
DEFINE_FORW_FUNC("to_s", to_s);
DEFINE_FORW_FUNC("!", fnot);
DEFINE_FORW_FUNC("!=", noteql);
DEFINE_FORW_FUNC("==", eqleql);
DEFINE_FORW_FUNC("===", eqleqleql);
DEFINE_FORW_FUNC("=~", eqltld);
DEFINE_FORW_FUNC("nil?", isnil);
