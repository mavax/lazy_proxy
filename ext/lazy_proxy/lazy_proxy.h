#include <ruby.h>

#define wrapped_object_free RUBY_TYPED_DEFAULT_FREE

#define DECLARE_FORW_FUNC(method_name) \
static VALUE lp_##method_name(int argc, VALUE* argv, VALUE self);
#define DEFINE_FORW_FUNC(method_name, sanitized_name) \
static VALUE lp_##sanitized_name(int argc, VALUE* argv, VALUE self) { \
  return rb_funcall2(lp_get_resolv(self), rb_intern(method_name), argc, argv); \
}
#define ATTACH_FORW_FUNC(method_name, sanitized_name) \
rb_define_method(rb_cLazyProxy, method_name, lp_##sanitized_name, -1);

static ID id_call;
static ID id_respond_to;

static VALUE rb_cLazyProxy;

struct wrapped_object {
  VALUE obj;
  VALUE blk;
  unsigned char isblk;
  unsigned char resolved;
};

static void wrapped_object_mark(void *p);
static size_t wrapped_object_memsize(const void *p);

static const rb_data_type_t wrapped_object_data_type = {
  "wrapped_object",
  {
    wrapped_object_mark,
    wrapped_object_free,
    wrapped_object_memsize
  },
  0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static struct wrapped_object* lp_ptr(VALUE obj);
static void lp_cpy(struct wrapped_object * dst, struct wrapped_object * src);
static void lp_set(VALUE self, VALUE obj, VALUE blk, unsigned char isblk);
static void lp_resolve(struct wrapped_object * ptr);
static unsigned char lp_is_unresolved_blk(struct wrapped_object *ptr);

static VALUE lp_alloc(VALUE klass);
static VALUE lp_get_resolv(VALUE self);
static VALUE lp_initialize(int argc, VALUE* argv, VALUE self);
static VALUE lp_inspect(VALUE self);
static VALUE lp_method_missing(int argc, VALUE* argv, VALUE self);
static VALUE lp_respond_to_missing(int argc, VALUE* argv, VALUE self);
static VALUE lp_init_copy(VALUE dst, VALUE src);

DECLARE_FORW_FUNC(enum_for);
DECLARE_FORW_FUNC(to_enum);
DECLARE_FORW_FUNC(to_s);
DECLARE_FORW_FUNC(fnot);
DECLARE_FORW_FUNC(noteql);
DECLARE_FORW_FUNC(eqleql);
DECLARE_FORW_FUNC(eqleqleql);
DECLARE_FORW_FUNC(eqltld);
DECLARE_FORW_FUNC(isnil);

void Init_lazy_proxy();



