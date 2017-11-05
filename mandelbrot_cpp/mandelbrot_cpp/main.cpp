#define CL_HPP_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <exception>
#include <vector>
#include <cstdint>

#include "bitmap.h"


static bool supportsImages(const cl::Device& dev)
{
	int result;
	dev.getInfo(CL_DEVICE_IMAGE_SUPPORT, &result);
	return (bool)result;
}


static cl::Device getDefaultDevice()
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	for (int i = 0; i < platforms.size(); i++)
	{
		std::vector<cl::Device> devices;
		if (platforms[i].getDevices(CL_DEVICE_TYPE_GPU, &devices) == CL_DEVICE_NOT_FOUND) continue;

		for (int j = 0; j < devices.size(); j++)
		{
			if (supportsImages(devices[j])) return devices[j];
		}
	}

	throw std::exception("Cannot find device with image support.");
}


int main()
{
	cl::Device device;
	try
	{
		device = getDefaultDevice();
	}
	catch (std::exception ex)
	{
		std::cerr << ex.what() << std::endl;
		return -1;
	}

	int context_err;
	cl::Context context(device, NULL, NULL, NULL, &context_err);
	if (context_err == CL_OUT_OF_HOST_MEMORY)
	{
		std::cerr << "OpenCL context error: CL_OUT_OF_HOST_MEMORY." << std::endl;
		return -1;
	}
	else if (context_err != CL_SUCCESS)
	{
		std::cerr << "OpenCL context error: " << context_err << "." << std::endl;
		return -1;
	}

	std::ifstream source_file;
	source_file.open("mandelbrot.cl");

	if (!source_file.is_open())
	{
		std::cerr << "Cannot open mandelbrot.cl" << std::endl;
		return -1;
	}

	std::string source_str(std::istreambuf_iterator<char>(source_file), (std::istreambuf_iterator<char>()));
	source_file.close();
	cl::Program::Sources sources(1, std::make_pair(source_str.c_str(), source_str.length() + 1));

	int program_err;
	cl::Program program(context, sources, &program_err);
	if (program_err == CL_INVALID_CONTEXT)
	{
		std::cerr << "OpenCL program error: CL_INVALID_CONTEXT." << std::endl;
		return -1;
	}
	else if (program_err == CL_INVALID_VALUE)
	{
		std::cerr << "OpenCL program error: invalid sources." << std::endl;
		return -1;
	}
	else if (program_err == CL_OUT_OF_RESOURCES)
	{
		std::cerr << "OpenCL program error: device out of memory." << std::endl;
		return -1;
	}
	else if (program_err == CL_OUT_OF_HOST_MEMORY)
	{
		std::cerr << "OpenCL program error: host out of memory." << std::endl;
		return -1;
	}
	else if (program_err != CL_SUCCESS)
	{
		std::cerr << "OpenCL program error: " << program_err << "." << std::endl;
		return -1;
	}

	int build_err = program.build("-cl-std=CL1.2");
	if (build_err != CL_BUILD_SUCCESS)
	{
		std::string err_descr;
		program.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &err_descr);

		std::cerr << source_str << std::endl;
		std::cerr << "OpenCL program build error: " << build_err << std::endl << std::endl;
		std::cerr << err_descr << std::endl;
		return -1;
	}
	
	int queue_err;
	cl::CommandQueue queue(context, device, NULL, &queue_err);
	if (queue_err != CL_SUCCESS)
	{
		std::cerr << "OpenCL queue error: " << queue_err << "." << std::endl;
		return -1;
	}

	int kernel_err;
	cl::Kernel kernel(program, "mandelbrot", &kernel_err);
	if (kernel_err != CL_SUCCESS)
	{
		std::cerr << "OpenCL kernel error: " << kernel_err << "." << std::endl;
		return -1;
	}

	const int width = 2048;
	const int height = 1024;

	std::vector<uint8_t> pixel_data(width * height * 4);
	std::fill(pixel_data.begin(), pixel_data.end(), 0);

	int image_err;
	cl::Image2D output_image(context, CL_MEM_WRITE_ONLY, cl::ImageFormat(CL_RGBA, CL_UNORM_INT8), width, height, 0, NULL, &image_err);
	if (image_err != CL_SUCCESS)
	{
		std::cerr << "OpenCL image creation error: " << image_err << "." << std::endl;
		return -1;
	}

	const cl_float x_min = -2.0;
	const cl_float x_max = 1.0;
	const cl_float y_min = -1.0;
	const cl_float y_max = 1.0;

	const cl_uint max_iterations = 40;

	kernel.setArg(0, output_image);
	kernel.setArg(1, x_min);
	kernel.setArg(2, x_max);
	kernel.setArg(3, y_min);
	kernel.setArg(4, y_max);
	kernel.setArg(5, max_iterations);

	queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(width, height));

	cl::size_t<3> origin;
	origin[0] = 0;
	origin[1] = 0;
	origin[2] = 0;

	cl::size_t<3> region;
	region[0] = width;
	region[1] = height;
	region[2] = 1;

	queue.enqueueReadImage(output_image, true, origin, region, 0, 0, pixel_data.data());

	try
	{
		save_to_bmp_file("mandelbrot.bmp", width, height, pixel_data);
	}
	catch (std::exception ex)
	{
		std::cout << ex.what() << std::endl;
		return -1;
	}

	std::cout << "Bitmap file written." << std::endl;

	return 0;
}
