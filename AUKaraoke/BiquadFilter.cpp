#include "BiquadFilter.h"

BiquadFilter::BiquadFilter() {
  a0 = a1 = a2 = b0 = b1 = b2 = 0.0f;
  x1 = x2 = y1 = y2 = 0.0f;
}

BiquadFilter::BiquadFilter(int _type, int _sf) {
  a0 = a1 = a2 = b0 = b1 = b2 = 0.0f;
  x1 = x2 = y1 = y2 = 0.0f;
  type = _type;
  sf = _sf;
  cutoff = 10;
  q = 1.0f;
}

void BiquadFilter::set(int _cutoff, float _q) {
  cutoff = _cutoff;
  q = _q;
}

float BiquadFilter::get(float _x) {
  switch (type) {
    case kBiquadFilter_BPF:
      setBPF();
      break;
    case kBiquadFilter_Notch:
      setNotch();
      break;
  }

  float ret = (b0 / a0) * _x + (b1 / a0) * x1 + (b2 / a0) * x2 -
              (a1 / a0) * y1 - (a2 / a0) * y2;
  x2 = x1;
  x1 = _x;
  y2 = y1;
  y1 = ret;
  return ret;
}

void BiquadFilter::setBPF() {
  float w0 = 2 * M_PI * cutoff / sf;
  float alpha = sin(w0) / (2 * q);
  float cs = cos(w0);

  b0 = alpha;
  b1 = 0;
  b2 = -alpha;
  a0 = 1 + alpha;
  a1 = -2 * cs;
  a2 = 1 - alpha;
}

void BiquadFilter::setNotch() {
  float w0 = 2 * M_PI * cutoff / sf;
  float alpha = sin(w0) / (2 * q);
  float cs = cos(w0);

  b0 = 1;
  b1 = -2 * cs;
  b2 = 1;
  a0 = 1 + alpha;
  a1 = -2 * cs;
  a2 = 1 - alpha;
}
