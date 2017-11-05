#include <iostream>
#include <vector>
#include <cstdint>

#include "opencl_mb.h"
#include "bitmap.h"


int main()
{
	const int width = 3000;
	const int height = 2000;

	std::vector<uint8_t> pixel_data(width * height * 4);
	std::fill(pixel_data.begin(), pixel_data.end(), 0);

	const float x_min = -2.0;
	const float x_max = 1.0;
	const float y_min = -1.0;
	const float y_max = 1.0;

	const unsigned max_iterations = 40;

	if (!init_opencl())
	{
		std::cerr << "Failed to initialise OpenCL.\n" << std::string(get_status_message()) << std::endl;
		return -1;
	}

	std::cout << std::string(get_status_message()) << "\nOpenCL initialised." << std::endl;

	if (!get_image_opencl(pixel_data))
	{
		std::cerr << "Failed to get image from OpenCL.\n" << std::string(get_status_message()) << std::endl;
		return -1;
	}

	try
	{
		save_to_bmp_file("mandelbrot.bmp", width, height, pixel_data);
	}
	catch (std::exception ex)
	{
		std::cerr << ex.what() << std::endl;
		return -1;
	}

	std::cout << "Bitmap file written." << std::endl;

	return 0;
}
