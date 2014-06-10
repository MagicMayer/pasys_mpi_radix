#if !defined(__TWEET_ERRORS_H__)
#define __TWEET_ERRORS_H__  1

/* may be replaced by a more clever Logging Routine later */
#define LOG_ERROR(...) fprintf(stderr, ##__VA_ARGS__); 

#define ASSERT_NULL_ARG(var, ret, eno) \
    do { \
        if(!(var)) { \
            errno = eno; \
            LOG_ERROR("<error : %s> (%d) %s '" #var "'\n", __func__, eno, strerror(eno)); \
            return(ret); \
        } \
    } while(0)

#endif /* __TWEET_ERROR_H__ */
