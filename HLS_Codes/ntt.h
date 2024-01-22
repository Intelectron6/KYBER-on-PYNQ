#include <ap_int.h>
#include <stdio.h>

#include "hls_stream.h"

typedef ap_uint<1> bit;
typedef ap_uint<8> ap_logn_t;
typedef ap_int<16> coeff_t;
typedef ap_int<32> double_coeff_t;

struct coeff_t_stream
{
   coeff_t value;
   bit last;
};

struct coeff_t_stream_big
{
   double_coeff_t value;
   bit last;
};

#define N 128
#define Nt 256
#define logN 7
extern coeff_t q, w_n;

int poly_mult (hls::stream<coeff_t_stream_big> &input, hls::stream<coeff_t_stream> &output);
