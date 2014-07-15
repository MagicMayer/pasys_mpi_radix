#ifndef __pasys_radix_h__
#define __pasys_radix_h__   1

#define LVLHITS_BUCKETSIZE  254
#define LVLMONTH_BUCKETSIZE  12

#define CN_MATRIX_MAXSIZE           (LVLHITS_BUCKETSIZE * (LVLMONTH_BUCKETSIZE + 1))
#define CN_MATRIX_OFFSET(i)         (i * (LVLMONTH_BUCKETSIZE + 1))
#define CN_MATRIX_OFFSET_TW(x)      (CN_MATRIX_OFFSET(x->hits))
#define CN_MATRIX_OFFSET_DIM(x,y)   (CN_MATRIX_OFFSET_TW(x) + y)

int count_elements(tweet_t *);

#endif /* __pasys_radix_h__ */
