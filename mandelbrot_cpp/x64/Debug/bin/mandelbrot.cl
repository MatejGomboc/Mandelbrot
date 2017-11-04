kernel void mandelbrot(write_only image2d_t picture)
{
	write_imagef(picture, (int2)(get_global_id(0), get_global_id(1)), (float4)(1.0f, 0.8f, 0.0f, 1.0f));
}
