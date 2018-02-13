#include <string>
#include <vector>
#include <tuple>
#include <CL\cl.hpp>

extern std::string oclErrorCodeString(const int error_code);

extern std::vector<std::string> oclPlatformExtensionsList(const cl::Platform& platform);

extern std::tuple<cl::Device, const int, const long> oclPlatformDeviceMaxFLOPs(const cl::Platform& platform);
