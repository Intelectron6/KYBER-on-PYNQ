#include "test_case.h"

int main()
{
	hls::stream<coeff_t_stream_big> in_data;
	hls::stream<coeff_t_stream> out_data;
	coeff_t_stream_big local_stream1;
	coeff_t_stream local_stream2;

	int i;

	coeff_t actual_outputs[Nt];

	//Writing randomly generated data to input stream
	for (i=0; i<Nt; i++)
	{
		coeff_t val1 = input1_vals[i];
		double_coeff_t val2 = (double_coeff_t) (input2_vals[i] * 65536);
		local_stream1.value = (double_coeff_t) (val1 + val2);
		if (i == Nt-1)
			local_stream1.last = 1;
		else
			local_stream1.last = 0;
		in_data.write(local_stream1);
	}

	//Calling hardware
	poly_mult (in_data, out_data);

	//Reading result from output stream
	for (i=0; i<Nt; i++)
	{
		local_stream2 = out_data.read();
		actual_outputs[i] = local_stream2.value;
	}

	//Comparing result with golden output
	int ret_val = 0;
	for (i=0; i<Nt; i++)
	{
		if (output_vals[i] != actual_outputs[i])
		{
			ret_val++;
			std::cout<<actual_outputs[i];
			break;
		}
	}

	return ret_val;
}
