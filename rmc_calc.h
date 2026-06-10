#ifndef RMC_CALC_H_
#define RMC_CALC_H_

int32_t map_r(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
int32_t clip_r(int32_t x, int32_t out_min, int32_t out_max);

#endif /* RMC_CALC_H_ */