/**
 * This is a memory malloc and free trace tools, use it in your code,
 * like this:
 *
 * your_function() {
 *   MleakParams mleak_params;
 *   ...
 *   load_mleak_params(mleak_params);
 *   start_mleak_check();
 *   ...
 *   ... your code
 *   ...
 *   stop_mleak_check();
 *   ...
 * }
 *
 * Use "mleak_analyzer.py" for analyze mleak_log.* is friendly
 *
 * Author: baorenyi <baorenyi@meituan.com>
 **/

#pragma once

#include <assert.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <atomic>
#include <random>

namespace tbsys {

enum class MleakOpType {
  kUnknown = 0,
  kMalloc = 1,
  kCalloc = 2,
  kRealloc = 3,
  kValloc = 4,
  kMemalign = 5,
  kFree = 6,
  kNew = 7,
  kNewArr = 8,
  kDelete = 9,
  kDeleteArr = 10,
};

struct MleakParams {
  int probability = 0;
  const char *log_file_name = nullptr;
};


int get_printed_stack_deep(const MleakOpType& op_type);

/**
 * print stacktrace address array to log
 * format:
 *  (1) malloc and free family
 *  op_type|ptr|size|stack_addr2|stack_addr3|....
 *
 *  (2) new and delete family
 *  op_type|ptr|size|stack_addr3|stack_addr4|....
 *
 *  the size of address array <=16
 */
void print_stacktrace(MleakOpType op_type, void *ptr, size_t size);

/**
 * all *alloc* and new* call this to malloc memory
 */
void *mleak_inernal_malloc(MleakOpType op_type, size_t size);
void *mleak_inernal_calloc(MleakOpType op_type, size_t nmemb, size_t size);
void *mleak_inernal_realloc(MleakOpType op_type, void *ptr, size_t size);
void *mleak_inernal_memalign(MleakOpType op_type, size_t alignment, size_t size);

/**
 * all free and delete* call this to free memory
 */
void mleak_inernal_free(MleakOpType op_type, void *ptr);

bool open_mleak_log_file();

/**
 * get real-time malloc_size and free_size
 */
void mleak_get_statistics(int64_t *memuse_size);

/**
 * you can change some params by this function
 * internal of this function would call stop_mleak_check() and
 * start_mleak_check()
 */
void load_mleak_params(const MleakParams &parmas);

void get_mleak_params(MleakParams *params);

/**
 * check enable_mleak_check, set seed for probability and open log file
 */
void start_mleak_check();

/**
 * set print_flag and close log file
 */
void stop_mleak_check();

/**
 * a lightweight logger for flush message to disk
 * default flush batch 4K, like pagecache
 */
class MleakLogger {
 public:
  MleakLogger()
      : file_(nullptr),
        log_seq_(0),
        closing_(false),
        ref_cnt_(0) {}

  ~MleakLogger() { Close(); }

  void Close() {
    closing_ = true;
    if (file_ != nullptr) {
      while (ref_cnt_ > 0) {
        fprintf(stderr, "waiting for close mleak log\n");
        usleep(100);
      }
      fclose(file_);
      file_ = nullptr;
    }
  }

  void SetLogName(const char* filename) {
    snprintf(log_name_, k_log_name_len_, "%s", filename);
  }

  const char* GetLogName() const {
    return log_name_;
  }

  MleakLogger(const MleakLogger &) = delete;
  void operator=(const MleakLogger &) = delete;

  void Logv(const char *buf, size_t bufsize) {
    ++ref_cnt_;
    if (file_ && !closing_) {
      fwrite(buf, 1, bufsize, file_);
    }
    --ref_cnt_;
  }

  bool Open() {
    char log_file_name[k_log_name_len_];
    snprintf(log_file_name, sizeof(log_file_name), "%s.%d", log_name_,
           getpid());
    if (access(log_file_name, F_OK) == 0) {
      char bak_file_name[k_log_name_len_];
      snprintf(bak_file_name, sizeof(bak_file_name), "%s.%d.%d", log_name_,
             getpid(), log_seq_++);
      rename(log_file_name, bak_file_name);
    }
    file_ = fopen(log_file_name, "w");
    if (file_ == nullptr) {
      fprintf(stderr, "open log file %s failed\n", log_file_name);
      return false;
    }
    closing_ = false;
    ref_cnt_ = 0;
    return true;
  }


 private:
  FILE *file_;
  static const uint64_t k_log_name_len_ = 1024;
  char log_name_[k_log_name_len_];
  int log_seq_;
  bool closing_;
  std::atomic<int64_t> ref_cnt_;
};

}  // namespace tbsys