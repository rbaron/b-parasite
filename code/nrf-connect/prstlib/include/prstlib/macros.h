#ifndef _PRST_MACROS_H_
#define _PRST_MACROS_H_

#define PRST_STRINGIFY(x) #x
#define PRST_TO_STRING(x) PRST_STRINGIFY(x)
#define PRST_LOCATION __FILE__ ":" PRST_TO_STRING(__LINE__)

#define RET_IF_ERR_MSG(expr, msg)                          \
  {                                                        \
    int err = (expr);                                      \
    if (err) {                                             \
      LOG_ERR("Error %d: " msg " in " PRST_LOCATION, err); \
      return err;                                          \
    }                                                      \
  }

#define RET_IF_ERR(expr) RET_IF_ERR_MSG(expr, "")

// Checks that expr evaluates to true, otherwise return 1.
#define RET_CHECK(expr, msg) RET_IF_ERR_MSG(!(expr), msg)

#define UNUSED_OK(expr) (void)expr;

#endif  // _PRST_MACROS_H_