nptl 是个啥可以看这个文章 https://blog.csdn.net/adcxf/article/details/3940982


typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;
双向链表

struct __pthread_mutex_s
{
  int __lock;
  unsigned int __count;
  int __owner;
  unsigned int __nusers;
  /* KIND must stay at this position in the structure to maintain
     binary compatibility with static initializers.  */
  int __kind;
  short __spins;
  short __elision;
#  define __PTHREAD_SPINS             0, 0
  __pthread_list_t __list;
# define __PTHREAD_MUTEX_HAVE_PREV      1
};

typedef union
{
  struct __pthread_mutex_s __data;
  char __size[__SIZEOF_PTHREAD_MUTEX_T];
  long int __align;
} pthread_mutex_t;



struct __pthread_cond_s
{
  __extension__ union
  {
    __extension__ unsigned long long int __wseq;
    struct
    {
      unsigned int __low;
      unsigned int __high;
    } __wseq32;
  };
  __extension__ union
  {
    __extension__ unsigned long long int __g1_start;
    struct
    {
      unsigned int __low;
      unsigned int __high;
    } __g1_start32;
  };
  unsigned int __g_refs[2] __LOCK_ALIGNMENT;
  unsigned int __g_size[2];
  unsigned int __g1_orig_size;
  unsigned int __wrefs;
  unsigned int __g_signals[2];
};


/* Mutex types.  */
enum
{
  PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_ADAPTIVE_NP
#if defined __USE_UNIX98 || defined __USE_XOPEN2K8
  ,
  PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL
#endif
#ifdef __USE_GNU
  /* For compatibility.  */
  , PTHREAD_MUTEX_FAST_NP = PTHREAD_MUTEX_TIMED_NP
#endif
};


PTHREAD_MUTEX_NORMAL
不检查死锁
This type of mutex does not detect deadlock. A thread attempting to relock this mutex without first unlocking it shall deadlock. Attempting to unlock a mutex locked by a different thread results in undefined behavior. Attempting to unlock an unlocked mutex results in undefined behavior.

PTHREAD_MUTEX_ERRORCHECK
提供错误检查
This type of mutex provides error checking. A thread attempting to relock this mutex without first unlocking it shall return with an error. A thread attempting to unlock a mutex which another thread has locked shall return with an error. A thread attempting to unlock an unlocked mutex shall return with an error.

PTHREAD_MUTEX_RECURSIVE
可重入的互斥
A thread attempting to relock this mutex without first unlocking it shall succeed in locking the mutex. The relocking deadlock which can occur with mutexes of type PTHREAD_MUTEX_NORMAL cannot occur with this type of mutex. Multiple locks of this mutex shall require the same number of unlocks to release the mutex before another thread can acquire the mutex. A thread attempting to unlock a mutex which another thread has locked shall return with an error. A thread attempting to unlock an unlocked mutex shall return with an error.

PTHREAD_MUTEX_DEFAULT
其实和PTHREAD_MUTEX_NORMAL一样
Attempting to recursively lock a mutex of this type results in undefined behavior. Attempting to unlock a mutex of this type which was not locked by the calling thread results in undefined behavior. Attempting to unlock a mutex of this type which is not locked results in undefined behavior. An implementation may map this mutex to one of the other mutex types.



/* Mutex protocols.  */
enum
{
  PTHREAD_PRIO_NONE,
  PTHREAD_PRIO_INHERIT,
  PTHREAD_PRIO_PROTECT
};

/* Flags in mutex attr.  */
#define PTHREAD_MUTEXATTR_PROTOCOL_SHIFT	28
#define PTHREAD_MUTEXATTR_PROTOCOL_MASK		0x30000000
#define PTHREAD_MUTEXATTR_PRIO_CEILING_SHIFT	12
#define PTHREAD_MUTEXATTR_PRIO_CEILING_MASK	0x00fff000
#define PTHREAD_MUTEXATTR_FLAG_ROBUST		0x40000000
#define PTHREAD_MUTEXATTR_FLAG_PSHARED		0x80000000
#define PTHREAD_MUTEXATTR_FLAG_BITS \
  (PTHREAD_MUTEXATTR_FLAG_ROBUST | PTHREAD_MUTEXATTR_FLAG_PSHARED \
   | PTHREAD_MUTEXATTR_PROTOCOL_MASK | PTHREAD_MUTEXATTR_PRIO_CEILING_MASK)

00001111000000000000111111111111  ~PTHREAD_MUTEXATTR_FLAG_BITS
11110000111111111111000000000000 PTHREAD_MUTEXATTR_FLAG_BITS

01000000000000000000000000000000 PTHREAD_MUTEXATTR_FLAG_ROBUST
10000000000000000000000000000000 PTHREAD_MUTEXATTR_FLAG_PSHARED
00110000000000000000000000000000 PTHREAD_MUTEXATTR_PROTOCOL_MASK
00000000111111111111000000000000 PTHREAD_MUTEXATTR_PRIO_CEILING_MASK



#define lll_futex_syscall(nargs, futexp, op, ...)                       \
  ({                                                                    \
    long int __ret = internal_syscall4(__NR_futex, __err, futexp, __lll_private_flag (FUTEX_WAIT, private), val, timeout);
    (__glibc_unlikely (INTERNAL_SYSCALL_ERROR_P (__ret, __err))         \
     ? -INTERNAL_SYSCALL_ERRNO (__ret, __err) : 0);                     \
  })

#define lll_futex_wait(futexp, val, private) \
  lll_futex_syscall (4, futexp, __lll_private_flag (FUTEX_WAIT, private), val, timeout)
  

#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL(name, err, nr, args...)			\
	internal_syscall##nr (SYS_ify (name), err, args)


#define SYS_ify(syscall_name)	__NR_##syscall_name

#undef internal_syscall4
#define internal_syscall4(number, err, arg1, arg2, arg3, arg4)		\
({									\
    unsigned long int resultvar;					\
    TYPEFY (arg4, __arg4) = ARGIFY (arg4);			 	\
    TYPEFY (arg3, __arg3) = ARGIFY (arg3);			 	\
    TYPEFY (arg2, __arg2) = ARGIFY (arg2);			 	\
    TYPEFY (arg1, __arg1) = ARGIFY (arg1);			 	\
    register TYPEFY (arg4, _a4) asm ("r10") = __arg4;			\
    register TYPEFY (arg3, _a3) asm ("rdx") = __arg3;			\
    register TYPEFY (arg2, _a2) asm ("rsi") = __arg2;			\
    register TYPEFY (arg1, _a1) asm ("rdi") = __arg1;			\
    asm volatile (							\
    "syscall\n\t"							\
    : "=a" (resultvar)							\
    : "0" (number), "r" (_a1), "r" (_a2), "r" (_a3), "r" (_a4)		\
    : "memory", REGISTERS_CLOBBERED_BY_SYSCALL);			\
    (long int) resultvar;						\
})

#define __NR_futex              202
   __SYSCALL(__NR_futex, sys_futex)



每个需要等待的task对应一个futex_q结构
