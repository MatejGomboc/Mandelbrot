#define CL_HPP_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <fstream>
#include <exception>
#include <string>
#include <algorithm> 

#include "opencl_mb.h"
#include "mandelbrot-kernel.h"
#include "opencl-support-lib.h"


static std::string status_message = std::string();
static cl::Context context;
static cl::CommandQueue queue;
static cl::Kernel kernel;
static bool initialised = false;


__declspec(dllexport) int get_status_message_len()
{
	return status_message.length();
}


__declspec(dllexport) char* get_status_message()
{
	return const_cast<char*>(status_message.c_str());
}


static bool supports_images(const cl::Device& dev)
{
	int result;
	dev.getInfo(CL_DEVICE_IMAGE_SUPPORT, &result);
	return (result != 0);
}


static bool supports_double(const cl::Device& dev)
{
	int result;
	dev.getInfo(CL_DEVICE_DOUBLE_FP_CONFIG, &result);
	return (result != 0);
}


static bool supports_double(const cl::Platform& pltf)
{
	std::string query;
	pltf.getInfo(CL_PLATFORM_EXTENSIONS, &query);
	return (query.find("cl_khr_fp64") != std::string::npos);
}


static int get_flops(const cl::Device& dev)
{
	int max_compute_units;
	dev.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &max_compute_units);

	int max_clock_frequency;
	dev.getInfo(CL_DEVICE_MAX_CLOCK_FREQUENCY, &max_clock_frequency);

	return(max_clock_frequency * max_compute_units);
}


static bool compare(const cl::Device& dev1, const cl::Device& dev2)
{
	return(get_flops(dev1) < get_flops(dev2));
}


static cl::Device get_default_device()
{
	std::vector<cl::Platform> platforms;
	std::vector<cl::Device> devices;

	cl::Platform::get(&platforms);

	for (int i = 0; i < platforms.size(); i++)
	{
		if (!supports_double(platforms[i])) continue;

		std::vector<cl::Device> devices_part;
		if (platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &devices_part) == CL_DEVICE_NOT_FOUND) continue;

		for (int j = 0; j < devices_part.size(); j++)
		{
			if (supports_images(devices_part[j]) && supports_double(devices_part[j]))
			{
				devices.push_back(devices_part[j]);
			}
		}
	}

	if (devices.size() < 1)
	{
		throw std::exception("Cannot find device with image and double floating point precision support.");
	}
	else
	{
		return *(std::max_element(std::begin(devices), std::end(devices), compare));
	}
}


__declspec(dllexport) bool init_opencl()
{
	if (!initialised)
	{
		try
		{
			cl::Device device = get_default_device();

			int context_err = 0;
			context = cl::Context(device, NULL, NULL, NULL, &context_err);
			if (context_err != CL_SUCCESS) throw std::exception(("OpenCL context error: " + oclErrorCodeString(context_err) + ".").c_str());

			int queue_err = 0;
			queue = cl::CommandQueue(context, device, NULL, &queue_err);
			if (queue_err != CL_SUCCESS) throw std::exception(("OpenCL command queue error: " + std::to_string(queue_err)).c_str());

			std::string source_string = std::string(mandelbrot_opencl_source_string);

			int program_err = 0; // NEEDS TO BE SO !!!
			cl::Program program(context, source_string, &program_err);
			if (program_err != CL_SUCCESS) throw std::exception(("OpenCL program error: " + oclErrorCodeString(program_err) + ".").c_str());

			int build_err = program.build("-cl-std=CL1.2");
			if (build_err != CL_SUCCESS)
			{
				std::string err_descr;
				program.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &err_descr);
				throw std::exception(("OpenCL program build error: " + oclErrorCodeString(build_err) + ".\n\n" + err_descr).c_str());
			}
			std::string build_log;
			program.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &build_log);
			status_message = build_log;

			int kernel_err = 0;
			kernel = cl::Kernel(program, "mandelbrot", &kernel_err);
			if (kernel_err != CL_SUCCESS) throw std::exception(("OpenCL kernel error: " + oclErrorCodeString(kernel_err) + ".").c_str());
		}
		catch (std::exception ex)
		{
			status_message = std::string(ex.what());
			return false;
		}

		initialised = true;
		return true;
	}

	status_message = "Cannot initialise OpenCL, already initialised.";
	return false;
}


__declspec(dllexport) void release_opencl()
{
	kernel = cl::Kernel();
	queue = cl::CommandQueue();
	context = cl::Context();
	initialised = false;
}


__declspec(dllexport) bool get_image_opencl(char* pixel_data, unsigned width, unsigned height, double x_min, double x_max,
	double y_min, double y_max, unsigned max_iterations)
{
	if (initialised)
	{
		try
		{
			int image_err;
			cl::Image2D output_image(context, CL_MEM_WRITE_ONLY, cl::ImageFormat(CL_RGBA, CL_UNORM_INT8), width, height, 0, NULL, &image_err);
			if (image_err != CL_SUCCESS) throw std::exception(("OpenCL image creation error: " + oclErrorCodeString(image_err)).c_str());

			int arg_error = kernel.setArg(0, output_image);
			if (arg_error != CL_SUCCESS) throw std::exception(("OpenCL setArg(output_image) error: " + oclErrorCodeString(arg_error) + ".").c_str());
			arg_error = kernel.setArg(1, x_min);
			if (arg_error != CL_SUCCESS) throw std::exception(("OpenCL setArg(x_min) error: " + oclErrorCodeString(arg_error) + ".").c_str());
			arg_error = kernel.setArg(2, x_max);
			if (arg_error != CL_SUCCESS) throw std::exception(("OpenCL setArg(x_max) error: " + oclErrorCodeString(arg_error) + ".").c_str());
			arg_error = kernel.setArg(3, y_min);
			if (arg_error != CL_SUCCESS) throw std::exception(("OpenCL setArg(y_min) error: " + oclErrorCodeString(arg_error) + ".").c_str());
			arg_error = kernel.setArg(4, y_max);
			if (arg_error != CL_SUCCESS) throw std::exception(("OpenCL setArg(y_max) error: " + oclErrorCodeString(arg_error) + ".").c_str());
			arg_error = kernel.setArg(5, max_iterations);
			if (arg_error != CL_SUCCESS) throw std::exception(("OpenCL setArg(max_iterations) error: " + oclErrorCodeString(arg_error) + ".").c_str());

			int exec_error = queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(width, height));
			if (exec_error != CL_SUCCESS) throw std::exception(("OpenCL enqueueNDRangeKernel error: " + oclErrorCodeString(exec_error) + ".").c_str());

			cl::size_t<3> origin;
			origin[0] = 0;
			origin[1] = 0;
			origin[2] = 0;

			cl::size_t<3> region;
			region[0] = width;
			region[1] = height;
			region[2] = 1;

			int read_err = queue.enqueueReadImage(output_image, true, origin, region, 0, 0, pixel_data);
			if (read_err != CL_SUCCESS) throw std::exception(("OpenCL enqueueReadImage error: " + oclErrorCodeString(read_err) + ".").c_str());
		}
		catch (std::exception ex)
		{
			status_message = std::string(ex.what());
			return false;
		}

		status_message = "OpenCL image successfuly created.";
		return true;
	}

	status_message = "OpenCL not initialised.";
	return false;
}
