#include "mleak.h"

void *malloc(size_t size) {
  return tbsys::mleak_inernal_malloc(tbsys::MleakOpType::kMalloc, size);
}

void *calloc(size_t nmemb, size_t size) {
  return tbsys::mleak_inernal_calloc(tbsys::MleakOpType::kCalloc, nmemb, size);
}

void *realloc(void *ptr, size_t size) {
  return tbsys::mleak_inernal_realloc(tbsys::MleakOpType::kRealloc, ptr, size);
}

void *memalign(size_t alignment, size_t size) {
  return tbsys::mleak_inernal_memalign(tbsys::MleakOpType::kMemalign, alignment, size);
}

void *valloc(size_t size) {
  return tbsys::mleak_inernal_memalign(tbsys::MleakOpType::kValloc, getpagesize(), size);
}

void free(void *ptr) {
  tbsys::mleak_inernal_free(tbsys::MleakOpType::kFree, ptr);
}

/* new */
void *operator new(size_t size) {
  return tbsys::mleak_inernal_malloc(tbsys::MleakOpType::kNew, size);
}

/* delete */
void operator delete(void *ptr) {
  tbsys::mleak_inernal_free(tbsys::MleakOpType::kDelete, ptr);
}

/* new[] */
void *operator new[](size_t size) {
  return tbsys::mleak_inernal_malloc(tbsys::MleakOpType::kNewArr, size);
}

/* delete[] */
void operator delete[](void *ptr) {
  tbsys::mleak_inernal_free(tbsys::MleakOpType::kDeleteArr, ptr);
}

namespace tbsys {

/* conversion from inner to outer pointer, and back */
#define outer_ptr(p) ((void *)((char *)(p) + sizeof(size_t)))
#define inner_ptr(p) ((void *)((char *)(p) - sizeof(size_t)))

#define alignment_outer_ptr(p, alignment) ((void *)((char *)(p) + alignment))
#define alignment_inner_ptr(p, alignment) ((void *)((char *)(p) - alignment))

/* initialize allocated memory header to 0 */
#define init_mem_header(p) (memset(p, 0, sizeof(size_t)))
#define init_alignment_header(p, align) (memset(p, 0, align))
#define set_alignment_header(p, align) (memcpy((char *)(p) + align - sizeof(size_t), &align, sizeof(align)))


static bool mleak_bootstrapped = false;
static char mleak_bootstrap_buffer[64];
static bool mleak_has_called_dlsym = false;
static void (*real_free)(void *) = nullptr;
static int g_mleak_probability_max = 1000000;  // max 100W

std::mt19937 g_mleak_gen;
bool g_need_print_mleak = false;
int g_mleak_probability = 0;  // default print probability is 0
std::atomic<int64_t> g_mleak_memuse_size = {0};

MleakLogger g_mleak_logger;


int get_printed_stack_deep(const MleakOpType& op_type) {
  if (op_type == MleakOpType::kNew ||
      op_type == MleakOpType::kNewArr ||
      op_type == MleakOpType::kDelete ||
      op_type == MleakOpType::kDeleteArr) {
    return 4;
  } else if (op_type == MleakOpType::kMalloc ||
             op_type == MleakOpType::kCalloc ||
             op_type == MleakOpType::kValloc ||
             op_type == MleakOpType::kRealloc ||
             op_type == MleakOpType::kMemalign ||
             op_type == MleakOpType::kFree) {
    return 3;
  }
  return 3;
}

void load_mleak_params(const MleakParams &params) {
  stop_mleak_check();
  g_mleak_probability = params.probability;
  g_mleak_logger.SetLogName(params.log_file_name);
  start_mleak_check();
}

void get_mleak_params(MleakParams *params) {
  params->probability = g_mleak_probability;
  params->log_file_name = g_mleak_logger.GetLogName();
}

void start_mleak_check() {
  if (g_mleak_probability > 0 && g_mleak_probability <= g_mleak_probability_max) {
    g_mleak_gen.seed(17);
    g_mleak_logger.Open();
    g_need_print_mleak = true;
  } else {
    g_need_print_mleak = false;
  }
}

void mleak_get_statistics(int64_t *memuse_size) {
  if (memuse_size != nullptr) {
    *memuse_size = g_mleak_memuse_size.load(std::memory_order_relaxed);
  }
}

void mleak_inernal_free(MleakOpType op_type, void *ptr) {
  if (real_free == nullptr) {
    real_free = (void (*)(void *))dlsym(RTLD_NEXT, "free");
  }
  if (ptr == nullptr) {
    return;
  }
  void *real_ptr = inner_ptr(ptr);
  size_t free_args;
  memcpy(&free_args, real_ptr, sizeof(free_args));
  if (free_args > 1) {
    real_ptr = alignment_inner_ptr(ptr, free_args);
  }
  size_t malloc_size = malloc_usable_size(real_ptr);
  g_mleak_memuse_size.fetch_sub(malloc_size, std::memory_order_relaxed);
  if (g_need_print_mleak) {
    if (free_args == 1) {
      print_stacktrace(op_type, real_ptr, malloc_size);
    }
  }
  real_free(real_ptr);
}

void *mleak_inernal_malloc(MleakOpType op_type, size_t size) {
  static void *(*real_malloc)(size_t);
  if (real_malloc == nullptr) {
    mleak_has_called_dlsym = true;
    real_malloc = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
  }
  if (size == 0) {
    return nullptr;
  }
  void *ptr = real_malloc(size + sizeof(size_t));
  if (ptr != nullptr) {
    size_t malloc_size = malloc_usable_size(ptr);
    g_mleak_memuse_size.fetch_add(malloc_size, std::memory_order_relaxed);
    init_mem_header(ptr);
    if (g_need_print_mleak) {
      size_t do_print = g_mleak_probability > 0 &&
        (g_mleak_gen() % g_mleak_probability_max) <= static_cast<uint32_t>(g_mleak_probability)
            ? 1
            : 0;
      if (do_print) {
        memcpy(ptr, &do_print, sizeof(do_print));
        print_stacktrace(op_type, ptr, malloc_size);
      }
    }
    if (real_free == nullptr) {
      real_free = (void (*)(void *))dlsym(RTLD_NEXT, "free");
    }
    return outer_ptr(ptr);
  }
  return nullptr;
}

void *mleak_inernal_calloc(MleakOpType op_type, size_t nmemb, size_t size) {
  static void *(*real_calloc)(size_t, size_t);
  if (real_calloc == nullptr) {
    if (mleak_has_called_dlsym && !mleak_bootstrapped) {
      assert(nmemb * size + sizeof(size_t) <= 64);
      mleak_bootstrapped = true;
      init_mem_header(mleak_bootstrap_buffer);
      return outer_ptr(mleak_bootstrap_buffer);
    }
    mleak_has_called_dlsym = true;
    real_calloc = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
  }
  void *ptr = real_calloc(nmemb * size + sizeof(size_t), 1);
  if (ptr != nullptr) {
    size_t malloc_size = malloc_usable_size(ptr);
    g_mleak_memuse_size.fetch_add(malloc_size, std::memory_order_relaxed);
    init_mem_header(ptr);
    if (g_need_print_mleak) {
      size_t do_print = g_mleak_probability > 0 &&
        (g_mleak_gen() % g_mleak_probability_max) <= static_cast<uint32_t>(g_mleak_probability)
            ? 1
            : 0;
      if (do_print) {
        memcpy(ptr, &do_print, sizeof(do_print));
        print_stacktrace(op_type, ptr, malloc_size);
      }
    }
    return outer_ptr(ptr);
  }

  if (real_free == nullptr) {
    real_free = (void (*)(void *))dlsym(RTLD_NEXT, "free");
  }
  return nullptr;
}

void *mleak_inernal_realloc(MleakOpType op_type, void *ptr, size_t size) {
  static void *(*real_realloc)(void *, size_t);
  if (real_realloc == nullptr) {
    mleak_has_called_dlsym = true;
    real_realloc = (void *(*)(void *, size_t))dlsym(RTLD_NEXT, "realloc");
  }
  if (ptr == nullptr) {
    return tbsys::mleak_inernal_malloc(tbsys::MleakOpType::kRealloc, size);
  }
  void *inner_ptr = inner_ptr(ptr);
  // internal of realloc will free inner_ptr
  g_mleak_memuse_size.fetch_sub(malloc_usable_size(inner_ptr), std::memory_order_relaxed);
  void *new_ptr = real_realloc(inner_ptr, size + sizeof(size_t));
  if (new_ptr != nullptr) {
    size_t malloc_size = malloc_usable_size(new_ptr);
    g_mleak_memuse_size.fetch_add(malloc_size, std::memory_order_relaxed);
    init_mem_header(new_ptr);
    if (g_need_print_mleak) {
      size_t do_print = g_mleak_probability > 0 &&
        (g_mleak_gen() % g_mleak_probability_max) <= static_cast<uint32_t>(g_mleak_probability)
            ? 1
            : 0;
      if (do_print) {
        memcpy(new_ptr, &do_print, sizeof(do_print));
        print_stacktrace(op_type, new_ptr, malloc_size);
      }
    }
    return outer_ptr(new_ptr);
  }
  return nullptr;
}

void *mleak_inernal_memalign(MleakOpType op_type, size_t alignment, size_t size) {
  static void *(*real_memalign)(size_t, size_t);
  if (real_memalign == nullptr) {
    mleak_has_called_dlsym = true;
    real_memalign = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "memalign");
  }
  size_t alignment_size = alignment < sizeof(size_t) ? sizeof(size_t) : alignment;
  void *ptr = real_memalign(alignment, size + alignment_size);
  if (ptr != nullptr) {
    size_t malloc_size = malloc_usable_size(ptr);
    g_mleak_memuse_size.fetch_add(malloc_size, std::memory_order_relaxed);
    init_alignment_header(ptr, alignment_size);
    set_alignment_header(ptr, alignment_size);
    return alignment_outer_ptr(ptr, alignment_size);
  }
  return nullptr;
}

void print_stacktrace(MleakOpType op_type, void *ptr, size_t size) {
  int deep = 16;
  void *stack_array[deep];
  int stack_num = backtrace(stack_array, deep);
  int deep_from = get_printed_stack_deep(op_type);

  char buf[256];
  int bufsize = sizeof(buf);
  char *p = buf;
  p += snprintf(p, bufsize, "%d,%p,%zu", static_cast<int>(op_type), ptr, size);
  for (int i = deep_from; i < stack_num - 2; ++i) {
    p += snprintf(p, bufsize, ",%p", stack_array[i]);
  }
  *p++ = '\n';
  g_mleak_logger.Logv(buf, p - buf);
}

void stop_mleak_check() {
  if (g_need_print_mleak == true) {
    g_need_print_mleak = false;
    g_mleak_logger.Close();
  }
}

}  // namespace tbsys