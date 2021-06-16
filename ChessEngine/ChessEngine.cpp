#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <stdlib.h>
#include "TreeNode.h"

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#ifdef __APPLE__
#include <OpenCL/opencl.hpp>
#else
#include <CL/opencl.hpp>
#endif

int leafCalculationCPU() {
	return 0;
}

int branchCalculationCPU() {
	return 0;
}


int gameTreeMax(TreeNode P0) {

	// Setting P0 as the root of the game tree T
	TreeNode T = P0;
	int* M_best = 0;

	int Lmin = 0;
	int Bmin = 0;
	int Lmax = 0;
	int Bmax = 0;
	int l = 0;
	int b = 0;

	int remLeaves; // remaining leaves
	int remBranches; // remaining branches

	while (T != NULL) {
		if (remLeaves > Lmin) {
			int** leaves = T.children.at(l); // Get l leaves from tree T
			
			// Call leafCalculationFunction on GPU with leaves
			
			// Get evaluatedValueList from GPU

			float evaluatedValueList [l];

			for (float v : evaluatedValueList) {
				// Update parent node in T
				TreeNode parentNode;

				// If parent node is root, set M_best to value
				if (parentNode == P0) {
					M_best = v;
				}
				// Pruning
			}
		}
		else if (remLeaves > 0) {
			int* leaf = 0; // Get one leaf node from tree T
			
			// Call leafCalculationFunction on CPU
			// Update the tree T by the evaluated leaf node
			int v = 0; // int v = leafCalculationFunction(leaf);
			TreeNode parentNode;
			if (parentNode == P0) {
					M_best = v;
				}
			// Pruning
		}
		else if (remBranches > Bmin) {
			int** branches [b];  // Get b branches from tree T
			// Call branchCalculationFunction in GPU
			// Get childNodeList from the GPU
			int** childNodeList [5];  // Could be any size
			for (int* c : childNodeList) {
				// Update T by generated child node c
			}
		}
		else if (remBranches > 0) {
			int* leaf = 0; // Get one leaf node from tree T
			// Call branchCalculationFunction on CPU
			// Update T by new child nodes from node
		}
	}
	return M_best;
}

int main()
{
	const int N_ELEMENTS = 1024 * 1024;
	int platform_id = 0, device_id = 0;

	try {
		std::unique_ptr<int[]> A(new int[N_ELEMENTS]);      // Or you can use simple dynamic arrays like: int* A = new int[N_ELEMENTS];
		std::unique_ptr<int[]> B(new int[N_ELEMENTS]);
		std::unique_ptr<int[]> C(new int[N_ELEMENTS]);

		for (int i = 0; i < N_ELEMENTS; ++i) {
			A[i] = i;
			B[i] = i;
		}

		// Query for platforms
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		// Get a list of devices on this platform
		std::vector<cl::Device> devices;
		// Select the platform.
		platforms[platform_id].getDevices(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices);

		// Create a context
		cl::Context context(devices);

		// Create a command queue
		// Select the device.
		cl::CommandQueue queue = cl::CommandQueue(context, devices[device_id]);

		// Create the memory buffers
		cl::Buffer bufferA = cl::Buffer(context, CL_MEM_READ_ONLY, N_ELEMENTS * sizeof(int));
		cl::Buffer bufferB = cl::Buffer(context, CL_MEM_READ_ONLY, N_ELEMENTS * sizeof(int));
		cl::Buffer bufferC = cl::Buffer(context, CL_MEM_WRITE_ONLY, N_ELEMENTS * sizeof(int));

		// Copy the input data to the input buffers using the command queue.
		queue.enqueueWriteBuffer(bufferA, CL_FALSE, 0, N_ELEMENTS * sizeof(int), A.get());
		queue.enqueueWriteBuffer(bufferB, CL_FALSE, 0, N_ELEMENTS * sizeof(int), B.get());

		// Read the program source
		std::ifstream sourceFile("mykernel.cl");
		std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
		cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()));

		// Make program from the source code
		cl::Program program = cl::Program(context, source);

		// Build the program for the devices
		program.build(devices);

		// Make kernel
		cl::Kernel vecadd_kernel(program, "vecadd");

		// Set the kernel arguments
		vecadd_kernel.setArg(0, bufferA);
		vecadd_kernel.setArg(1, bufferB);
		vecadd_kernel.setArg(2, bufferC);

		// Execute the kernel
		cl::NDRange global(N_ELEMENTS);
		cl::NDRange local(256);
		queue.enqueueNDRangeKernel(vecadd_kernel, cl::NullRange, global, local);

		// Copy the output data back to the host
		queue.enqueueReadBuffer(bufferC, CL_TRUE, 0, N_ELEMENTS * sizeof(int), C.get());

		// Verify the result
		bool result = true;
		for (int i = 0; i < N_ELEMENTS; i++) {
			if (C[i] != A[i] + B[i]) {
				result = false;
				break;
			}
		}
		if (result)
			std::cout << "Success!\n";
		else
			std::cout << "Failed!\n";
	}
	catch (cl::Error err) {
		std::cout << "Error: " << err.what() << "(" << err.err() << ")" << std::endl;
		return(EXIT_FAILURE);
	}

	std::cout << "Done.\n";
	return(EXIT_SUCCESS);
}
