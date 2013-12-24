__kernel void threshold(__read_only image2d_t img_input,__write_only image2d_t img_output,__constant float* thresh)
{
	int2 coords = (int2)(get_global_id(0), get_global_id(1));
	const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | //Natural coordinates
						  CLK_ADDRESS_CLAMP | //Clamp to zeros
						  CLK_FILTER_NEAREST; //Don't interpolate
	float4 p = read_imagef(img_input, smp, coords);
	
	if( p.w < *thresh)
		p.w = 0.0f;
	else
		p.w = 1.0f;

	write_imagef(img_output,coords,p);
}
