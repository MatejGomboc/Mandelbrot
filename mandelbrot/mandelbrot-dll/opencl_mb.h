#ifndef OPENCL_MB_H
#define OPENCL_MB_H

extern "C" __declspec(dllexport) int get_status_message_len();
extern "C" __declspec(dllexport) char* get_status_message();

extern "C" __declspec(dllexport) bool init_opencl();
extern "C" __declspec(dllexport) void release_opencl();

extern "C" __declspec(dllexport) bool get_image_opencl(char* pixel_data,
	 unsigned width, unsigned height,
	 double x_min, double x_max,
	 double y_min, double y_max,
	 unsigned max_iterations);

#endif
