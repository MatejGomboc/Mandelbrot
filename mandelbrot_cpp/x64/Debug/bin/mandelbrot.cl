kernel void mandelbrot (
		write_only image2d_t output_image,
		const float x_min,
		const float x_max,
		const float y_min,
		const float y_max,
		const uint max_iterations
	)
{
	float x0 = ((float)get_global_id(0) / (float)get_global_size(0)) * (x_max - x_min) + x_min;
	float y0 = ((float)get_global_id(1) / (float)get_global_size(1)) * (y_max - y_min) + y_min;

	float x = 0.0f;
	float y = 0.0f;
	uint iteration = 0;
	float temp = 0.0f;

	while ((x*x + y*y < 2.0f * 2.0f) && (iteration < max_iterations))
	{
		temp = x*x - y*y + x0;
		y = 2.0f * x*y + y0;
		x = temp;
		iteration = iteration + 1;
	}

	write_imagef(output_image, (int2)(get_global_id(0), get_global_id(1)), (float4)((float)iteration / (float)max_iterations));
}
