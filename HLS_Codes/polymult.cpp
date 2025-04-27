#include "ntt.h"

coeff_t q = 3329;
coeff_t inv_n = 3303;
//double_coeff_t v = 20159;

/*coeff_t mod(double_coeff_t A)
{
	#pragma HLS inline OFF
	//double_coeff_t v = (double_coeff_t) ((1<<26) + 1664)/q;
	double_coeff_t t = (v * A + (1 << 25)) >> 26;
	t = t * q;
	coeff_t val;
	if (A < t)
		val = A - t + q;
	else
		val = A - t;
	return val;
}*/

ap_uint<13> m = 5039;

coeff_t mod(double_coeff_t A)
{
	#pragma HLS pipeline II = 1
	coeff_t val;
	ap_uint<36> t123 = m * A;
	ap_uint<12> t = (t123 >> 24);
	ap_uint<24> ta = t * q;
	ap_uint<24> c = A - ta;
	if (c > q)
		val = (coeff_t) (c - q);
	else
		val = (coeff_t) c;
	return val;
}

coeff_t modadd(coeff_t x, coeff_t y)
{
	#pragma HLS inline
	coeff_t w = x + y;
	return (coeff_t)(w - (w < q ? (coeff_t)0 : q));
}

coeff_t modsub(coeff_t x, coeff_t y)
{
	#pragma HLS inline
	coeff_t s = x + (x > y ? (coeff_t)0 : q);
	return (coeff_t)(s - y);
}

void butterfly_unit_dif(coeff_t w, coeff_t a, coeff_t b, coeff_t &x, coeff_t &y)
{
	#pragma HLS pipeline II = 1
	x = modadd(a, b);
	y = modsub(a, b);
	y = mod(w * y);
}

void butterfly_unit_dit(coeff_t w, coeff_t a, coeff_t b, coeff_t &x, coeff_t &y)
{
	#pragma HLS pipeline II = 1
	coeff_t wb = mod(w * b);
	x = modadd(a, wb);
	y = modsub(a, wb);
}

void delay_cycle()
{
	#ifdef __SYNTHESIS__
		ap_wait_n(1);
	#endif
}

void ntt_stage1 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b, coeff_t fifo[])
{
	#pragma HLS dataflow
	coeff_t twiddle_coeff = 1729;

	#pragma HLS DEPENDENCE variable = fifo inter RAW false

	int x, y;
	coeff_t a_, b_, it, bf1, bf2, tf;

	for (int i = 0; i < 64; i++)
	{
		#pragma HLS pipeline
		it = a.read();
		fifo[i + 64] = it;
	}

	for (int j = 0; j < 1; j++)
	{
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		int iter = 0;
		for (int k = 0; k < 64; k++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			a_ = fifo[iter + 64];
			b_ = a.read();
			tf = twiddle_coeff;
			butterfly_unit_dit(tf, a_, b_, bf1, bf2);
			b.write(bf1);
			fifo[iter] = bf2;
			iter++;
			delay_cycle();
		}

		for (int i = 0; i < 64; i++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			b.write(fifo[i]);
			delay_cycle();
		}
	}
}

void ntt_stage2 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b, coeff_t fifo[])
{
	#pragma HLS dataflow
	coeff_t twiddle_coeffs[2] = {2580, 3289};

	#pragma HLS DEPENDENCE variable = fifo inter RAW false

	int x, y;
	coeff_t a_, b_, it, bf1, bf2, tf;

	for (int i = 0; i < 32; i++)
	{
		#pragma HLS pipeline
		it = a.read();
		fifo[i + 64] = it;
	}

	for (int j = 0; j < 2; j++)
	{
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		int iter = 0;
		for (int k = 0; k < 32; k++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			a_ = fifo[iter + 64];
			b_ = a.read();
			tf = twiddle_coeffs[j];
			butterfly_unit_dit(tf, a_, b_, bf1, bf2);
			b.write(bf1);
			fifo[iter] = bf2;
			iter++;
			delay_cycle();
		}

		for (int i = 0; i < 32; i++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			b.write(fifo[i]);
			delay_cycle();
			if (j < 1)
			{
				it = a.read();
				fifo[i + 64] = it;
			}
		}
	}
}

void ntt_stage3 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b, coeff_t fifo[])
{
	#pragma HLS dataflow
	coeff_t twiddle_coeffs[4] = {2642, 630, 1897, 848};

	#pragma HLS DEPENDENCE variable = fifo inter RAW false

	int x, y;
	coeff_t a_, b_, it, bf1, bf2, tf;

	for (int i = 0; i < 16; i++)
	{
		#pragma HLS pipeline
		it = a.read();
		fifo[i + 64] = it;
	}

	for (int j = 0; j < 4; j++)
	{
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		int iter = 0;
		for (int k = 0; k < 16; k++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			a_ = fifo[iter + 64];
			b_ = a.read();
			tf = twiddle_coeffs[j];
			butterfly_unit_dit(tf, a_, b_, bf1, bf2);
			b.write(bf1);
			fifo[iter] = bf2;
			iter++;
			delay_cycle();
		}

		for (int i = 0; i < 16; i++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			b.write(fifo[i]);
			delay_cycle();
			if (j < 3)
			{
				it = a.read();
				fifo[i + 64] = it;
			}
		}
	}
}

void ntt_stage4 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b, coeff_t fifo[])
{
	#pragma HLS dataflow
	coeff_t twiddle_coeffs[8] = {1062, 1919, 193, 797, 2786, 3260, 569, 1746};

	#pragma HLS DEPENDENCE variable = fifo inter RAW false

	int x, y;
	coeff_t a_, b_, it, bf1, bf2, tf;

	for (int i = 0; i < 8; i++)
	{
		#pragma HLS pipeline
		it = a.read();
		fifo[i + 64] = it;
	}

	for (int j = 0; j < 8; j++)
	{
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		int iter = 0;
		int ind = 1;
		for (int k = 0; k < 8; k++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			a_ = fifo[iter + 64];
			b_ = a.read();
			tf = twiddle_coeffs[j];
			butterfly_unit_dit(tf, a_, b_, bf1, bf2);
			b.write(bf1);
			fifo[iter] = bf2;
			iter++;
			delay_cycle();
		}

		for (int i = 0; i < 8; i++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			b.write(fifo[i]);
			delay_cycle();
			if (j < 7)
			{
				it = a.read();
				fifo[i + 64] = it;
			}
		}
	}
}

void ntt_stage5 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b, coeff_t fifo[])
{
	#pragma HLS dataflow
	coeff_t twiddle_coeffs[16] = {296, 2447, 1339, 1476, 3046, 56, 2240, 1333,
								  1426, 2094, 535, 2882, 2393, 2879, 1974, 821};

	#pragma HLS DEPENDENCE variable = fifo inter RAW false

	int x, y;
	coeff_t a_, b_, it, bf1, bf2, tf;

	for (int i = 0; i < 4; i++)
	{
		#pragma HLS pipeline
		it = a.read();
		fifo[i + 64] = it;
	}

	for (int j = 0; j < 16; j++)
	{
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		int iter = 0;
		int ind = 1;
		for (int k = 0; k < 4; k = k + 1)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			a_ = fifo[iter + 64];
			b_ = a.read();
			tf = twiddle_coeffs[j];
			butterfly_unit_dit(tf, a_, b_, bf1, bf2);
			b.write(bf1);
			fifo[iter] = bf2;
			iter++;
			delay_cycle();
		}

		for (int i = 0; i < 4; i++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			b.write(fifo[i]);
			delay_cycle();
			if (j < 15)
			{
				it = a.read();
				fifo[i + 64] = it;
			}
		}
	}
}

void ntt_stage6 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b, coeff_t fifo[])
{
	#pragma HLS dataflow
	coeff_t twiddle_coeffs[32] = {289, 331, 3253, 1756, 1197, 2304, 2277, 2055,
								  650, 1977, 2513, 632, 2865, 33, 1320, 1915,
								  2319, 1435, 807, 452, 1438, 2868, 1534, 2402,
								  2647, 2617, 1481, 648, 2474, 3110, 1227, 910};

	#pragma HLS DEPENDENCE variable = fifo inter RAW false

	int x, y;
	coeff_t a_, b_, it, bf1, bf2, tf;

	for (int i = 0; i < 2; i++)
	{
		#pragma HLS pipeline
		it = a.read();
		fifo[i + 64] = it;
	}

	for (int j = 0; j < 32; j++)
	{
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		int iter = 0;
		int ind = 1;
		for (int k = 0; k < 2; k = k + 1)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			a_ = fifo[iter + 64];
			b_ = a.read();
			tf = twiddle_coeffs[j];
			butterfly_unit_dit(tf, a_, b_, bf1, bf2);
			b.write(bf1);
			fifo[iter] = bf2;
			iter++;
			delay_cycle();
		}

		for (int i = 0; i < 2; i++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			b.write(fifo[i]);
			delay_cycle();
			if (j < 31)
			{
				it = a.read();
				fifo[i + 64] = it;
			}
		}
	}
}


void ntt_stage7 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b,  coeff_t fifo[])
{
	#pragma HLS inline off

	#pragma HLS DEPENDENCE variable = fifo inter RAW false
	coeff_t twiddle_coeffs[64] = {17, 2761, 583, 2649, 1637, 723, 2288, 1100,
								  1409, 2662, 3281, 233, 756, 2156, 3015, 3050,
								  1703, 1651, 2789, 1789, 1847, 952, 1461, 2687,
								  939, 2308, 2437, 2388, 733, 2337, 268, 641,
							 	  1584, 2298, 2037, 3220, 375, 2549, 2090, 1645,
								  1063, 319, 2773, 757, 2099, 561, 2466, 2594,
								  2804, 1092, 403, 1026, 1143, 2150, 2775, 886,
								  1722, 1212, 1874, 1029, 2110, 2935, 885, 2154};
	int x, y;
	coeff_t u, t, it, bf1, bf2;

	u = a.read();

	for (int j = 0; j < 64; j++)
	{
		#pragma HLS pipeline II = 1
		#pragma HLS DEPENDENCE variable = fifo inter RAW false

		t = a.read();
		butterfly_unit_dit(twiddle_coeffs[j], u, t, bf1, bf2);
		b.write(bf1);
		b.write(bf2);
		if (j < 63)
			u = a.read();
	}
}

void intt_stage1 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b, coeff_t fifo[])
{
	coeff_t twiddle_coeffs[64] = {1175, 2444, 394, 1219, 2300, 1455, 2117, 1607,
								  2443,  554, 1179, 2186, 2303, 2926, 2237,  525,
								  735,  863, 2768, 1230, 2572,  556, 3010, 2266,
								  1684, 1239,  780, 2954,  109, 1292, 1031, 1745,
								  2688, 3061,  992, 2596,  941,  892, 1021, 2390,
								  642, 1868, 2377, 1482, 1540,  540, 1678, 1626,
								  279,  314, 1173, 2573, 3096,   48,  667, 1920,
								  2229, 1041, 2606, 1692,  680, 2746,  568, 3312};

	#pragma HLS inline off
	#pragma HLS DEPENDENCE variable = fifo inter RAW false
	int x, y;
	coeff_t u, t, it, bf1, bf2;

	u = a.read();

	for (int j = 0; j < 64; j++)
	{
		#pragma HLS pipeline II = 1
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		t = a.read();
		butterfly_unit_dif(twiddle_coeffs[j], u, t, bf1, bf2);
		b.write(bf1);
		b.write(bf2);
		if (j < 63)
			u = a.read();
	}
}

void intt_stage2 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b, coeff_t fifo[])
{
	#pragma HLS dataflow
	coeff_t twiddle_coeffs[32] = {2419, 2102,  219,  855, 2681, 1848,  712,  682,
								  927, 1795,  461, 1891, 2877, 2522, 1894, 1010,
								  1414, 2009, 3296,  464, 2697,  816, 1352, 2679,
								  1274, 1052, 1025, 2132, 1573,   76, 2998, 3040};

	#pragma HLS DEPENDENCE variable = fifo inter RAW false

	int x, y;
	coeff_t a_, b_, it, bf1, bf2, tf;

	for (int i = 0; i < 2; i++)
	{
		#pragma HLS pipeline
		it = a.read();
		fifo[i + 64] = it;
	}

	int ind = 0;
	int count = 0;
	for (int j = 0; j < 32; j++)
	{
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		int iter = 0;
		for (int k = 0; k < 2; k = k + 1)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			a_ = fifo[iter + 64];
			b_ = a.read();
			tf = twiddle_coeffs[ind];
			butterfly_unit_dif(tf, a_, b_, bf1, bf2);
			b.write(bf1);
			fifo[iter] = bf2;
			iter++;
			count++;
			if (count % 2 == 0)
				ind++;
			delay_cycle();
		}

		for (int i = 0; i < 2; i++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			b.write(fifo[i]);
			delay_cycle();
			if (j < 31)
			{
				it = a.read();
				fifo[i + 64] = it;
			}
		}
	}
}

void intt_stage3 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b, coeff_t fifo[])
{
	#pragma HLS dataflow
	coeff_t twiddle_coeffs[16] = {2508, 1355,  450,  936,  447, 2794, 1235, 1903,
								  1996, 1089, 3273,  283, 1853, 1990,  882, 3033};

	#pragma HLS DEPENDENCE variable = fifo inter RAW false

	int x, y;
	int m = 4;
	coeff_t a_, b_, it, bf1, bf2, tf;

	for (int i = 0; i < 4; i++)
	{
		#pragma HLS pipeline
		it = a.read();
		fifo[i + 64] = it;
	}

	int ind = 0;
	int count = 0;
	for (int j = 0; j < 16; j++)
	{
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		int iter = 0;
		for (int k = 0; k < 4; k = k + 1)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			a_ = fifo[iter + 64];
			b_ = a.read();
			tf = twiddle_coeffs[ind];
			butterfly_unit_dif(tf, a_, b_, bf1, bf2);
			b.write(bf1);
			fifo[iter] = bf2;
			iter++;
			count++;
			if (count % 4 == 0)
				ind++;
			delay_cycle();
		}

		for (int i = 0; i < 4; i++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			b.write(fifo[i]);
			delay_cycle();
			if (j < 15)
			{
				it = a.read();
				fifo[i + 64] = it;
			}
		}
	}
}

void intt_stage4 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b, coeff_t fifo[])
{
	#pragma HLS dataflow
	coeff_t twiddle_coeffs[8] = {1583, 2760, 69, 543, 2532, 3136, 1410, 2267};

	#pragma HLS DEPENDENCE variable = fifo inter RAW false

	int x, y;
	coeff_t a_, b_, it, bf1, bf2, tf;

	for (int i = 0; i < 8; i++)
	{
		#pragma HLS pipeline
		it = a.read();
		fifo[i + 64] = it;
	}

	int ind = 0;
	int count = 0;
	for (int j = 0; j < 8; j++)
	{
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		int iter = 0;
		for (int k = 0; k < 8; k = k + 1)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			a_ = fifo[iter + 64];
			b_ = a.read();
			tf = twiddle_coeffs[ind];
			butterfly_unit_dif(tf, a_, b_, bf1, bf2);
			b.write(bf1);
			fifo[iter] = bf2;
			iter++;
			count++;
			if (count % 8 == 0)
				ind++;
			delay_cycle();
		}

		for (int i = 0; i < 8; i++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			b.write(fifo[i]);
			delay_cycle();
			if (j < 7)
			{
				it = a.read();
				fifo[i + 64] = it;
			}
		}
	}
}

void intt_stage5 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b, coeff_t fifo[])
{
	#pragma HLS dataflow
	coeff_t twiddle_coeffs[4] = {2481, 1432, 2699,  687};

	#pragma HLS DEPENDENCE variable = fifo inter RAW false

	int x, y;
	coeff_t a_, b_, it, bf1, bf2, tf;

	for (int i = 0; i < 16; i++)
	{
		#pragma HLS pipeline
		it = a.read();
		fifo[i + 64] = it;
	}

	int ind = 0;
	int count = 0;
	for (int j = 0; j < 4; j++)
	{
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		int iter = 0;
		for (int k = 0; k < 16; k = k + 1)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			a_ = fifo[iter + 64];
			b_ = a.read();
			tf = twiddle_coeffs[ind];
			butterfly_unit_dif(tf, a_, b_, bf1, bf2);
			b.write(bf1);
			fifo[iter] = bf2;
			iter++;
			count++;
			if (count % 16 == 0)
				ind++;
			delay_cycle();
		}

		for (int i = 0; i < 16; i++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			b.write(fifo[i]);
			delay_cycle();
			if (j < 3)
			{
				it = a.read();
				fifo[i + 64] = it;
			}
		}
	}
}

void intt_stage6 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b, coeff_t fifo[])
{
	#pragma HLS dataflow
	coeff_t twiddle_coeffs[2] = {40,  749};

	#pragma HLS DEPENDENCE variable = fifo inter RAW false

	int x, y;
	coeff_t a_, b_, it, bf1, bf2, tf;

	for (int i = 0; i < 32; i++)
	{
		#pragma HLS pipeline
		it = a.read();
		fifo[i + 64] = it;
	}

	int ind = 0;
	int count = 0;
	for (int j = 0; j < 2; j++)
	{
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		int iter = 0;
		for (int k = 0; k < 32; k = k + 1)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			a_ = fifo[iter + 64];
			b_ = a.read();
			tf = twiddle_coeffs[ind];
			butterfly_unit_dif(tf, a_, b_, bf1, bf2);
			b.write(bf1);
			fifo[iter] = bf2;
			iter++;
			count++;
			if (count == 32)
				ind++;
			delay_cycle();
		}

		for (int i = 0; i < 32; i++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			b.write(fifo[i]);
			delay_cycle();
			if (j < 1)
			{
				it = a.read();
				fifo[i + 64] = it;
			}
		}
	}
}

void intt_stage7 (hls::stream<coeff_t> &a, hls::stream<coeff_t> &b,  coeff_t fifo[])
{
	#pragma HLS inline off
	#pragma HLS DEPENDENCE variable = fifo inter RAW false
	int x, y;
	coeff_t a_, b_, it, bf1, bf2, bfn1, bfn2, tf;

	for (int i = 0; i < 64; i++)
	{
		#pragma HLS pipeline
		it = a.read();
		fifo[i + 64] = it;
	}

	for (int j = 0; j < 1; j++)
	{
		#pragma HLS DEPENDENCE variable = fifo inter RAW false
		int iter = 0;
		for (int k = 0; k < 64; k = k + 1)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			a_ = fifo[iter + 64];
			b_ = a.read();
			tf = 1600;
			butterfly_unit_dif(tf, a_, b_, bf1, bf2);
			bfn1 = mod(bf1 * inv_n);
			bfn2 = mod(bf2 * inv_n);
			b.write(bfn1);
			fifo[iter] = bfn2;
			iter++;
			delay_cycle();
		}

		for (int i = 0; i < 64; i++)
		{
			#pragma HLS pipeline II = 1
			#pragma HLS DEPENDENCE variable = fifo inter RAW false
			b.write(fifo[i]);
			delay_cycle();
		}
	}
}

void read_inputs (hls::stream<coeff_t_stream> &input, hls::stream<coeff_t> &se, hls::stream<coeff_t> &so)
{
	coeff_t_stream x;
	coeff_t a;
	int i;

	for (i=0; i<Nt; i++)
	{
		#pragma HLS pipeline II = 1
		x = input.read();
		a = x.value;
		if (i%2 == 0)
			se.write(a);
		else
			so.write(a);
	}
}

void write_outputs (hls::stream<coeff_t> &se, hls::stream<coeff_t> &so, hls::stream<coeff_t_stream> &output)
{
	coeff_t a1, a0;
	coeff_t_stream y;
	int i;

	y.last = 0;
	for (i=0; i<N; i++)
	{
		#pragma HLS pipeline II = 1
		a0 = se.read();
		a1 = so.read();
		y.value = a0;
		output.write(y);
		y.value = a1;
		if (i == N-1)
			y.last = 1;
		output.write(y);
	}
}

void ct_ntt (hls::stream<coeff_t_stream> &input, hls::stream<coeff_t_stream> &output)
{
	#pragma HLS dataflow

	hls::stream<coeff_t> s0o("s0o"), s1o("s1o"), s2o("s2o"), s3o("s3o"),
						 s4o("s4o"), s5o("s5o"), s6o("s6o"), s7o("s7o"),
						 s0e("s0e"), s1e("s1e"), s2e("s2e"), s3e("s3e"),
						 s4e("s4e"), s5e("s5e"), s6e("s6e"), s7e("s7e");

	coeff_t fo7[65], fo6[66], fo5[68], fo4[72], fo3[80], fo2[96], fo1[128];
	coeff_t fe7[65], fe6[66], fe5[68], fe4[72], fe3[80], fe2[96], fe1[128];

	coeff_t_stream x, y;

	#pragma HLS STREAM variable = s7o depth = 1
	#pragma HLS STREAM variable = s6o depth = 2
	#pragma HLS STREAM variable = s5o depth = 4
	#pragma HLS STREAM variable = s4o depth = 8
	#pragma HLS STREAM variable = s3o depth = 16
	#pragma HLS STREAM variable = s2o depth = 32
	#pragma HLS STREAM variable = s1o depth = 64
	#pragma HLS STREAM variable = s0o depth = 128

	#pragma HLS STREAM variable = s7e depth = 1
	#pragma HLS STREAM variable = s6e depth = 2
	#pragma HLS STREAM variable = s5e depth = 4
	#pragma HLS STREAM variable = s4e depth = 8
	#pragma HLS STREAM variable = s3e depth = 16
	#pragma HLS STREAM variable = s2e depth = 32
	#pragma HLS STREAM variable = s1e depth = 64
	#pragma HLS STREAM variable = s0e depth = 128


	read_inputs(input, s0e, s0o);

	ntt_stage1 (s0e, s1e, fe1);
	ntt_stage1 (s0o, s1o, fo1);

	ntt_stage2 (s1e, s2e, fe2);
	ntt_stage2 (s1o, s2o, fo2);

	ntt_stage3 (s2e, s3e, fe3);
	ntt_stage3 (s2o, s3o, fo3);

	ntt_stage4 (s3e, s4e, fe4);
	ntt_stage4 (s3o, s4o, fo4);

	ntt_stage5 (s4e, s5e, fe5);
	ntt_stage5 (s4o, s5o, fo5);

	ntt_stage6 (s5e, s6e, fe6);
	ntt_stage6 (s5o, s6o, fo6);

	ntt_stage7 (s6e, s7e, fe7);
	ntt_stage7 (s6o, s7o, fo7);

	write_outputs(s7e, s7o, output);
}


void gs_intt (hls::stream<coeff_t_stream> &input, hls::stream<coeff_t_stream> &output)
{
	#pragma HLS dataflow

	hls::stream<coeff_t> s0o("s0o"), s1o("s1o"), s2o("s2o"), s3o("s3o"),
						 s4o("s4o"), s5o("s5o"), s6o("s6o"), s7o("s7o"),
						 s0e("s0e"), s1e("s1e"), s2e("s2e"), s3e("s3e"),
						 s4e("s4e"), s5e("s5e"), s6e("s6e"), s7e("s7e");

	coeff_t fo7[128], fo6[96], fo5[80], fo4[72], fo3[68], fo2[66], fo1[65];
	coeff_t fe7[128], fe6[96], fe5[80], fe4[72], fe3[68], fe2[66], fe1[65];

	coeff_t_stream x, y;

	#pragma HLS STREAM variable = s7o depth = 1
	#pragma HLS STREAM variable = s6o depth = 2
	#pragma HLS STREAM variable = s5o depth = 4
	#pragma HLS STREAM variable = s4o depth = 8
	#pragma HLS STREAM variable = s3o depth = 16
	#pragma HLS STREAM variable = s2o depth = 32
	#pragma HLS STREAM variable = s1o depth = 64
	#pragma HLS STREAM variable = s0o depth = 128

	#pragma HLS STREAM variable = s7e depth = 1
	#pragma HLS STREAM variable = s6e depth = 2
	#pragma HLS STREAM variable = s5e depth = 4
	#pragma HLS STREAM variable = s4e depth = 8
	#pragma HLS STREAM variable = s3e depth = 16
	#pragma HLS STREAM variable = s2e depth = 32
	#pragma HLS STREAM variable = s1e depth = 64
	#pragma HLS STREAM variable = s0e depth = 128

	read_inputs(input, s0e, s0o);

	intt_stage1 (s0e, s1e, fe1);
	intt_stage1 (s0o, s1o, fo1);

	intt_stage2 (s1e, s2e, fe2);
	intt_stage2 (s1o, s2o, fo2);

	intt_stage3 (s2e, s3e, fe3);
	intt_stage3 (s2o, s3o, fo3);

	intt_stage4 (s3e, s4e, fe4);
	intt_stage4 (s3o, s4o, fo4);

	intt_stage5 (s4e, s5e, fe5);
	intt_stage5 (s4o, s5o, fo5);

	intt_stage6 (s5e, s6e, fe6);
	intt_stage6 (s5o, s6o, fo6);

	intt_stage7 (s6e, s7e, fe7);
	intt_stage7 (s6o, s7o, fo7);

	write_outputs(s7e, s7o, output);
}

void stream_split (hls::stream<coeff_t_stream_big> &input,
				   hls::stream<coeff_t_stream> &input1,
				   hls::stream<coeff_t_stream> &input2)
{

	coeff_t_stream_big x;
	double_coeff_t a;
	coeff_t_stream x1, x2;
	coeff_t a1, a2;
	int i;

	for (i=0; i<Nt; i++)
	{
		#pragma HLS pipeline II = 1
		x = input.read();
		a = x.value;
		a1 = a(double_coeff_t::width - 1, coeff_t::width);
		a2 = a(coeff_t::width - 1, 0);
		if (i == Nt-1)
		{
			x1.last = 1;
			x2.last = 1;
		}
		else
		{
			x1.last = 0;
			x2.last = 0;
		}
		x1.value = a1;
		x2.value = a2;
		input1.write(x1);
		input2.write(x2);
	}
}

void point_wise_mult (hls::stream<coeff_t_stream> &input1,
					  hls::stream<coeff_t_stream> &input2,
		   	   	   	  hls::stream<coeff_t_stream> &output)
{
	coeff_t_stream xe, xo, ye, yo, z;
	coeff_t ae, be, ce, ao, bo, co, c1, c2, c2s, c3, c4;
	int i;

	coeff_t pm_factors[128] = {17, 3312, 2761, 568, 583, 2746, 2649, 680,
							   1637, 1692, 723, 2606, 2288, 1041, 1100, 2229,
							   1409, 1920, 2662, 667, 3281, 48, 233, 3096,
							   756, 2573, 2156, 1173, 3015, 314, 3050, 279,
							   1703, 1626, 1651, 1678, 2789, 540, 1789, 1540,
							   1847, 1482, 952, 2377, 1461, 1868, 2687, 642,
							   939, 2390, 2308, 1021, 2437, 892, 2388, 941,
							   733, 2596, 2337, 992, 268, 3061, 641, 2688,
							   1584, 1745, 2298, 1031, 2037, 1292, 3220, 109,
							   375, 2954, 2549, 780, 2090, 1239, 1645, 1684,
							   1063, 2266, 319, 3010, 2773, 556, 757, 2572,
							   2099, 1230, 561, 2768, 2466, 863, 2594, 735,
							   2804, 525, 1092, 2237, 403, 2926, 1026, 2303,
							   1143, 2186, 2150, 1179, 2775, 554, 886, 2443,
							   1722, 1607, 1212, 2117, 1874, 1455, 1029, 2300,
							   2110, 1219, 2935, 394, 885, 2444, 2154, 1175};

	z.last = 0;
	for (i=0; i<N; i++)
	{
		#pragma HLS pipeline II = 1
		xe = input1.read();
		xo = input1.read();
		ye = input2.read();
		yo = input2.read();
		ao = xo.value;
		bo = yo.value;
		ae = xe.value;
		be = ye.value;

		c1 = mod (ae * be);
		c2 = mod (ao * bo);
		c2s = mod (c2 * pm_factors[i]);
		c3 = mod (ae * bo);
		c4 = mod (ao * be);

		ce = modadd (c1, c2s);
		co = modadd (c3, c4);

		z.value = ce;
		output.write(z);
		if (i == N-1)
			z.last = 1;
		z.value = co;
		output.write(z);
	}
}

int poly_mult (hls::stream<coeff_t_stream_big> &input,
	       hls::stream<coeff_t_stream> &output)
{
	#pragma HLS dataflow
	#pragma HLS INTERFACE axis register port=input
	#pragma HLS INTERFACE axis register port=output
	#pragma HLS INTERFACE s_axilite port=return bundle=CTRL_BUS

	hls::stream<coeff_t_stream> input1, input2, middle1, middle2, middle3;

	stream_split(input, input1, input2);
	ct_ntt(input1, middle1);
	ct_ntt(input2, middle2);
	point_wise_mult(middle1, middle2, middle3);
	gs_intt(middle3, output);

	return 0;
}
