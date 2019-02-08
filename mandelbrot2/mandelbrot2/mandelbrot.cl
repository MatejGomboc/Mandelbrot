/* Copyright (C) 2019 Matej Gomboc https://github.com/MatejGomboc

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see http://www.gnu.org/licenses/. */


#define M_2_PI 6.283185307179586476925286766559f
#define M_PI_2 1.5707963267948966192313216916398f
#define M_PI 3.1415926535897932384626433832795f


kernel void mandelbrot (
		write_only image2d_t output_image,
		const float x_min,
		const float x_max,
		const float y_min,
		const float y_max,
		const uint max_iterations
	)
{
	/*float x0 = ((float)get_global_id(0) / (float)get_global_size(0)) * (x_max - x_min) + x_min;
	float y0 = ((float)get_global_id(1) / (float)get_global_size(1)) * (y_max - y_min) + y_min;*/

	/*float x = 0.0f;
	float y = 0.0f;
	uint iteration = 0f;
	float temp = 0.0f;*/

	/*while ((x * x + y * y < 2.0f * 2.0f) && (iteration < max_iterations))
	{
		temp = x * x - y * y + x0;
		y = 2.0f * x * y + y0;
		x = temp;
		iteration = iteration + 1;
	}*/
	
	/*float quotient = (float)iteration / (float)max_iterations;*/

	/*write_imagef(output_image, (int2)(get_global_id(0), get_global_id(1)), (float4)(
		0.5 * sin(M_2_PI * quotient - M_PI) + 0.5,
		0.5 * sin(M_2_PI * quotient - M_PI_2) + 0.5,
		0.5 * sin(M_2_PI * quotient) + 0.5,
		1.0
	));*/

	write_imagef(output_image, (int2)(get_global_id(0), get_global_id(1)), (float4)(0.5, 0.5, 0.5, 1.0));
}
