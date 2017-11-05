#define CL_HPP_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <fstream>
#include <exception>
#include <string>

#include "opencl_mb.h"


static std::string status_message = std::string();


int get_status_message_len()
{
	return status_message.length();
}


const char* get_status_message()
{
	return status_message.c_str();
}


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


static cl::Context context;
static cl::CommandQueue queue;
static cl::Kernel kernel;


bool init_opencl()
{
	try
	{
		cl::Device device = getDefaultDevice();

		int context_err;
		context = cl::Context(device, NULL, NULL, NULL, &context_err);
		if (context_err == CL_OUT_OF_HOST_MEMORY) throw std::exception("OpenCL context error: CL_OUT_OF_HOST_MEMORY.");
		else if (context_err != CL_SUCCESS) throw std::exception(("OpenCL context error: " + std::to_string(context_err)).c_str());

		std::ifstream source_file;
		source_file.open("mandelbrot.cl");

		if (!source_file.is_open()) throw std::exception("Cannot open mandelbrot.cl");

		std::string source_str(std::istreambuf_iterator<char>(source_file), (std::istreambuf_iterator<char>()));
		source_file.close();
		cl::Program::Sources sources(1, std::make_pair(source_str.c_str(), source_str.length() + 1));

		int program_err;
		cl::Program program(context, sources, &program_err);
		if (program_err == CL_INVALID_CONTEXT) throw std::exception("OpenCL program error: CL_INVALID_CONTEXT.");
		else if (program_err == CL_INVALID_VALUE) throw std::exception("OpenCL program error: invalid sources.");
		else if (program_err == CL_OUT_OF_RESOURCES) throw std::exception("OpenCL program error: device out of memory.");
		else if (program_err == CL_OUT_OF_HOST_MEMORY) throw std::exception("OpenCL program error: host out of memory.");
		else if (program_err != CL_SUCCESS) throw std::exception(("OpenCL program error: " + std::to_string(program_err)).c_str());

		int build_err = program.build("-cl-std=CL1.2");
		if (build_err != CL_BUILD_SUCCESS)
		{
			std::string err_descr;
			program.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &err_descr);
			throw std::exception((source_str + "\n" + "OpenCL program build error: " + std::to_string(build_err) + "\n\n" + err_descr).c_str());
		}

		std::string build_log;
		program.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &build_log);
		status_message = build_log;

		int queue_err;
		queue = cl::CommandQueue(context, device, NULL, &queue_err);
		if (queue_err != CL_SUCCESS) throw std::exception(("OpenCL queue error: " + std::to_string(queue_err)).c_str());

		int kernel_err;
		kernel = cl::Kernel(program, "mandelbrot", &kernel_err);
		if (kernel_err != CL_SUCCESS) throw std::exception(("OpenCL kernel error: " + std::to_string(kernel_err)).c_str());
	}
	catch (std::exception ex)
	{
		status_message = std::string(ex.what());
		return false;
	}

	return true;
}


bool get_image_opencl(std::vector<uint8_t>& pixel_data, const int width, const int height, const float x_min, const float x_max,
	const float y_min, const float y_max, const unsigned max_iterations)
{
	try
	{
		int image_err;
		cl::Image2D output_image(context, CL_MEM_WRITE_ONLY, cl::ImageFormat(CL_RGBA, CL_UNORM_INT8), width, height, 0, NULL, &image_err);
		if (image_err != CL_SUCCESS) throw std::exception(("OpenCL image creation error: " + std::to_string(image_err)).c_str());
		
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
	}
	catch (std::exception ex)
	{
		status_message = std::string(ex.what());
		return false;
	}

	status_message = std::string();

	return true;
}
