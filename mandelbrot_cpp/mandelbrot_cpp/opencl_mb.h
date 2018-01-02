#ifndef OPENCL_MB_H
#define OPENCL_MB_H

#include <vector>
#include <cstdint>

extern int get_status_message_len();
extern const char* get_status_message();

extern bool init_opencl();

extern bool get_image_opencl(std::vector<uint8_t>& pixel_data,
	const unsigned width = 3000, const unsigned height = 2000,
	const float x_min = -2.0, const float x_max = 1.0,
	const float y_min = -1.0, const float y_max = 1.0,
	const unsigned max_iterations = 40);

#endif
