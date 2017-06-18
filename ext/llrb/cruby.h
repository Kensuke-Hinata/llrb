#ifndef LLRB_CRUBY_H
#define LLRB_CRUBY_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "cruby/internal.h"
#include "cruby/vm_core.h"
#pragma GCC diagnostic pop
#include "cruby/method.h"

/* start vm_insnhelper.h (which can't be compiled without calling static function) */

/* optimize insn */
#define FIXNUM_2_P(a, b) ((a) & (b) & 1)
#if USE_FLONUM
#define FLONUM_2_P(a, b) (((((a)^2) | ((b)^2)) & 3) == 0) /* (FLONUM_P(a) && FLONUM_P(b)) */
#else
#define FLONUM_2_P(a, b) 0
#endif

/* end vm_insnhelper.h */

#endif // LLRB_CRUBY_H
