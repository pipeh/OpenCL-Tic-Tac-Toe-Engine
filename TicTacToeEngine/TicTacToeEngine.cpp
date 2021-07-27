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

int leafCalculationCPU(int* board, int pindex, int eindex) {
	int V;
	int accum = 0;

	for (int i = 0; i < 8; i++) {
		const int* l = lines[i];
		// Recorrer el tablero y asignar puntajes
		int score = scores[(board[l[0]] == pindex) + (board[l[1]] == pindex) + (board[l[2]] == pindex)];
		score -= scores[(board[l[0]] == eindex) + (board[l[1]] == eindex) + (board[l[2]] == eindex)];
		accum += score; 
	}
	V = accum;
	return V;
}

int* branchCalculationCPU(int* board, int pindex, int moves) {
	int* Ml = new int[9 * moves];

	int count = 0;
    for (int i = 0; i < 9; i++) {
		if (board[i] == 0) {

			for (int j = 0; j < 9; j++) {
				Ml[count * 9 + j] = board[j];
			}
			Ml[count * 9 + i] = pindex;
			count++;
      }
    }
	return Ml;
}


int* leafCalculationGPUCall(std::vector<TreeNode*> leaves) {
	// Call leafCalculationFunction on GPU with leaves
	int plSize = leaves.size();

	const int N_ELEMENTS = 1024 * 1024;
	int platform_id = 0, device_id = 0;

	std::vector<int> Pl;  // Or you can use simple dynamic arrays like: int* A = new int[N_ELEMENTS];
	int* V = new int[plSize];
	int pindex = 2;
	int eindex = 1;

	for (int i = 0; i < plSize; ++i) {
		int* board = leaves[i]->getBoard();
		for (int j = 0; j < 9; j++) {
			Pl.push_back(board[j]);
		}
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
	cl::Buffer bufferPl = cl::Buffer(context, CL_MEM_READ_ONLY, plSize * sizeof(int) * 9);
	cl::Buffer bufferPIndex = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
	cl::Buffer bufferEIndex = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
	cl::Buffer bufferV = cl::Buffer(context, CL_MEM_WRITE_ONLY, plSize * sizeof(int));

	// Copy the input data to the input buffers using the command queue.
	queue.enqueueWriteBuffer(bufferPl, CL_FALSE, 0, plSize * sizeof(int) * 9, &Pl[0]);
	queue.enqueueWriteBuffer(bufferPIndex, CL_FALSE, 0, sizeof(int), &pindex);
	queue.enqueueWriteBuffer(bufferEIndex, CL_FALSE, 0, sizeof(int), &eindex);

	// Read the program source
	std::ifstream sourceFile("mykernel.cl");
	std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()));

	// Make program from the source code
	cl::Program program = cl::Program(context, source);

	// Build the program for the devices
	try
	{
		program.build(devices);
	}
	catch (cl::Error& e)
	{
		if (e.err() == CL_BUILD_PROGRAM_FAILURE)
		{
			for (cl::Device dev : devices)
			{
				// Check the build status
				cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(dev);
				if (status != CL_BUILD_ERROR)
					continue;

				// Get the build log
				std::string name = dev.getInfo<CL_DEVICE_NAME>();
				std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev);
				std::cerr << "Build log for " << name << ":" << std::endl
					<< buildlog << std::endl;
			}
		}
		else {
			throw e;
		}
	}

	// Make kernel
	cl::Kernel lckernel(program, "leafCalculation");

	// Set the kernel arguments
	lckernel.setArg(0, bufferPl);
	lckernel.setArg(1, bufferV);
	lckernel.setArg(2, bufferPIndex);
	lckernel.setArg(3, bufferEIndex);

	// Execute the kernel
	cl::NDRange global(plSize);
	cl::NDRange local(1);

	queue.enqueueNDRangeKernel(lckernel, cl::NullRange, global, local);

	// Copy the output data back to the host
	queue.enqueueReadBuffer(bufferV, CL_TRUE, 0, plSize * sizeof(int), V);

	return V;
}

int* branchCalculationGPUCall(std::vector<TreeNode*> branches, int moves) {
	// Call leafCalculationFunction on GPU with leaves
	int plSize = branches.size();

	const int N_ELEMENTS = 1024 * 1024;
	int platform_id = 0, device_id = 0;

	std::vector<int> Pl;  // Or you can use simple dynamic arrays like: int* A = new int[N_ELEMENTS];
	int* M = new int[plSize * 9 * moves];
	int pindex = 2;

	for (int i = 0; i < plSize; ++i) {
		int* board = branches[i]->getBoard();
		for (int j = 0; j < 9; j++) {
			Pl.push_back(board[j]);
		}
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
	cl::Buffer bufferPl = cl::Buffer(context, CL_MEM_READ_ONLY, plSize * sizeof(int) * 9);
	cl::Buffer bufferPIndex = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
	cl::Buffer bufferMoves = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(int));
	cl::Buffer bufferM = cl::Buffer(context, CL_MEM_WRITE_ONLY, plSize * sizeof(int) * 9 * moves);

	// Copy the input data to the input buffers using the command queue.
	queue.enqueueWriteBuffer(bufferPl, CL_FALSE, 0, plSize * sizeof(int) * 9, &Pl[0]);
	queue.enqueueWriteBuffer(bufferPIndex, CL_FALSE, 0, sizeof(int), &pindex);
	queue.enqueueWriteBuffer(bufferMoves, CL_FALSE, 0, sizeof(int), &moves);

	// Read the program source
	std::ifstream sourceFile("mykernel.cl");
	std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()));

	// Make program from the source code
	cl::Program program = cl::Program(context, source);

	// Build the program for the devices
	try
	{
		program.build(devices);
	}
	catch (cl::Error& e)
	{
		if (e.err() == CL_BUILD_PROGRAM_FAILURE)
		{
			for (cl::Device dev : devices)
			{
				// Check the build status
				cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(dev);
				if (status != CL_BUILD_ERROR)
					continue;

				// Get the build log
				std::string name = dev.getInfo<CL_DEVICE_NAME>();
				std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev);
				std::cerr << "Build log for " << name << ":" << std::endl
					<< buildlog << std::endl;
			}
		}
		else {
			throw e;
		}
	}

	// Make kernel
	cl::Kernel lckernel(program, "branchCalculation");

	// Set the kernel arguments
	lckernel.setArg(0, bufferPl);
	lckernel.setArg(1, bufferM);
	lckernel.setArg(2, bufferPIndex);
	lckernel.setArg(3, bufferMoves);

	// Execute the kernel
	cl::NDRange global(plSize);
	cl::NDRange local(1);

	queue.enqueueNDRangeKernel(lckernel, cl::NullRange, global, local);

	// Copy the output data back to the host
	queue.enqueueReadBuffer(bufferM, CL_TRUE, 0, plSize * sizeof(int) * 9 * moves, M);

	return M;
}

// Algoritmo 2
int* gameTreeMax(TreeNode *P0, int pindex, int eindex, int moves) {

	// Alpha-Beta Pruning
	//int alpha = MIN;
	//int beta = MAX;
	int M_best[9];

	// Setting P0 as the root of the game tree T
	TreeNode *T = P0;
	int depth = 0;

	int remLeaves = 0; // remaining leaves
	int remBranches = 1; // remaining branches
	int acumLeaves = 0;
	int acumBranches = 0;

	int Lmin = 3;
	int Bmin = 3;
	//int Lmax = 6;
	//int Bmax = 6;

	int n = 0;
	int *board = P0->getBoard();

	for (int i = 0; i < 9; i++) {
		if (board[i] != 0) {
			n++;
		}
	}

	while (T != NULL) {
		if (remLeaves > Lmin) {
			std::vector<TreeNode*> leaves = T->getChilds(); // Get l leaves from tree T at depth
			
			// Call leafCalculationFunction on GPU with leaves
			int *V = leafCalculationGPUCall(leaves); // Get evaluatedValueList from GPU: V

			// Quizás aquí debería haber un for por cada hoja analizada
			for (int i = 0; i < leaves.size(); i++) {
				// Get value, parentNode, and depth
				int v = V[i];
				TreeNode* parentNode = leaves[i]->getParent();
				int parentBest = parentNode->getValue();

				if (depth % 2 == 0) {
					// Update parent as maximizing player
					int best = std::max(parentBest, v);
					parentNode->setValue(best);

					//alpha = std::max(alpha, best);

					// Alpha Beta Pruning
					//if (beta <= alpha) {
					//	break;
					//}
				} else {
					// Update parent as minimizing player
					int best = std::min(parentBest, v);
					parentNode->setValue(best);

					//beta = std::min(beta, best);

					// Alpha Beta Pruning
					//if (beta <= alpha) {
					//	break;
					//}
				}

				// If parent node is NULL, set M_best to movement
				//if (parentNode == P0) {
				//	int best = v;
				//}
				// Pruning
				parentNode->removeChild(i);
			}
		}
		else if (remLeaves > 0) {
			TreeNode* leaf = T->getChild(0); // Get one leaf node from tree T
			
			// Call leafCalculationFunction on CPU
			int v = leafCalculationCPU(leaf->getBoard(), 1, 0); // TODO: asignar pindex, eindex

			// Update the tree T by the evaluated leaf node
			TreeNode* parentNode = leaf->getParent();

			if (depth % 2 == 0) {
				// Maximizing player
				int best = std::max(best, v);
				parentNode->setValue(best);

				//alpha = std::max(alpha, best);

				// Alpha Beta Pruning
				//if (beta <= alpha) {
				//	break;
				//}
			} else {
				// Minimizing player
				int best = std::min(best, v);
				parentNode->setValue(best);

				//beta = std::min(beta, best);

				// Alpha Beta Pruning
				//if (beta <= alpha) {
				//	break;
				//}
			}

			//if (parentNode == P0) {
			//	int best = v;
			//}
			// Pruning
			parentNode->removeChild(0);
		}

		else if (remBranches > Bmin) {
			int remMoves = moves - depth;
			std::vector<TreeNode*> branches = T->getBranches(3);  // Get b branches from tree T
			// Call branchCalculationFunction in GPU
			int* childNodeList = branchCalculationGPUCall(branches, remMoves);
			// Get childNodeList from the GPU

			// Update T by generated child node c TODO: Revisar
			for (int i = 0; i < branches.size(); i++) {
				for (int j = 0; j < remMoves; j++) {
					int board[9];
					for (int k = 0; k < 9; k++) {
						board[k] = childNodeList[(i + j) * 9 + k];
					}
					branches[i]->appendChild(new TreeNode(board));
				}
			}
		}
		else if (remBranches > 0) {
			int remMoves = moves - depth;
			std::vector<TreeNode*> branch = T->getBranches(1); // Get one leaf node from tree T
			// Call branchCalculationFunction on CPU
			int* newNodes = branchCalculationCPU(branch[1]->getBoard(), 1, remMoves);
			// Update T by new child nodes from node

			// Update T by generated child node c TODO: Revisar
			for (int j = 0; j < remMoves; j++) {
				int board[9];
				for (int k = 0; k < 9; k++) {
					board[k] = newNodes[j * 9 + k];
				}
				branch[1]->appendChild(new TreeNode(board));
			}
		}
		// Actualizar profundidad
		depth++;
		remLeaves = acumBranches;
		remBranches = acumBranches;
		acumLeaves = 0;
		acumBranches = 0;
	}
	return M_best;
}

int main() {
	int board[9] = {0, 1, 2, 0, 2, 1, 0, 0, 0};
	int board2[9] = { 1, 0, 0, 0, 2, 1, 0, 2, 0 };
	int moves = 5;
	TreeNode* test = new TreeNode(board);
	TreeNode* test2 = new TreeNode(board2);
	std::vector<TreeNode*> v;
	v.push_back(test);
	v.push_back(test2);
	int* x = branchCalculationGPUCall(v, moves);
	//for (int j = 0; j < moves * v.size(); j++) {
	//	for (int i = 0; i < 9; i++) {
	//		std::cout << x[j * 9 + i];
	//	}
	//	std::cout << "\n";
	//}

	int* y = branchCalculationCPU(board, 1, moves);
	//for (int j = 0; j < moves; j++) {
	//	for (int i = 0; i < 9; i++) {
	//		std::cout << y[9 * j + i];
	//	}
	//	std::cout << "\n";
	//}
	//int val = leafCalculationCPU(board, 1, 2);
	//std::cout << val;
	//std::cout << "\n";
	//std::cout << x[1];

	int* testGTS = gameTreeMax(test, 1, 2, 5);
	return 0;
}