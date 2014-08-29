#ifndef __AUKaraoke__BiquadFilter__
#define __AUKaraoke__BiquadFilter__

#include <iostream>

#include <math.h>
#include <stdlib.h>

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum { kBiquadFilter_BPF = 0, kBiquadFilter_Notch = 1, kBiquadFilter_Type = 2 };

class BiquadFilter {
 public:
  BiquadFilter();
  BiquadFilter(int _type, int _sf);

  void set(int _cutoff, float _q);
  float get(float _x);
  void setBPF();
  void setNotch();

  int type, sf, cutoff;
  float q;
  float a0, a1, a2, b0, b1, b2;
  float x1, x2, y1, y2;  // x1 = x[n-1], x2 = x[n-2], y1 = y[n-1], y2 = y[n-2]
};

#endif /* defined(__AUKaraoke__BiquadFilter__) */
