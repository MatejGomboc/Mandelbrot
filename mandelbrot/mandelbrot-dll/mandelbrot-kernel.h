const char* mandelbrot_opencl_source_string = 
"#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
"#define M_2_PI 6.283185307179586476925286766559\n"
"#define M_PI_2 1.5707963267948966192313216916398\n"
"#define M_PI 3.1415926535897932384626433832795\n"
"kernel void mandelbrot (\n"
"        write_only image2d_t output_image,\n"
"        const double x_min,\n"
"        const double x_max,\n"
"        const double y_min,\n"
"        const double y_max,\n"
"        const uint max_iterations\n"
"    )\n"
"{\n"
"    double x0 = ((double)get_global_id(0) / (double)get_global_size(0)) * (x_max - x_min) + x_min;\n"
"    double y0 = ((double)get_global_id(1) / (double)get_global_size(1)) * (y_max - y_min) + y_min;\n"
"    double x = 0.0;\n"
"    double y = 0.0;\n"
"    uint iteration = 0;\n"
"    double temp = 0.0;\n"
"    while ((x*x + y*y < 2.0 * 2.0) && (iteration < max_iterations))\n"
"    {\n"
"        temp = x*x - y*y + x0;\n"
"        y = 2.0 * x*y + y0;\n"
"        x = temp;\n"
"        iteration = iteration + 1;\n"
"    }\n"
"    double quotient = (double)iteration / (double)max_iterations;\n"
"    write_imagef(output_image, (int2)(get_global_id(0), get_global_id(1)), (float4)(\n"
"        0.5 * sin(M_2_PI * quotient - M_PI) + 0.5,\n"
"        0.5 * sin(M_2_PI * quotient - M_PI_2) + 0.5,\n"
"        0.5 * sin(M_2_PI * quotient) + 0.5,\n"
"        1.0\n"
"    ));\n"
"}\n"
;