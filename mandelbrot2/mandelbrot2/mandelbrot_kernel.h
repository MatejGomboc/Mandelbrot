const char* mandelbrot_ocl_source = 
"#define M_2_PI 6.283185307179586476925286766559f\n"
"#define M_PI_2 1.5707963267948966192313216916398f\n"
"#define M_PI 3.1415926535897932384626433832795f\n"
"kernel void mandelbrot (\n"
"        write_only image2d_t output_image,\n"
"        const float x_min,\n"
"        const float x_max,\n"
"        const float y_min,\n"
"        const float y_max,\n"
"        const uint max_iterations\n"
"    )\n"
"{\n"
"    write_imagef(output_image, (int2)(get_global_id(0), get_global_id(1)), (float4)(0.5, 0.5, 0.5, 1.0));\n"
"}\n"
"\0"
;