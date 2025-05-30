/*
  +----------------------------------------------------------------------+
  | Swoole                                                               |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.0 of the Apache license,    |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.apache.org/licenses/LICENSE-2.0.html                      |
  | If you did not receive a copy of the Apache2.0 license and are unable|
  | to obtain it through the world-wide-web, please send a note to       |
  | license@swoole.com so we can mail you a copy immediately.            |
  +----------------------------------------------------------------------+
  | Author: Tianfeng Han  <rango@swoole.com>                             |
  |         Twosee  <twose@qq.com>                                       |
  +----------------------------------------------------------------------+
*/

#pragma once

typedef volatile int32_t sw_atomic_int32_t;
typedef volatile uint32_t sw_atomic_uint32_t;
typedef volatile int64_t sw_atomic_int64_t;
typedef volatile uint64_t sw_atomic_uint64_t;

typedef sw_atomic_int64_t sw_atomic_long_t;
typedef sw_atomic_uint64_t sw_atomic_ulong_t;
typedef sw_atomic_uint32_t sw_atomic_t;

#define sw_atomic_cmp_set(lock, old, set) __sync_bool_compare_and_swap(lock, old, set)
#define sw_atomic_value_cmp_set(value, expected, set) __sync_val_compare_and_swap(value, expected, set)
#define sw_atomic_fetch_add(value, add) __sync_fetch_and_add(value, add)
#define sw_atomic_fetch_sub(value, sub) __sync_fetch_and_sub(value, sub)
#define sw_atomic_memory_barrier() __sync_synchronize()
#define sw_atomic_add_fetch(value, add) __sync_add_and_fetch(value, add)
#define sw_atomic_sub_fetch(value, sub) __sync_sub_and_fetch(value, sub)

#if defined(__x86_64__)
#define sw_atomic_cpu_pause() __asm__ __volatile__("pause")
#elif defined(__aarch64__)
#define sw_atomic_cpu_pause() __asm__ __volatile__("yield")
#else
#define sw_atomic_cpu_pause()
#endif

void sw_spinlock(sw_atomic_t *lock);
#define sw_spinlock_release(lock) __sync_lock_release(lock)
int sw_atomic_futex_wait(sw_atomic_t *atomic, double timeout);
int sw_atomic_futex_wakeup(sw_atomic_t *atomic, int n);
