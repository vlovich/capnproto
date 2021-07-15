#pragma once
// Unfortunately the real tsan_interface.h imports system headers which screws up our brittle
// include order. Instead let's manually redeclare all the interfaces from the real tsan_interface.h

#if KJ_BUILD_WITH_TSAN
extern "C" {
// See <sanitizer/tsan_interface.h> for documentation.
// Do not use these directly from within the KJ codebase. Instead use the `kj::` equivalent wrappers

void __tsan_acquire(void*);
void __tsan_release(void*);

void __tsan_mutex_create(void *addr, unsigned flags);
void __tsan_mutex_destroy(void *addr, unsigned flags);

void __tsan_mutex_pre_lock(void *addr, unsigned flags);
void __tsan_mutex_post_lock(void *addr, unsigned flags, int recursion);

int __tsan_mutex_pre_unlock(void *addr, unsigned flags);
void __tsan_mutex_post_unlock(void *addr, unsigned flags);

void __tsan_mutex_pre_signal(void *addr, unsigned flags);
void __tsan_mutex_post_signal(void *addr, unsigned flags);

void __tsan_mutex_pre_divert(void *addr, unsigned flags);
void __tsan_mutex_post_divert(void *addr, unsigned flags);

void *__tsan_external_register_tag(const char *object_type);
void __tsan_external_register_header(void *tag, const char *header);
void __tsan_external_assign_tag(void *addr, void *tag);
void __tsan_external_read(void *addr, void *caller_pc, void *tag);
void __tsan_external_write(void *addr, void *caller_pc, void *tag);

void *__tsan_get_current_fiber(void);
void *__tsan_create_fiber(unsigned flags);
void __tsan_destroy_fiber(void *fiber);
void __tsan_switch_to_fiber(void *fiber, unsigned flags);
void __tsan_set_fiber_name(void *fiber, const char *name);

static const unsigned __tsan_switch_to_fiber_no_sync = 1 << 0;

void __tsan_on_initialize();
int __tsan_on_finalize(int failed);
}  // extern "C"
#endif

#if KJ_BUILD_WITH_TSAN
#define TSAN_WRAPPER(kjName, tsanName)                                                          \
  template <typename ...Args>                                                                   \
  [[gnu::always_inline]] inline auto kjName(Args&&... args) -> decltype(instance<tsanName>()) { \
    return tsanName(kj::fwd<Args>(args)...);                                                    \
  }
#else
#define TSAN_WRAPPER(ret, kjName, tsanName)       \
  template <typename ...Args>                     \
  KJ_ALWAYS_INLINE(void kjName)(Args&&... args) {}
#endif

namespace kj::tsan {
namespace {
TSAN_WRAPPER(acquire, __tsan_acquire)
TSAN_WRAPPER(release, __tsan_release)
}  // namespace

namespace mutex {
namespace {
// Creation flags
static KJ_CONSTEXPR(const) unsigned LINKER_INIT = 1 << 0;
static KJ_CONSTEXPR(const) unsigned WRITE_REENTRANT = 1 << 1;
static KJ_CONSTEXPR(const) unsigned READ_REENTRANT = 1 << 2;
static KJ_CONSTEXPR(const) unsigned NOT_STATIC = 1 << 8;

// Operation flags
static KJ_CONSTEXPR(const) unsigned READ_LOCK = 1 << 3;
static KJ_CONSTEXPR(const) unsigned TRY_LOCK = 1 << 4;
static KJ_CONSTEXPR(const) unsigned TRY_LOCK_FAILED = 1 << 5;
// static const unsigned RECURSIVE_LOCK = 1 << 6;
// static const unsigned RECURSIVE_UNLOCK = 1 << 7;

static KJ_CONSTEXPR(const) unsigned TRY_READ_LOCK = READ_LOCK | TRY_LOCK;
static KJ_CONSTEXPR(const) unsigned TRY_READ_LOCK_FAILED = TRY_READ_LOCK | TRY_LOCK_FAILED;

TSAN_WRAPPER(create, __tsan_mutex_create)
TSAN_WRAPPER(destroy, __tsan_mutex_destroy)
TSAN_WRAPPER(preLock, __tsan_mutex_pre_lock)
TSAN_WRAPPER(postLock, __tsan_mutex_post_lock)
TSAN_WRAPPER(preUnlock, __tsan_mutex_pre_unlock)
TSAN_WRAPPER(postUnlock, __tsan_mutex_post_unlock)
TSAN_WRAPPER(preSignal, __tsan_mutex_pre_signal)
TSAN_WRAPPER(postSignal, __tsan_mutex_post_signal)
}  // namespace
}  // namespace mutex

namespace fiber {
namespace {
TSAN_WRAPPER(getCurrent, __tsan_get_current_fiber)
TSAN_WRAPPER(create, __tsan_create_fiber)
TSAN_WRAPPER(destroy, __tsan_destroy_fiber)
TSAN_WRAPPER(switchTo, __tsan_switch_to_fiber);
TSAN_WRAPPER(setName, __tsan_set_fiber_name);;
}  // namespace
}  // namespace fiber
}  // namespace kj::tsan

#undef TSAN_WRAPPER
