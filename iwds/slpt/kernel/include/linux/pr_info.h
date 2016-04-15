#ifndef _PR_INFO_H_
#define _PR_INFO_H_

#ifdef DEBUG
#define PR_INFO_DEBUG 1
#else
#define PR_INFO_DEBUG 0
#endif

#ifndef PR_ERR_DEBUG
#define PR_ERR_DEBUG 0
#endif

extern int slpt_kernel_printf(const char *fmt, ...);

#ifndef pr_info
#define pr_err(args...)                         \
    do {                                        \
        if (PR_ERR_DEBUG)                       \
            slpt_kernel_printf(args);           \
    } while (0)

#define pr_info(args...)                        \
    do {                                        \
        if (PR_INFO_DEBUG == 1)                 \
            slpt_kernel_printf(args);           \
    } while (0)

#define pr_debug(args...) pr_info(args)
#endif

#endif /* _PR_INFO_H_ */
