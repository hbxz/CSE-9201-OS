
#ifdef __osf__
#define HAS_NO_SIZED_TYPES
#endif

/*
 * Some systems don't have u_int32_t and u_int16_t.
 */

#ifdef HAS_NO_SIZED_TYPES

#if defined(__alpha__) || defined(__alpha)
/* Alpha processor: lp64 */
typedef unsigned int u_int32_t;
typedef unsigned short u_int16_t;
#else
#error "HAS_NO_SIZED_TYPES defined and I don't know what the sizes should be"
#endif

#endif
