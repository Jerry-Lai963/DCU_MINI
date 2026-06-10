#include <stdint.h>

int32_t map_r(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
  int32_t res;

  if(x< in_min) x = in_min;
  if(x> in_max) x = in_max;
  res = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

  if(res < out_min) res = out_min;
  if(res > out_max) res = out_max;
  return res;
}

int32_t clip_r(int32_t x, int32_t out_min, int32_t out_max) {
  if(x < out_min) x = out_min;
  if(x > out_max) x = out_max;
  return x;
}