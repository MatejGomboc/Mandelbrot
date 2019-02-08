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


#include "plotter.h"
#include <string>
#include <exception>
#include <algorithm>
#include "mandelbrot_kernel.h"


Plotter::Plotter()
{
	// get number of OpenCL platforms
	cl_uint num_ocl_platforms;
	cl_int ocl_error_code = clGetPlatformIDs(0, nullptr, &num_ocl_platforms);
	if (ocl_error_code != CL_SUCCESS)
		throw std::exception(("(" + std::string(__FILE__) + ", " + std::to_string(__LINE__) + ") " +
		"OpenCL error: " + ocl_error_string(ocl_error_code)).c_str());

	// get OpenCL platform IDs
	std::vector<cl_platform_id> ocl_platforms(num_ocl_platforms);
	ocl_error_code = clGetPlatformIDs(num_ocl_platforms, ocl_platforms.data(), nullptr);
	if (ocl_error_code != CL_SUCCESS)
		throw std::exception(("(" + std::string(__FILE__) + ", " + std::to_string(__LINE__) + ") " +
		"OpenCL error: " + ocl_error_string(ocl_error_code)).c_str());

	// query all OpenCL devices
	for (cl_uint ocl_platform_indx = 0; ocl_platform_indx < num_ocl_platforms; ocl_platform_indx++)
	{
		cl_uint num_ocl_devices_part;
		ocl_error_code = clGetDeviceIDs(ocl_platforms[ocl_platform_indx], CL_DEVICE_TYPE_ALL, 0, nullptr, &num_ocl_devices_part);
		if (ocl_error_code != CL_SUCCESS)
			throw std::exception(("(" + std::string(__FILE__) + ", " + std::to_string(__LINE__) + ") " +
			"OpenCL error: " + ocl_error_string(ocl_error_code)).c_str());

		std::vector<cl_device_id> ocl_devices_part(num_ocl_devices_part);
		ocl_error_code = clGetDeviceIDs(ocl_platforms[ocl_platform_indx], CL_DEVICE_TYPE_ALL, num_ocl_devices_part, ocl_devices_part.data(), nullptr);
		if (ocl_error_code != CL_SUCCESS)
			throw std::exception(("(" + std::string(__FILE__) + ", " + std::to_string(__LINE__) + ") " +
			"OpenCL error: " + ocl_error_string(ocl_error_code)).c_str());

		m_ocl_devices.insert(m_ocl_devices.end(), ocl_devices_part.begin(), ocl_devices_part.end());
	}

	// compile and run kernel on each device, if error remove device from list
	for (cl_uint ocl_device_indx = 0; ocl_device_indx < (cl_uint)m_ocl_devices.size(); ocl_device_indx++)
	{
		cl_context ocl_context = clCreateContext(NULL, 1, &m_ocl_devices[ocl_device_indx], nullptr, nullptr, &ocl_error_code);
		if (ocl_error_code != CL_SUCCESS)
		{
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		cl_program ocl_program = clCreateProgramWithSource(ocl_context, 1, &mandelbrot_ocl_source, nullptr, &ocl_error_code);
		if (ocl_error_code != CL_SUCCESS)
		{
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		ocl_error_code = clBuildProgram(ocl_program, 0, nullptr, nullptr, nullptr, nullptr);
		if (ocl_error_code != CL_SUCCESS)
		{
			clReleaseProgram(ocl_program);
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		cl_kernel ocl_kernel = clCreateKernel(ocl_program, "mandelbrot", &ocl_error_code);
		if (ocl_error_code != CL_SUCCESS)
		{
			clReleaseProgram(ocl_program);
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		const cl_image_format ocl_image_format = {CL_RGBA, CL_UNORM_INT8};
		cl_mem ocl_image = clCreateImage2D(ocl_context, CL_MEM_WRITE_ONLY, &ocl_image_format, 1000, 1000, 0, nullptr, &ocl_error_code);
		if (ocl_error_code != CL_SUCCESS)
		{
			clReleaseKernel(ocl_kernel);
			clReleaseProgram(ocl_program);
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		ocl_error_code = clSetKernelArg(ocl_kernel, 0, sizeof(cl_mem), &ocl_image);
		if (ocl_error_code != CL_SUCCESS)
		{
			clReleaseMemObject(ocl_image);
			clReleaseKernel(ocl_kernel);
			clReleaseProgram(ocl_program);
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		const cl_float x_min = 0.0f;
		ocl_error_code = clSetKernelArg(ocl_kernel, 1, sizeof(cl_float), &x_min);
		if (ocl_error_code != CL_SUCCESS)
		{
			clReleaseMemObject(ocl_image);
			clReleaseKernel(ocl_kernel);
			clReleaseProgram(ocl_program);
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		const cl_float x_max = 1.0f;
		ocl_error_code = clSetKernelArg(ocl_kernel, 2, sizeof(cl_float), &x_max);
		if (ocl_error_code != CL_SUCCESS)
		{
			clReleaseMemObject(ocl_image);
			clReleaseKernel(ocl_kernel);
			clReleaseProgram(ocl_program);
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		const cl_float y_min = 0.0f;
		ocl_error_code = clSetKernelArg(ocl_kernel, 3, sizeof(cl_float), &y_min);
		if (ocl_error_code != CL_SUCCESS)
		{
			clReleaseMemObject(ocl_image);
			clReleaseKernel(ocl_kernel);
			clReleaseProgram(ocl_program);
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		const cl_float y_max = 0.0f;
		ocl_error_code = clSetKernelArg(ocl_kernel, 4, sizeof(cl_float), &y_max);
		if (ocl_error_code != CL_SUCCESS)
		{
			clReleaseMemObject(ocl_image);
			clReleaseKernel(ocl_kernel);
			clReleaseProgram(ocl_program);
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		const cl_uint max_iterations = 100;
		ocl_error_code = clSetKernelArg(ocl_kernel, 4, sizeof(cl_uint), &max_iterations);
		if (ocl_error_code != CL_SUCCESS)
		{
			clReleaseMemObject(ocl_image);
			clReleaseKernel(ocl_kernel);
			clReleaseProgram(ocl_program);
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		cl_command_queue ocl_command_queue = clCreateCommandQueue(ocl_context, m_ocl_devices[ocl_device_indx], 0, &ocl_error_code);
		if (ocl_error_code != CL_SUCCESS)
		{
			clReleaseMemObject(ocl_image);
			clReleaseKernel(ocl_kernel);
			clReleaseProgram(ocl_program);
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		const size_t ocl_global_work_offset[2] = {0, 0};
		const size_t ocl_global_work_size[2] = {1000, 1000};
		ocl_error_code = clEnqueueNDRangeKernel(ocl_command_queue, ocl_kernel, 2, ocl_global_work_offset, ocl_global_work_size,
			nullptr, 0, nullptr, nullptr);
		if (ocl_error_code != CL_SUCCESS)
		{
			clReleaseCommandQueue(ocl_command_queue);
			clReleaseMemObject(ocl_image);
			clReleaseKernel(ocl_kernel);
			clReleaseProgram(ocl_program);
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		const size_t ocl_origin[3] = {0, 0, 0};
		const size_t ocl_region[3] = {1000, 1000, 1};
		char* pixel_data = new char[1000 * 1000 * 4];
		ocl_error_code = clEnqueueReadImage(ocl_command_queue, ocl_image, true, ocl_origin, ocl_region, 0, 0, pixel_data, 0, nullptr, nullptr);
		if (ocl_error_code != CL_SUCCESS)
		{
			delete [] pixel_data;
			clReleaseCommandQueue(ocl_command_queue);
			clReleaseMemObject(ocl_image);
			clReleaseKernel(ocl_kernel);
			clReleaseProgram(ocl_program);
			clReleaseContext(ocl_context);
			m_ocl_devices.erase(std::remove(m_ocl_devices.begin(), m_ocl_devices.end(), m_ocl_devices[ocl_device_indx]), m_ocl_devices.end());
			ocl_device_indx--;
			continue;
		}

		delete[] pixel_data;
		clReleaseCommandQueue(ocl_command_queue);
		clReleaseMemObject(ocl_image);
		clReleaseKernel(ocl_kernel);
		clReleaseProgram(ocl_program);
		clReleaseContext(ocl_context);
		continue;
	}
}


Plotter::~Plotter()
{
}


std::string Plotter::ocl_error_string(cl_int error_code)
{
	switch (error_code)
	{
	case(CL_SUCCESS) :
		return "CL_SUCCESS";
	case(CL_DEVICE_NOT_FOUND) :
		return "CL_DEVICE_NOT_FOUND";
	case(CL_DEVICE_NOT_AVAILABLE) :
		return "CL_DEVICE_NOT_AVAILABLE";
	case(CL_COMPILER_NOT_AVAILABLE) :
		return "CL_COMPILER_NOT_AVAILABLE";
	case(CL_MEM_OBJECT_ALLOCATION_FAILURE) :
		return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case(CL_OUT_OF_RESOURCES) :
		return "CL_OUT_OF_RESOURCES";
	case(CL_OUT_OF_HOST_MEMORY) :
		return "CL_OUT_OF_HOST_MEMORY";
	case(CL_PROFILING_INFO_NOT_AVAILABLE) :
		return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case(CL_MEM_COPY_OVERLAP) :
		return "CL_MEM_COPY_OVERLAP";
	case(CL_IMAGE_FORMAT_MISMATCH) :
		return "CL_IMAGE_FORMAT_MISMATCH";
	case(CL_IMAGE_FORMAT_NOT_SUPPORTED) :
		return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case(CL_BUILD_PROGRAM_FAILURE) :
		return "CL_BUILD_PROGRAM_FAILURE";
	case(CL_MAP_FAILURE) :
		return "CL_MAP_FAILURE";
	case(CL_MISALIGNED_SUB_BUFFER_OFFSET) :
		return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST) :
		return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case(CL_COMPILE_PROGRAM_FAILURE) :
		return "CL_COMPILE_PROGRAM_FAILURE";
	case(CL_LINKER_NOT_AVAILABLE) :
		return "CL_LINKER_NOT_AVAILABLE";
	case(CL_LINK_PROGRAM_FAILURE) :
		return "CL_LINK_PROGRAM_FAILURE";
	case(CL_DEVICE_PARTITION_FAILED) :
		return "CL_DEVICE_PARTITION_FAILED";
	case(CL_KERNEL_ARG_INFO_NOT_AVAILABLE) :
		return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
	case(CL_INVALID_VALUE) :
		return "CL_INVALID_VALUE";
	case(CL_INVALID_DEVICE_TYPE) :
		return "CL_INVALID_DEVICE_TYPE";
	case(CL_INVALID_PLATFORM) :
		return "CL_INVALID_PLATFORM";
	case(CL_INVALID_DEVICE) :
		return "CL_INVALID_DEVICE";
	case(CL_INVALID_CONTEXT) :
		return "CL_INVALID_CONTEXT";
	case(CL_INVALID_QUEUE_PROPERTIES) :
		return "CL_INVALID_QUEUE_PROPERTIES";
	case(CL_INVALID_COMMAND_QUEUE) :
		return "CL_INVALID_COMMAND_QUEUE";
	case(CL_INVALID_HOST_PTR) :
		return "CL_INVALID_HOST_PTR";
	case(CL_INVALID_MEM_OBJECT) :
		return "CL_INVALID_MEM_OBJECT";
	case(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR) :
		return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case(CL_INVALID_IMAGE_SIZE) :
		return "CL_INVALID_IMAGE_SIZE";
	case(CL_INVALID_SAMPLER) :
		return "CL_INVALID_SAMPLER";
	case(CL_INVALID_BINARY) :
		return "CL_INVALID_BINARY";
	case(CL_INVALID_BUILD_OPTIONS) :
		return "CL_INVALID_BUILD_OPTIONS";
	case(CL_INVALID_PROGRAM) :
		return "CL_INVALID_PROGRAM";
	case(CL_INVALID_PROGRAM_EXECUTABLE) :
		return "CL_INVALID_PROGRAM_EXECUTABLE";
	case(CL_INVALID_KERNEL_NAME) :
		return "CL_INVALID_KERNEL_NAME";
	case(CL_INVALID_KERNEL_DEFINITION) :
		return "CL_INVALID_KERNEL_DEFINITION";
	case(CL_INVALID_KERNEL) :
		return "CL_INVALID_KERNEL";
	case(CL_INVALID_ARG_INDEX) :
		return "CL_INVALID_ARG_INDEX";
	case(CL_INVALID_ARG_VALUE) :
		return "CL_INVALID_ARG_VALUE";
	case(CL_INVALID_ARG_SIZE) :
		return "CL_INVALID_ARG_SIZE";
	case(CL_INVALID_KERNEL_ARGS) :
		return "CL_INVALID_KERNEL_ARGS";
	case(CL_INVALID_WORK_DIMENSION) :
		return "CL_INVALID_WORK_DIMENSION";
	case(CL_INVALID_WORK_GROUP_SIZE) :
		return "CL_INVALID_WORK_GROUP_SIZE";
	case(CL_INVALID_WORK_ITEM_SIZE) :
		return "CL_INVALID_WORK_ITEM_SIZE";
	case(CL_INVALID_GLOBAL_OFFSET) :
		return "CL_INVALID_GLOBAL_OFFSET";
	case(CL_INVALID_EVENT_WAIT_LIST) :
		return "CL_INVALID_EVENT_WAIT_LIST";
	case(CL_INVALID_EVENT) :
		return "CL_INVALID_EVENT";
	case(CL_INVALID_OPERATION) :
		return "CL_INVALID_OPERATION";
	case(CL_INVALID_GL_OBJECT) :
		return "CL_INVALID_GL_OBJECT";
	case(CL_INVALID_BUFFER_SIZE) :
		return "CL_INVALID_BUFFER_SIZE";
	case(CL_INVALID_MIP_LEVEL) :
		return "CL_INVALID_MIP_LEVEL";
	case(CL_INVALID_GLOBAL_WORK_SIZE) :
		return "CL_INVALID_GLOBAL_WORK_SIZE";
	case(CL_INVALID_PROPERTY) :
		return "CL_INVALID_PROPERTY";
	case(CL_INVALID_IMAGE_DESCRIPTOR) :
		return "CL_INVALID_IMAGE_DESCRIPTOR";
	case(CL_INVALID_COMPILER_OPTIONS) :
		return "CL_INVALID_COMPILER_OPTIONS";
	case(CL_INVALID_LINKER_OPTIONS) :
		return "CL_INVALID_LINKER_OPTIONS";
	case(CL_INVALID_DEVICE_PARTITION_COUNT) :
		return "CL_INVALID_DEVICE_PARTITION_COUNT";
	default:
		return "UNKNOWN_OCL_ERROR_CODE(" + std::to_string(error_code) + ")";
	}
}