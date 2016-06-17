#include "lazy_proxy.h"

static void wrapped_object_mark(void *p) {
  struct wrapped_object *ptr = p;
  rb_gc_mark(ptr->obj);
}

static size_t wrapped_object_memsize(const void *p) {
  return sizeof(struct wrapped_object);
}

static VALUE lp_alloc(VALUE klass) {
  struct wrapped_object *ptr;

  VALUE proxy_obj = TypedData_Make_Struct(klass, struct wrapped_object, &wrapped_object_data_type, ptr);
  ptr->obj = Qundef;
  return proxy_obj;
}

static struct wrapped_object * lp_ptr(VALUE obj) {
  struct wrapped_object *ptr;

  TypedData_Get_Struct(obj, struct wrapped_object, &wrapped_object_data_type, ptr);
  return ptr;
}

static void lp_cpy(struct wrapped_object * dst, struct wrapped_object * src) {
  memcpy(dst, src, sizeof(struct wrapped_object));
}

static void lp_set(VALUE self, VALUE obj, unsigned char isblk) {
  struct wrapped_object data = {obj, isblk};
  struct wrapped_object * ptr = lp_ptr(self);

  lp_cpy(ptr, &data);
}

static void lp_resolve(struct wrapped_object * ptr) {
  VALUE resolved = rb_funcall(ptr->obj, rb_intern("call"), 0);
  struct wrapped_object data = {resolved, 0};

  lp_cpy(ptr, &data);
}

static VALUE lp_get_resolv(VALUE self) {
  struct wrapped_object * ptr = lp_ptr(self);

  if(ptr->isblk)
    lp_resolve(ptr);

  return ptr->obj;
}

static VALUE lp_initialize(int argc, VALUE* argv, VALUE self) {
  VALUE arg, blk;
  rb_scan_args(argc, argv, "01&", &arg, &blk);

  if(!NIL_P(arg))
    lp_set(self, arg, 0);
  else if(!NIL_P(blk))
    lp_set(self, blk, 1);
  else
    lp_set(self, Qnil, 0);

  return self;
}

static VALUE lp_inspect(VALUE self) {
  VALUE str, cname = rb_obj_class(self);
  struct wrapped_object * ptr = lp_ptr(self);

  str = rb_sprintf("#<%"PRIsVALUE": %+"PRIsVALUE"", rb_class_path(cname), ptr->obj);

  if(ptr->isblk)
    rb_str_buf_cat2(str, " (unresolved)");

  rb_str_buf_cat2(str, ">");

  return str;
}

static VALUE lp_method_missing(int argc, VALUE* argv, VALUE self) {
  VALUE method_name = *argv, obj = lp_get_resolv(self);

  return rb_funcall2(obj, SYM2ID(method_name), --argc, ++argv);
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
  rb_cLazyProxy = rb_define_class("LazyProxy", rb_cObject);

  rb_define_alloc_func(rb_cLazyProxy, lp_alloc);

  rb_define_private_method(rb_cLazyProxy, "initialize", lp_initialize, -1);
  rb_define_private_method(rb_cLazyProxy, "initialize_copy", lp_init_copy, 1);
  rb_define_method(rb_cLazyProxy, "__setobj__", lp_initialize, -1);
  rb_define_method(rb_cLazyProxy, "__getobj__", lp_get_resolv, 0);
  rb_define_method(rb_cLazyProxy, "inspect", lp_inspect, 0);
  rb_define_method(rb_cLazyProxy, "method_missing", lp_method_missing, -1);

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
