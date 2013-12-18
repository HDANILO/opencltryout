__kernel void copy(__global char* string_entrada, __global char* string_saida)
{
	const int idx = get_global_id(0);
	string_saida[idx] = string_entrada[idx];
}