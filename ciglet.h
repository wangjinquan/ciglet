#ifndef CIGLET_H
#define CIGLET_H

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>

#ifndef CIGLET_SINGLE_FILE
  #include "external/fastapprox-all.h"
#endif

// scalar operations

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

static inline FP_TYPE linterp(FP_TYPE a, FP_TYPE b, FP_TYPE ratio) {
    return a + (b - a) * ratio;
}

/*
  For higher efficiency we would like to replace some std math functions by their
    approximated versions.
  However we also need to be careful with approximation error.
  The numbers after function names listed below indicate the level of precision,
    the higher the better (but slower).
*/
#define sin_3 sin
#define sin_2 fastsinfull
#define sin_1 fastersinfull
#define cos_3 cos
#define cos_2 fastcosfull
#define cos_1 fastercosfull
#define exp_3 exp
#define exp_2 fastexp
#define exp_1 fasterexp
#define log_3 log
#define log_2 fastlog
#define log_1 fasterlog
#define atan2_3 atan2
#define atan2_2 atan2
#define atan2_1 fastatan2

#ifndef M_PI
  #define M_PI    3.14159265358979323846264
#endif
#ifndef M_PI_2
  #define M_PI_2  1.57079632679489661923132
#endif
#define M_EPS 1e-15
#define M_INF 1e15

// https://gist.github.com/volkansalma/2972237
static inline FP_TYPE fastatan2(FP_TYPE y, FP_TYPE x) {
  if(x == 0.0f) {
    if(y > 0.0f) return M_PI_2;
    if(y == 0.0f) return 0.0f;
    return -M_PI_2;
  }
  FP_TYPE atan;
  FP_TYPE z = y / x;
  if(fabs(z) < 1.0f) {
    atan = z / (1.0f + 0.28f * z * z);
    if(x < 0.0f) {
      if(y < 0.0f) return atan - M_PI;
      return atan + M_PI;
    }
  } else {
    atan = M_PI_2 - z / (z * z + 0.28f);
    if(y < 0.0f) return atan - M_PI;
  }
  return atan;
}

static inline FP_TYPE randu() {
  return ((FP_TYPE)rand() / RAND_MAX - 0.5) * 2.0;
}

static inline FP_TYPE randn(FP_TYPE u, FP_TYPE v) {
  FP_TYPE u1 = (FP_TYPE)rand() / RAND_MAX;
  FP_TYPE u2 = (FP_TYPE)rand() / RAND_MAX;
  return sqrt(-2.0 * log_2(u1) * v) * cos_2(2.0 * M_PI * u2) + u;
}

// complex scalar arithmetics

typedef struct {
  FP_TYPE real;
  FP_TYPE imag;
} cplx;

static inline cplx c_cplx(FP_TYPE real, FP_TYPE imag) {
  cplx ret;
  ret.real = real;
  ret.imag = imag;
  return ret;
}

static inline cplx c_conj(cplx a) {
  a.imag = -a.imag;
  return a;
}

static inline cplx c_add(cplx a, cplx b) {
  cplx ret;
  ret.real = a.real + b.real;
  ret.imag = a.imag + b.imag;
  return ret;
}

static inline cplx c_sub(cplx a, cplx b) {
  cplx ret;
  ret.real = a.real - b.real;
  ret.imag = a.imag - b.imag;
  return ret;
}

static inline cplx c_mul(cplx a, cplx b) {
  cplx ret;
  ret.real = a.real * b.real - a.imag * b.imag;
  ret.imag = a.real * b.imag + a.imag * b.real;
  return ret;
}

static inline cplx c_div(cplx a, cplx b) {
  cplx ret;
  FP_TYPE denom = b.real * b.real + b.imag * b.imag;
  ret.real = (a.real * b.real + a.imag * b.imag) / denom;
  ret.imag = (a.imag * b.real - a.real * b.imag) / denom;
  return ret;
}

#define c_exp c_exp_2

static inline cplx c_exp_3(cplx a) {
  cplx ret;
  FP_TYPE mag = exp_3(a.real);
  ret.real = mag * cos_3(a.imag);
  ret.imag = mag * sin_3(a.imag);
  return ret;
}

static inline cplx c_exp_2(cplx a) {
  cplx ret;
  FP_TYPE mag = exp_2(a.real);
  ret.real = mag * cos_2(a.imag);
  ret.imag = mag * sin_2(a.imag);
  return ret;
}

static inline cplx c_exp_1(cplx a) {
  cplx ret;
  FP_TYPE mag = exp(a.real);
  ret.real = mag * cos(a.imag);
  ret.imag = mag * sin(a.imag);
  return ret;
}

static inline FP_TYPE c_abs(cplx a) {
  return sqrt(a.real * a.real + a.imag * a.imag);
}

#define c_arg_3 c_arg_2
#define c_arg c_arg_2

static inline FP_TYPE c_arg_2(cplx a) {
  return atan2_2(a.imag, a.real);
}

static inline FP_TYPE c_arg_1(cplx a) {
  return atan2_1(a.imag, a.real);
}

// vector operations & statics

#define def_n_to_one(name, op, init) \
static inline FP_TYPE name(FP_TYPE* src, int n) { \
  FP_TYPE ret = init; \
  for(int i = 0; i < n; i ++) \
    ret = op(ret, src[i]); \
  return ret; \
}

#define def_add(a, b) ((a) + (b))
#define def_addsqr(a, b) ((a) + (b) * (b))
#define def_max(a, b) ((a) > (b) ? (a) : (b))
#define def_min(a, b) ((a) < (b) ? (a) : (b))

def_n_to_one(sumfp, def_add, 0);
def_n_to_one(sumsqrfp, def_addsqr, 0);
def_n_to_one(maxfp, def_max, src[0]);
def_n_to_one(minfp, def_min, src[0]);

#define meanfp(src, n) (sumfp(src, n) / (n))

static inline FP_TYPE varfp(FP_TYPE* src, int n) {
  FP_TYPE mean = meanfp(src, n);
  FP_TYPE sumsqr = sumsqrfp(src, n) / n;
  return sumsqr - mean * mean;
}

FP_TYPE cig_qselect(FP_TYPE* x, int nx, int n);
FP_TYPE cig_medianfp(FP_TYPE* x, int nx);

static inline FP_TYPE selectnth(FP_TYPE* x, int nx, int n) {
  return cig_qselect(x, nx, n);
}

static inline FP_TYPE medianfp(FP_TYPE* x, int nx) {
  return cig_medianfp(x, nx);
}

FP_TYPE* cig_sort(FP_TYPE* x, int nx, int* outidx);

static inline FP_TYPE* sort(FP_TYPE* x, int nx, int* outidx) {
  return cig_sort(x, nx, outidx);
}

static inline FP_TYPE cov(FP_TYPE* x, FP_TYPE* y, int nx) {
  FP_TYPE ux = meanfp(x, nx);
  FP_TYPE uy = meanfp(y, nx);
  FP_TYPE cov = 0;
  for(int i = 0; i < nx; i ++)
    cov += (x[i] - ux) * (y[i] - uy);
  return cov / nx;
}

static inline FP_TYPE corr(FP_TYPE* x, FP_TYPE* y, int nx) {
  return cov(x, y, nx) / sqrt(varfp(x, nx) * varfp(y, nx));
}

int cig_find_peak(FP_TYPE* x, int lidx, int uidx, int orient);

static inline FP_TYPE find_peak(FP_TYPE* x, int lidx, int uidx) {
  return cig_find_peak(x, lidx, uidx, 1);
}

static inline FP_TYPE find_valley(FP_TYPE* x, int lidx, int uidx) {
  return cig_find_peak(x, lidx, uidx, -1);
}

// memory (de)allocation

static inline FP_TYPE* linspace(FP_TYPE x0, FP_TYPE x1, int nx) {
  FP_TYPE* x = malloc(nx * sizeof(FP_TYPE));
  for(int i = 0; i < nx; i ++)
    x[i] = x0 + (x1 - x0) * i / (nx - 1);
  return x;
}

static inline int* iota(int x0, int step, int nx) {
  int* x = malloc(nx * sizeof(int));
  for(int i = 0; i < nx; i ++)
    x[i] = x0 + step * i;
  return x;
}

#define malloc2d(m, n, size) (void*)malloc2d_(m, n, size)
static inline void** malloc2d_(size_t m, size_t n, size_t size) {
  void** ret = calloc(m, sizeof(void*));
  for(size_t i = 0; i < m; i++)
    ret[i] = calloc(n, size);
  return ret;
}

#define copy2d(src, m, n, size) (void*)copy2d_((void**)src, m, n, size)
static inline void** copy2d_(void** src, size_t m, size_t n, size_t size) {
  void** ret = malloc2d(m, n, size);
  for(size_t i = 0; i < m; i ++)
    memcpy(ret[i], src[i], n * size);
  return ret;
}

#define free2d(ptr, m) free2d_((void**)(ptr), m)
static inline void free2d_(void** ptr, size_t m) {
  for(size_t i = 0; i < m; i ++)
    free(ptr[i]);
  free(ptr);
}

#define flatten(ptr, m, n, size) (void*)flatten_((void**)(ptr), m, n, size)
static inline void* flatten_(void** ptr, size_t m, size_t n, size_t size) {
  void* ret = malloc(size * m * n);
  for(size_t i = 0; i < m; i ++)
    memcpy(ret + i * n * size, ptr[i], n * size);
  return ret;
}

#define reshape(ptr, m, n, size) (void*)reshape_((void**)(ptr), m, n, size)
static inline void** reshape_(void* ptr, size_t m, size_t n, size_t size) {
  void** ret = malloc(m * sizeof(void*));
  for(size_t i = 0; i < m; i ++) {
    ret[i] = malloc(n * size);
    memcpy(ret[i], ptr + i * n * size, n * size);
  }
  return ret;
}

void** cig_transpose(void** ptr, size_t m, size_t n, size_t size);

#define transpose(ptr, m, n, size) (void*)cig_transpose((void**)(ptr), m, n, size)

// Audio I/O functions

FP_TYPE* wavread(char* filename, int* fs, int* nbit, int* nx);
void wavwrite(FP_TYPE* y, int ny, int fs, int nbit, char* filename);

// DSP functions

static inline FP_TYPE* fetch_frame(FP_TYPE* x, int nx, int center, int nf) {
  FP_TYPE* y = malloc(nf * sizeof(FP_TYPE));
  for(int i = 0; i < nf; i ++) {
    int isrc = center + i - nf / 2;
    y[i] = (isrc >= 0 && isrc < nx) ? x[isrc] : 0;
  }
  return y;
}

// generate a sinusoid given frequency, amplitude and its phase at n/2 position
static inline FP_TYPE* gensin(FP_TYPE freq, FP_TYPE ampl, FP_TYPE phse, int n, int fs) {
  FP_TYPE tpffs = 2.0 * M_PI / fs * freq;
  FP_TYPE c = 2.0 * cos_3(tpffs);
  FP_TYPE* s = calloc(n, sizeof(FP_TYPE));
  s[0] = cos_3(tpffs * (- n / 2) + phse);
  s[1] = cos_3(tpffs * (- n / 2 + 1) + phse);
  for(int t = 2; t < n; t ++) {
    s[t] = c * s[t - 1] - s[t - 2];
  }
  return s;
}

FP_TYPE* cig_gensins(FP_TYPE* freq, FP_TYPE* ampl, FP_TYPE* phse,
  int nsin, int fs, int n);

static inline FP_TYPE* gensins(FP_TYPE* freq, FP_TYPE* ampl, FP_TYPE* phse,
  int nsin, int fs, int n) {
  return cig_gensins(freq, ampl, phse, nsin, fs, n);
}

static inline FP_TYPE* boxcar(int n) {
  FP_TYPE* ret = malloc(n * sizeof(FP_TYPE));
  for(int i = 0; i < n; i ++)
    ret[i] = 1.0;
  return ret;
}

static inline FP_TYPE* hanning(int n) {
  FP_TYPE* ret = malloc(n * sizeof(FP_TYPE));
  for(int i = 0; i < n; i ++)
    ret[i] = 0.5 * (1 - cos_3(2 * M_PI * i / (n - 1)));
  return ret;
}

static inline FP_TYPE* hamming(int n) {
  FP_TYPE* ret = malloc(n * sizeof(FP_TYPE));
  for(int i = 0; i < n; i ++)
    ret[i] = 0.54 - 0.46 * cos_3(2 * M_PI * i / (n - 1));
  return ret;
}

static inline FP_TYPE* mltsine(int n) {
  FP_TYPE* ret = malloc(n * sizeof(FP_TYPE));
  for(int i = 0; i < n; i ++)
    ret[i] = sin_3(M_PI / n * (i + 0.5));
  return ret;
}

static inline FP_TYPE* blackman_harris(int n) {
  FP_TYPE* ret = malloc(n * sizeof(FP_TYPE));
  const FP_TYPE a0 = 0.35875;
  const FP_TYPE a1 = 0.48829;
  const FP_TYPE a2 = 0.14128;
  const FP_TYPE a3 = 0.01168;
  for(int i = 0; i < n; i ++)
    ret[i] = a0 - a1 * cos_3(2.0 * M_PI * i / n) +
                  a2 * cos_3(4.0 * M_PI * i / n) -
                  a3 * cos_3(6.0 * M_PI * i / n);
  return ret;
}

static inline FP_TYPE* blackman(int n) {
  FP_TYPE* ret = malloc(n * sizeof(FP_TYPE));
  const FP_TYPE a0 = 0.42;
  const FP_TYPE a1 = 0.5;
  const FP_TYPE a2 = 0.08;
  for(int i = 0; i < n; i ++)
    ret[i] = a0 - a1 * cos_3(2.0 * M_PI * i / n) +
                  a2 * cos_3(4.0 * M_PI * i / n);
  return ret;
}

void cig_fft(FP_TYPE* xr, FP_TYPE* xi, FP_TYPE* yr, FP_TYPE* yi,
  int n, FP_TYPE* buffer, FP_TYPE mode);

static inline void fft(FP_TYPE* xr, FP_TYPE* xi, FP_TYPE* yr, FP_TYPE* yi,
  int n, FP_TYPE* buffer) {
  cig_fft(xr, xi, yr, yi, n, buffer, -1.0);
}

static inline void ifft(FP_TYPE* xr, FP_TYPE* xi, FP_TYPE* yr, FP_TYPE* yi,
  int n, FP_TYPE* buffer) {
  cig_fft(xr, xi, yr, yi, n, buffer, 1.0);
}

void cig_idft(FP_TYPE* xr, FP_TYPE* xi, FP_TYPE* yr, FP_TYPE* yi, int n);

static inline void idft(FP_TYPE* xr, FP_TYPE* xi, FP_TYPE* yr, FP_TYPE* yi, int n) {
  cig_idft(xr, xi, yr, yi, n);
}

FP_TYPE* cig_dct(FP_TYPE* x, int nx);

static inline FP_TYPE* dct(FP_TYPE* x, int nx) {
  return cig_dct(x, nx);
}

static inline FP_TYPE* fftshift(FP_TYPE* x, int n) {
  FP_TYPE* y = malloc(n * sizeof(FP_TYPE));
  int halfs = n / 2;
  int halfl = (n + 1) / 2;
  for(int i = 0; i < halfs; i ++)
    y[i] = x[i + halfl];
  for(int i = 0; i < halfl; i ++)
    y[i + halfs] = x[i];
  return y;
}

static inline FP_TYPE* unwrap(FP_TYPE* x, int n) {
  FP_TYPE* y = malloc(n * sizeof(FP_TYPE));
  y[0] = x[0];
  for(int i = 1; i < n; i ++) {
    if(fabs(x[i] - x[i - 1]) > M_PI)
      y[i] = y[i - 1] + x[i] - (x[i - 1] + 2.0 * M_PI * (x[i] > x[i - 1] ? 1.0 : -1.0));
    else
      y[i] = y[i - 1] + x[i] - x[i - 1];
  }
  return y;
}

static inline FP_TYPE wrap(FP_TYPE x) {
  return x - round(x / 2.0 / M_PI) * 2.0 * M_PI;
}

static inline FP_TYPE* diff(FP_TYPE* x, int nx) {
  FP_TYPE* y = malloc(nx * sizeof(FP_TYPE));
  y[0] = x[0];
  for(int i = 1; i < nx; i ++)
    y[i] = x[i] - x[i - 1];
  return y;
}

static inline FP_TYPE* cumsum(FP_TYPE* x, int nx) {
  FP_TYPE* y = malloc(nx * sizeof(FP_TYPE));
  y[0] = x[0];
  for(int i = 1; i < nx; i ++)
    y[i] = y[i - 1] + x[i];
  return y;
}

static inline FP_TYPE* flip(FP_TYPE* x, int nx) {
  FP_TYPE* y = malloc(nx * sizeof(FP_TYPE));
  for(int i = 0; i < nx; i ++)
    y[i] = x[nx - i - 1];
  return y;
}

static inline FP_TYPE* abscplx(FP_TYPE* xr, FP_TYPE* xi, int n) {
  FP_TYPE* y = malloc(n * sizeof(FP_TYPE));
  for(int i = 0; i < n; i ++)
    y[i] = sqrt(xr[i] * xr[i] + xi[i] * xi[i]);
  return y;
}

static inline FP_TYPE* argcplx(FP_TYPE* xr, FP_TYPE* xi, int n) {
  FP_TYPE* y = malloc(n * sizeof(FP_TYPE));
  for(int i = 0; i < n; i ++)
    y[i] = atan2_3(xi[i], xr[i]);
  return y;
}

static inline FP_TYPE* polar2real(FP_TYPE* xabs, FP_TYPE* xarg, int n) {
  FP_TYPE* y = malloc(n * sizeof(FP_TYPE));
  for(int i = 0; i < n; i ++)
    y[i] = xabs[i] * cos_2(xarg[i]);
  return y;
}

static inline FP_TYPE* polar2imag(FP_TYPE* xabs, FP_TYPE* xarg, int n) {
  FP_TYPE* y = malloc(n * sizeof(FP_TYPE));
  for(int i = 0; i < n; i ++)
    y[i] = xabs[i] * sin_2(xarg[i]);
  return y;
}

static inline FP_TYPE phase_diff(FP_TYPE p1, FP_TYPE p2) {
  if(p2 > p1) p1 += (int)((p2 - p1) / 2.0 / M_PI + 1) * 2.0 * M_PI;
  return fmod(p1 - p2 + M_PI, M_PI * 2.0) - M_PI;
}

static inline void complete_symm(FP_TYPE* x, int n) {
  if(n / 2 == (n + 1) / 2) // even
    x[n / 2] = x[n / 2 - 1];
  for(int i = n / 2 + 1; i < n; i ++)
    x[i] = x[n - i];
}

static inline void complete_asymm(FP_TYPE* x, int n) {
  if(n / 2 == (n + 1) / 2) // even
    x[n / 2] = x[n / 2 - 1];
  for(int i = n / 2 + 1; i < n; i ++)
    x[i] = -x[n - i];
}

static inline FP_TYPE* rceps(FP_TYPE* S, int nfft) {
  FP_TYPE* buff = malloc(nfft * 4 * sizeof(FP_TYPE));
  FP_TYPE* C = buff;
  FP_TYPE* S_symm = buff + nfft;
  FP_TYPE* fftbuff = buff + nfft * 2;
  for(int i = 0; i < nfft / 2 + 1; i ++)
    S_symm[i] = S[i];
  complete_symm(S_symm, nfft);
  ifft(S_symm, NULL, C, NULL, nfft, fftbuff);
  return realloc(C, nfft * sizeof(FP_TYPE));
}

static inline FP_TYPE* irceps(FP_TYPE* C, int nfft) {
  FP_TYPE* buff = malloc(nfft * 4 * sizeof(FP_TYPE));
  FP_TYPE* S = buff;
  FP_TYPE* C_symm = buff + nfft;
  FP_TYPE* fftbuff = buff + nfft * 2;
  for(int i = 0; i < nfft / 2 + 1; i ++)
    C_symm[i] = C[i];
  complete_symm(C_symm, nfft);
  fft(C_symm, NULL, S, NULL, nfft, fftbuff);
  return realloc(S, nfft * sizeof(FP_TYPE));
}

static inline FP_TYPE* minphase(FP_TYPE* S, int nfft) {
  FP_TYPE* buff = malloc(nfft * 4 * sizeof(FP_TYPE));
  FP_TYPE* S_symm = buff;
  FP_TYPE* C = buff + nfft;
  FP_TYPE* fftbuff = buff + nfft * 2;
  for(int i = 0; i < nfft / 2 + 1; i ++)
    S_symm[i] = S[i];
  complete_symm(S_symm, nfft);
  ifft(S_symm, NULL, C, NULL, nfft, fftbuff);
  for(int i = 1; i < nfft / 2 + 1; i ++)
    C[i] *= 2.0;
  for(int i = nfft / 2 + 2; i < nfft; i ++)
    C[i] = 0.0;
  fft(C, NULL, NULL, S_symm, nfft, fftbuff);
  return realloc(S_symm, nfft * sizeof(FP_TYPE));
}

FP_TYPE* cig_winfir(int order, FP_TYPE cutoff, FP_TYPE cutoff2,
  char* type, char* window);

static inline FP_TYPE* fir1(int order, FP_TYPE cutoff, char* type, char* window) {
  return cig_winfir(order, cutoff / 2.0, 0, type, window);
}

static inline FP_TYPE* fir1bp(int order, FP_TYPE cutoff_low, FP_TYPE cutoff_high,
  char* window) {
  return cig_winfir(order, cutoff_low / 2.0, cutoff_high / 2.0, "bandpass", window);
}

FP_TYPE* cig_convolution(FP_TYPE* x, FP_TYPE* h, int nx, int nh);

static inline FP_TYPE* conv(FP_TYPE* x, FP_TYPE* h, int nx, int nh) {
  return cig_convolution(x, h, nx, nh);
}

FP_TYPE* cig_filter(FP_TYPE* b, int nb, FP_TYPE* a, int na, FP_TYPE* x, int nx);

static inline FP_TYPE* filter(FP_TYPE* b, int nb, FP_TYPE* a, int na,
  FP_TYPE* x, int nx) {
  return cig_filter(b, nb, a, na, x, nx);
}

static inline FP_TYPE* filtfilt(FP_TYPE* b, int nb, FP_TYPE* a, int na,
  FP_TYPE* x, int nx) {
  FP_TYPE* y = cig_filter(b, nb, a, na, x, nx);
  FP_TYPE* z = malloc(nx * sizeof(FP_TYPE));
  for(int i = 0; i < nx; i ++) z[i] = y[nx - i - 1]; // flip the signal
  FP_TYPE* y2 = cig_filter(b, nb, a, na, z, nx); // fliter again
  for(int i = 0; i < nx; i ++) y[i] = y2[nx - i - 1]; // flip back the signal
  free(z);
  free(y2);
  return y;
}

FP_TYPE* cig_interp(FP_TYPE* xi, FP_TYPE* yi, int ni, FP_TYPE* x, int nx);

static inline FP_TYPE* interp1(FP_TYPE* xi, FP_TYPE* yi, int ni, FP_TYPE* x, int nx) {
  return cig_interp(xi, yi, ni, x, nx);
}

FP_TYPE* cig_interpu(FP_TYPE xi0, FP_TYPE xi1, FP_TYPE* yi, int ni, FP_TYPE* x, int nx);

static inline FP_TYPE* interp1u(FP_TYPE xi0, FP_TYPE xi1, FP_TYPE* yi, int ni,
  FP_TYPE* x, int nx) {
  return cig_interpu(xi0, xi1, yi, ni, x, nx);
}

FP_TYPE* cig_sincinterpu(FP_TYPE xi0, FP_TYPE xi1, FP_TYPE* yi, int ni, FP_TYPE* x, int nx);
static inline FP_TYPE* sincinterp1u(FP_TYPE xi0, FP_TYPE xi1, FP_TYPE* yi, int ni,
  FP_TYPE* x, int nx) {
  return cig_sincinterpu(xi0, xi1, yi, ni, x, nx);
}

FP_TYPE* cig_medfilt(FP_TYPE* x, int nx, int order);

static inline FP_TYPE* medfilt1(FP_TYPE* x, int nx, int order) {
  return cig_medfilt(x, nx, order);
}

static inline FP_TYPE* white_noise(FP_TYPE amplitude, int n) {
  FP_TYPE* y = malloc(n * sizeof(FP_TYPE));
  for(int i = 0; i < n; i ++)
    y[i] = ((FP_TYPE)rand() / RAND_MAX - 0.5) * amplitude * 2.0;
  return y;
}

static inline FP_TYPE* moving_avg(FP_TYPE* x, int nx, FP_TYPE halford) {
  FP_TYPE* acc = malloc(nx * sizeof(FP_TYPE));

  acc[0] = x[0];
  for(int i = 1; i < nx; i ++) acc[i] = acc[i - 1] + x[i];

  FP_TYPE* interp_idx = malloc(nx * sizeof(FP_TYPE));
  for(int i = 0; i < nx; i ++) interp_idx[i] = i + halford;
  FP_TYPE* interp_upper = interp1u(0, nx, acc, nx, interp_idx, nx);
  for(int i = 0; i < nx; i ++) interp_idx[i] = i - halford;
  FP_TYPE* interp_lower = interp1u(0, nx, acc, nx, interp_idx, nx);

  for(int i = 0; i < nx; i ++) {
    interp_upper[i] = (interp_upper[i] - interp_lower[i]) / halford * 0.5;
  }
  
  free(acc);
  free(interp_idx);
  free(interp_lower);
  
  return interp_upper;
}

static inline FP_TYPE* moving_rms(FP_TYPE* x, int nx, int order) {
  FP_TYPE* xsqr = malloc(nx * sizeof(FP_TYPE));
  for(int i = 0; i < nx; i ++)
    xsqr[i] = x[i] * x[i];
  FP_TYPE* y = moving_avg(xsqr, nx, order);
  for(int i = 0; i < nx; i ++)
    y[i] = sqrt(y[i]);
  free(xsqr);
  return y;
}

static inline FP_TYPE itakura_saito(FP_TYPE* S, FP_TYPE* S0, int nS) {
  FP_TYPE* d = malloc(nS * sizeof(FP_TYPE));
  for(int i = 0; i < nS; i ++)
    d[i] = S[i] / S0[i] - log_2(S[i] / S0[i]) - 1.0;
  FP_TYPE ret = log_2(sumfp(d, nS) / nS);
  free(d);
  return ret;
}

// DTFT sinc function
static inline FP_TYPE safe_aliased_sinc(FP_TYPE M, FP_TYPE omega) {
  FP_TYPE denom = sin_3(omega * 0.5);
  if(fabs(denom) < 10e-5) return M;
  return sin_3(M * omega * 0.5) / denom;
}

// derivative of DTFT sinc function
static inline FP_TYPE safe_aliased_dsinc(FP_TYPE M, FP_TYPE omega) {
  FP_TYPE cosn = cos_3(M * omega / 2);
  FP_TYPE cosw = cos_3(omega / 2);
  FP_TYPE sinn = sin_3(M * omega / 2);
  FP_TYPE sinw = sin_3(omega / 2);
  if(fabs(sinw) < 1e-5) return 0;
  return 0.5 * (M * cosn - cosw / sinw * sinn) / sinw;
}

FP_TYPE* cig_rresample(FP_TYPE* x, int nx, FP_TYPE ratio, int* ny);

static inline FP_TYPE* rresample(FP_TYPE* x, int nx, FP_TYPE ratio, int* ny) {
  return cig_rresample(x, nx, ratio, ny);
}

// function pointer of one-to-one float point operation with environment variable
typedef FP_TYPE (*fpe_one_to_one)(FP_TYPE x, void* env);

FP_TYPE cig_fzero(fpe_one_to_one func, FP_TYPE xmin, FP_TYPE xmax, void* env);

static inline FP_TYPE fzero(fpe_one_to_one func, FP_TYPE xmin, FP_TYPE xmax, void* env) {
  return cig_fzero(func, xmin, xmax, env);
}

// Audio/Speech Processing Functions

typedef struct {
  int nchannel;     // number of channels/bands
  int nf;           // size of frequency response
  FP_TYPE fnyq;     // upperbound of frequency response
  FP_TYPE** fresp;  // array of frequency response of each channel

  int* lower_idx;   // index where the freq. resp. in each channel rises from 0
  int* upper_idx;   // index where the freq. resp. in each channel fades to 0
} filterbank;

static inline FP_TYPE mel2freq(FP_TYPE mel) {
  return 700.0 * (exp_2(mel / 1125.0) - 1.0);
}

static inline FP_TYPE freq2mel(FP_TYPE f) {
  return 1125.0 * log_2(1.0 + f / 700.0);
}

static inline FP_TYPE freq2bark(FP_TYPE f) {
  return 6.0 * asinh(f / 600.0);
}

static inline FP_TYPE bark2freq(FP_TYPE z) {
  return 600.0 * sinh(z / 6.0);
}

static inline FP_TYPE eqloud(FP_TYPE f) {
  FP_TYPE f2 = f * f;
  FP_TYPE f4 = f2 * f2;
  return f4 / (f2 + 1.6e5) / (f2 + 1.6e5) * (f2 + 1.44e6) / (f2 + 9.61e6);
}

static inline FP_TYPE* melspace(FP_TYPE fmin, FP_TYPE fmax, int n) {
  FP_TYPE* freq = malloc((n + 1) * sizeof(FP_TYPE));
  FP_TYPE mmin = freq2mel(fmin);
  FP_TYPE mmax = freq2mel(fmax);
  for(int i = 0; i <= n; i ++)
    freq[i] = mel2freq((FP_TYPE)i / n * (mmax - mmin) + mmin);
  return freq;
}

void cig_stft_forward(FP_TYPE* x, int nx, int* center, int* nwin, int nfrm,
  int nfft, char* window, int subt_mean,
  FP_TYPE* norm_factor, FP_TYPE* weight_factor, FP_TYPE** Xmagn, FP_TYPE** Xphse);

FP_TYPE* cig_stft_backward(FP_TYPE** Xmagn, FP_TYPE** Xphse, int nhop, int nfrm,
  int offset, int hop_factor, int zp_factor, int nfade, FP_TYPE norm_factor, int* ny);

static inline void stft(FP_TYPE* x, int nx, int nhop, int nfrm, int hopfc, int zpfc,
  FP_TYPE* normfc, FP_TYPE* weightfc, FP_TYPE** Xmagn, FP_TYPE** Xphse) {
  int* center = malloc(nfrm * sizeof(int));
  int* nwin = malloc(nfrm * sizeof(int));
  for(int i = 0; i < nfrm; i ++) {
    center[i] = nhop * i;
    nwin[i] = nhop * hopfc;
  }
  cig_stft_forward(x, nx, center, nwin, nfrm, nhop * hopfc * zpfc, "blackman", 0,
    normfc, weightfc, Xmagn, Xphse);
  free(center);
  free(nwin);
}

static inline FP_TYPE* istft(FP_TYPE** Xmagn, FP_TYPE** Xphse, int nhop, int nfrm,
  int hopfc, int zpfc, FP_TYPE normfc, int* ny) {
  return cig_stft_backward(Xmagn, Xphse, nhop, nfrm, 0, hopfc, zpfc, 32, normfc, ny);
}

FP_TYPE cig_qifft(FP_TYPE* magn, int k, FP_TYPE* dst_freq);

static inline FP_TYPE qifft(FP_TYPE* magn, int k, FP_TYPE* dst_freq) {
  return cig_qifft(magn, k, dst_freq);
}

static inline FP_TYPE** spgm2cegm(FP_TYPE** S, int nfrm, int nfft, int ncep) {
  FP_TYPE** C = (FP_TYPE**)malloc2d(nfrm, ncep, sizeof(FP_TYPE));
  FP_TYPE* xbuff = calloc(nfft, sizeof(FP_TYPE));
  FP_TYPE* cbuff = calloc(nfft, sizeof(FP_TYPE));
  FP_TYPE* fftbuff = calloc(nfft * 2, sizeof(FP_TYPE));
  
  for(int i = 0; i < nfrm; i ++) {
    for(int j = 0; j < nfft / 2 + 1; j ++)
      xbuff[j] = log_2(S[i][j] + M_EPS);
    complete_symm(xbuff, nfft);
    ifft(xbuff, NULL, cbuff, NULL, nfft, fftbuff);
    for(int j = 0; j < ncep; j ++) {
      C[i][j] = cbuff[j];
    }
  }
  
  free(fftbuff);
  free(cbuff);
  free(xbuff);
  return C;
}

static inline FP_TYPE** cegm2spgm(FP_TYPE** C, int nfrm, int nfft, int ncep) {
  FP_TYPE** S = (FP_TYPE**)malloc2d(nfrm, nfft / 2 + 1, sizeof(FP_TYPE));
  FP_TYPE* xbuff = calloc(nfft, sizeof(FP_TYPE));
  FP_TYPE* cbuff = calloc(nfft, sizeof(FP_TYPE));
  FP_TYPE* fftbuff = calloc(nfft * 2, sizeof(FP_TYPE));
  
  for(int i = 0; i < nfrm; i ++) {
    for(int j = 0; j < ncep; j ++)
      cbuff[j] = C[i][j];
    for(int j = ncep; j < nfft / 2 + 1; j ++)
      cbuff[j] = 0;
    complete_symm(cbuff, nfft);
    fft(cbuff, NULL, xbuff, NULL, nfft, fftbuff);
    for(int j = 0; j < nfft / 2 + 1; j ++)
      S[i][j] = exp_2(xbuff[j]);
  }
  
  free(fftbuff);
  free(cbuff);
  free(xbuff);
  return S;
}

filterbank* cig_create_empty_filterbank(int nf, FP_TYPE fnyq, int nchannel);
filterbank* cig_create_plp_filterbank(int nf, FP_TYPE fnyq, int nchannel);
filterbank* cig_create_melfreq_filterbank(int nf, FP_TYPE fnyq, int nchannel,
  FP_TYPE min_freq, FP_TYPE max_freq, FP_TYPE scale);
void cig_delete_filterbank(filterbank* dst);

static inline filterbank* create_filterbank(int nf, FP_TYPE fnyq, int nchannel) {
  return cig_create_empty_filterbank(nf, fnyq, nchannel);
}

static inline filterbank* create_plpfilterbank(int nf, FP_TYPE fnyq, int nchannel) {
  return cig_create_plp_filterbank(nf, fnyq, nchannel);
}

static inline filterbank* create_melfilterbank(int nf, FP_TYPE fnyq, int nchannel,
  FP_TYPE min_freq, FP_TYPE max_freq) {
  return cig_create_melfreq_filterbank(nf, fnyq, nchannel, min_freq, max_freq, 1.0);
}

static inline void delete_filterbank(filterbank* dst) {
  cig_delete_filterbank(dst);
}

FP_TYPE** cig_filterbank_spectrogram(filterbank* fbank, FP_TYPE** S, int nfrm,
  int nfft, int fs, int crtenergy);

static inline FP_TYPE** filterbank_spgm(filterbank* fbank, FP_TYPE** S, int nfrm,
  int nfft, int fs, int crtenergy) {
  return cig_filterbank_spectrogram(fbank, S, nfrm, nfft, fs, crtenergy);
}

FP_TYPE* cig_filterbank_spectrum(filterbank* fbank, FP_TYPE* S, int nfft, int fs,
  int crtenergy);

static inline FP_TYPE* filterbank_spec(filterbank* fbank, FP_TYPE* S, int nfft,
  int fs, int crtenergy) {
  return cig_filterbank_spectrum(fbank, S, nfft, fs, crtenergy);
}

static inline FP_TYPE* be2cc(FP_TYPE* band_energy, int nbe, int ncc, int with_energy) {
  FP_TYPE* dctcoef = dct(band_energy, nbe);
  FP_TYPE energy = dctcoef[0];
  ncc = min(nbe - 1, ncc);
  for(int i = 0; i < ncc; i ++)
    dctcoef[i] = dctcoef[i + 1];
  if(with_energy)
    dctcoef[ncc] = energy;
  return dctcoef;
}

static inline FP_TYPE** be2ccgm(FP_TYPE** E, int nfrm, int nbe, int ncc, int with_energy) {
  FP_TYPE** C = malloc(nfrm * sizeof(FP_TYPE*));
  for(int i = 0; i < nfrm; i ++)
    C[i] = be2cc(E[i], nbe, ncc, with_energy);
  return C;
}

FP_TYPE* cig_spec2env(FP_TYPE* S, int nfft, int fs, FP_TYPE f0, FP_TYPE* Cout);

static inline FP_TYPE* spec2env(FP_TYPE* S, int nfft, int fs, FP_TYPE f0, FP_TYPE* Cout) {
  return cig_spec2env(S, nfft, fs, f0, Cout);
}

typedef struct {
  FP_TYPE T0;
  FP_TYPE te;
  FP_TYPE tp;
  FP_TYPE ta;
  FP_TYPE Ee;
} lfmodel;

lfmodel cig_lfmodel_from_rd(FP_TYPE rd, FP_TYPE T0, FP_TYPE Ee);

static inline lfmodel lfmodel_from_rd(FP_TYPE rd, FP_TYPE T0, FP_TYPE Ee) {
  return cig_lfmodel_from_rd(rd, T0, Ee);
};

FP_TYPE* cig_lfmodel_spectrum(lfmodel model, FP_TYPE* freq, int nf, FP_TYPE* dst_phase);

static inline FP_TYPE* lfmodel_spectrum(lfmodel model, FP_TYPE* freq, int nf, FP_TYPE* dst_phase) {
  return cig_lfmodel_spectrum(model, freq, nf, dst_phase);
}

FP_TYPE* cig_lfmodel_period(lfmodel model, int fs, int n);

static inline FP_TYPE* lfmodel_period(lfmodel model, int fs, int n) {
  return cig_lfmodel_period(model, fs, n);
}

// Plotting Functions (Gnuplot Interface)
// Notice: not supported on Windows
#if _POSIX_C_SOURCE >= 2

typedef struct {
  FILE* handle;
} figure;

static inline figure* plotopen() {
  figure* fg = malloc(sizeof(figure));
  fg -> handle = popen("gnuplot -p", "w");
  return fg;
}

static inline void plot(figure* fg, FP_TYPE* x, FP_TYPE* y, int nx, char ccolor) {
  char* color = "blue";
  switch(ccolor) {
    case 'b':
      color = "blue";
      break;
    case 'r':
      color = "red";
      break;
    case 'g':
      color = "green";
      break;
    case 'k':
      color = "black";
      break;
    case 'c':
      color = "cyan";
      break;
    case 'y':
      color = "yellow";
      break;
  }
  fprintf(fg -> handle, "plot '-' with lines lc rgb \"%s\"\n", color);
  for(int i = 0; i < nx; i ++) {
    fprintf(fg -> handle, "%f %f\n", x == NULL ? (FP_TYPE)i : x[i], y[i]);
  }
  fprintf(fg -> handle, "e\n");
}

static inline void imagesc(figure* fg, FP_TYPE** X, int m, int n) {
  fprintf(fg -> handle, "set xrange [%f:%f]\n", 0.0, n - 1.0);
  fprintf(fg -> handle, "set yrange [%f:%f]\n", 0.0, m - 1.0);
  fprintf(fg -> handle, "set view map\n");
  fprintf(fg -> handle, "splot '-' matrix with image\n");
  for(int i = 0; i < m; i ++) {
    for(int j = 0; j < n; j ++)
      fprintf(fg -> handle, "%5.3e ", X[i][j]);
    fprintf(fg -> handle, "\n");
  }
    fprintf(fg -> handle, "e\n");
}

static inline void plotclose(figure* fg) {
  pclose(fg -> handle);
  free(fg);
}

#endif

#endif
