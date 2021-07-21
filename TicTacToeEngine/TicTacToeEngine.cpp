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

const int lines[8][3] = {
	{0, 1, 2},
  {3, 4, 5}, 
  {6, 7, 8}, 
  {0, 3, 6}, 
  {1, 4, 7}, 
  {2, 5, 8}, 
  {0, 4, 8}, 
  {2, 4, 6}
};

const int scores[4] = {0, 1, 10, 100};

const int MAX = 1000;
const int MIN = -1000;

// Algo similar al algoritmo 1
int alphaBetaPrunning(int depth, int nodeIndex, bool maximizingPlayer, int values[], int alpha, int beta) {

}

int* leafCalculationCPU(int Pl[][], int pindex, int eindex) {
	int length = sizeof(Pl) / sizeof(int);
	int V[length];

	for (int i = 0; i < length; i++) {
		int board[] = Pl[i];
    int accum = 0;

    for(int l[] : lines) {
      // Recorrer el tablero y asignar puntajes
      int score = scores[(board[l[0]] == pindex) + (board[l[1]] == pindex) + (board[l[2]] == pindex)];
      score -= scores[(board[l[0]] == eindex) + (board[l[1]] == eindex) + (board[l[2]] == eindex)];
      accum += score; 
    }
    V[i] = accum;
	}

	return V;
}

int* branchCalculationCPU(int Pl[][], int pindex, int eindex) {
	int length = sizeof(Pl) / sizeof(int);
	int Ml[length][][];

	for (int i = 0; i < length; i++) {
		int board[] = Pl[i]; //2
    int generatedMoves[][];

    for (int j = 0; j < 9; j++) {
      if (board[j] == 0) {
        int new_move[9];
        std::copy(board, board + 9, new_move);
        new_move[j] = pindex;
        std::copy(new_move, new_move + 9, generatedMoves[j]);
      }
    }
    Ml[i] = generatedMoves;
	}
}

// Algoritmo 2
int gameTreeMax(TreeNode P0, int pindex, int eindex) {

	// Setting P0 as the root of the game tree T
	TreeNode T = P0;
	int M_best[9];

	int Lmin = 3;
	int Bmin = 3;
	int Lmax = 6;
	int Bmax = 6;
	int l = 0;
	int b = 0;

	int remLeaves = 0; // remaining leaves
	int remBranches = 1; // remaining branches

	bool maximizingPlayer = true; // O puede depender de la altura a la que se haga el cálculo.

	while (T != NULL) {
		if (remLeaves > Lmin) {
			int** leaves = T.child; // Get l leaves from tree T
			int plSize = sizeof(leaves) / sizeof(TreeNode);
			
			// Call leafCalculationFunction on GPU with leaves
			const int N_ELEMENTS = 1024 * 1024;
			int platform_id = 0, device_id = 0;

			try {
				std::unique_ptr<int[][]> Pl(new int[plSize][9]);   // Or you can use simple dynamic arrays like: int* A = new int[N_ELEMENTS];
				std::unique_ptr<int[]> V(new int[plSize]);

				for (int i = 0; i < plSize; ++i) {
					Pl[i] = T.child[i].board;
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
				cl::Buffer bufferPl = cl::Buffer(context, CL_MEM_READ_ONLY, plSize * sizeof(int));
				cl::Buffer bufferPIndex = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
				cl::Buffer bufferEIndex = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
				cl::Buffer bufferV = cl::Buffer(context, CL_MEM_WRITE_ONLY, plSize * sizeof(int));

				// Copy the input data to the input buffers using the command queue.
				queue.enqueueWriteBuffer(bufferPl, CL_FALSE, 0, plSize * sizeof(int), Pl.get());
				queue.enqueueWriteBuffer(bufferPIndex, CL_FALSE, 0, sizeof(int), pindex);
				queue.enqueueWriteBuffer(bufferEIndex, CL_FALSE, 0, sizeof(int), eindex);

				// Read the program source
				std::ifstream sourceFile("mykernel.cl");
				std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
				cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()));

				// Make program from the source code
				cl::Program program = cl::Program(context, source);

				// Build the program for the devices
				program.build(devices);

				// Make kernel
				cl::Kernel vecadd_kernel(program, "leafCalculation");

				// Set the kernel arguments
				vecadd_kernel.setArg(0, bufferPl);
				vecadd_kernel.setArg(1, bufferV);
				vecadd_kernel.setArg(2, bufferPIndex);
				vecadd_kernel.setArg(3, bufferEIndex);

				// Execute the kernel
				cl::NDRange global(plSize);
				cl::NDRange local(256);
				queue.enqueueNDRangeKernel(vecadd_kernel, cl::NullRange, global, local);

				// Copy the output data back to the host
				queue.enqueueReadBuffer(bufferV, CL_TRUE, 0, plSize * sizeof(int), V.get());

			// Get evaluatedValueList from GPU: V
			// Ya tenemos los values aqui
			for (int v : V) {
				// Update parent node in T
				TreeNode parentNode; // Minimax de alguna forma

				if (v >= beta) {
					M_best = v;
				} 
				if (v >= alpha) {
					// Actualizar el best move
					alpha = v;
				}

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
			TreeNode* branches = T.getBranches(b);  // Get b branches from tree T
			// Call branchCalculationFunction in GPU

			// Get childNodeList from the GPU
			int** childNodeList [5];  // Could be any size
			for (int* c : childNodeList) {
				// Update T by generated child node c
			}
		}
		else if (remBranches > 0) {
			TreeNode branch = T.getBranch(); // Get one leaf node from tree T
			// Call branchCalculationFunction on CPU
			int* newNodes = branchCalculationCPU(branch.getBoard(), 0, 1);
			// Update T by new child nodes from node
			for (int* n : newNodes) {
				branch.appendChild(new TreeNode(n));
			}
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
