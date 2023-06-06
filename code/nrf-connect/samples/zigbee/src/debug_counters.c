#include "debug_counters.h"

#include <prstlib/macros.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(debug_counters, CONFIG_LOG_DEFAULT_LEVEL);

static const char kDebugCountersDir[] = "/lfs/debug_counters";

#define PRST_MAX_COUNTER_NAME_LENGTH 64
#define PRST_MAX_FILE_NAME_LENGTH (sizeof(kDebugCountersDir) + 1 + PRST_MAX_COUNTER_NAME_LENGTH)

static int mk_filename(const char* counter_name, char* buff, size_t buff_size) {
  RET_CHECK(strlen(counter_name) <= PRST_MAX_FILE_NAME_LENGTH, "Counter name too long");
  strcpy(buff, kDebugCountersDir);
  strcat(buff, "/");
  strcat(buff, counter_name);
  return 0;
}

int prst_debug_counters_init() {
  struct fs_dirent entry;
  int err = fs_stat(kDebugCountersDir, &entry);
  if (err == 0) {
    LOG_DBG("Directory %s already exists", kDebugCountersDir);
    return 0;
  } else if (err == -ENOENT) {
    LOG_DBG("Creating directory %s", kDebugCountersDir);
    return fs_mkdir(kDebugCountersDir);
  }
  LOG_ERR("Unexpected error in fs_stat for %s: %d", kDebugCountersDir, err);
  return err;
}

int prst_debug_counters_increment(const char* counter_name) {
  char filename[PRST_MAX_FILE_NAME_LENGTH];
  RET_IF_ERR(mk_filename(counter_name, filename, sizeof(filename)));

  LOG_DBG("Incrementing counter %s", filename);

  // Open the file.
  struct fs_file_t file;
  fs_file_t_init(&file);
  RET_IF_ERR(fs_open(&file, filename, FS_O_CREATE | FS_O_RDWR));

  // Read the current value.
  prst_debug_counter_t value;
  int n_read = fs_read(&file, &value, sizeof(value));
  if (n_read != sizeof(value)) {
    LOG_WRN("fs_read returned %d, expected %d, assuming first access", n_read, sizeof(value));
    value = 0;
  }

  // Increment the value.
  value++;

  // Write back to file.
  RET_CHECK(fs_seek(&file, 0, SEEK_SET) == 0, "Unable to seek");
  ssize_t written = fs_write(&file, &value, sizeof(value));
  if (written != sizeof(value)) {
    LOG_ERR("fs_write returned %d, expected %d", written, sizeof(value));
  }
  return fs_close(&file);
}

int prst_debug_counters_get(const char* counter_name, prst_debug_counter_t* value) {
  char filename[PRST_MAX_FILE_NAME_LENGTH];
  RET_IF_ERR(mk_filename(counter_name, filename, sizeof(filename)));

  LOG_DBG("Getting counter %s", filename);

  // Open the file.
  struct fs_file_t file;
  fs_file_t_init(&file);
  RET_IF_ERR(fs_open(&file, filename, FS_O_CREATE | FS_O_READ));

  // Read the current value.
  int n_read = fs_read(&file, value, sizeof(value));
  if (n_read != sizeof(prst_debug_counter_t)) {
    LOG_WRN("fs_read returned %d, expected %d, assuming first access", n_read, sizeof(prst_debug_counter_t));
    // return -1;
    *value = 0;
  }

  return fs_close(&file);
}

int prst_debug_counters_get_all(prst_debug_counters_callback_t callback) {
  LOG_DBG("Getting all counters from %s", kDebugCountersDir);
  struct fs_dir_t dirp;
  fs_dir_t_init(&dirp);

  static struct fs_dirent entry;
  RET_IF_ERR(fs_opendir(&dirp, kDebugCountersDir));
  prst_debug_counter_t value;
  for (;;) {
    RET_IF_ERR(fs_readdir(&dirp, &entry));
    // End of directory;
    if (entry.name[0] == 0) {
      break;
    }
    LOG_DBG("Found %s", entry.name);
    prst_debug_counters_get(entry.name, &value);
    callback(entry.name, value);
  }
  return fs_closedir(&dirp);
}

typedef struct {
  struct k_work work;
  const char* counter_name;
} counter_increment_work_t;

static void increment_counter_callback(struct k_work* work) {
  counter_increment_work_t* counter_work = CONTAINER_OF(work, counter_increment_work_t, work);
  LOG_DBG("Incrementing counter callback for %s", counter_work->counter_name);
  prst_debug_counters_increment(counter_work->counter_name);
  free(counter_work);
  LOG_DBG("Freed memory for %s", counter_work->counter_name);
}

int prst_debug_counters_increment_async(const char* counter_name) {
  counter_increment_work_t* work = malloc(sizeof(counter_increment_work_t));
  work->counter_name = counter_name;
  k_work_init(&work->work, increment_counter_callback);
  LOG_DBG("Scheduling increment of %s", counter_name);
  return k_work_submit(&work->work);
}