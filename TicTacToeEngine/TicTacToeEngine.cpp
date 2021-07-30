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

int leafCalculationCPU(int* board, int pindex, int eindex) {
	int V = 0;

	for (int i = 0; i < 8; i++) {
		const int* l = lines[i];
		// Recorrer el tablero y asignar puntajes
		int score = scores[(board[l[0]] == pindex) + (board[l[1]] == pindex) + (board[l[2]] == pindex)];
		score -= scores[(board[l[0]] == eindex) + (board[l[1]] == eindex) + (board[l[2]] == eindex)];
		V += score; 
	}
	return V;
}

std::pair<int*, bool*> branchCalculationCPU(int* board, int pindex, int moves) {
	int* Ml = new int[9 * moves];
	bool* B = new bool[moves];

	for (int i = 0; i < moves; i++) {
		B[i] = moves == 1;
	}

	int count = 0;
    for (int i = 0; i < 9; i++) {
		if (board[i] == 0) {

			for (int j = 0; j < 9; j++) {
				Ml[count * 9 + j] = board[j];
			}
			Ml[count * 9 + i] = pindex;

			for (int j = 0; j < 8; j++) {
				const int* l = lines[j];
				if (Ml[count * 9 + l[0]] == pindex && Ml[count * 9 + l[1]] == pindex && Ml[count * 9 + l[2]] == pindex) {
					B[count] = true;
					break;
				}
			}
			count++;
      }
    }
	return std::make_pair(Ml, B);
}


int* leafCalculationGPUCall(std::vector<TreeNode*> leaves, int pindex, int eindex) {
	// Call leafCalculationFunction on GPU with leaves
	int plSize = leaves.size();

	const int N_ELEMENTS = 1024 * 1024;
	int platform_id = 0, device_id = 0;

	std::vector<int> Pl;  // Or you can use simple dynamic arrays like: int* A = new int[N_ELEMENTS];
	int* V = new int[plSize];

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

std::pair<int*, bool*> branchCalculationGPUCall(std::vector<TreeNode*> branches, int moves, int pindex) {
	// Call leafCalculationFunction on GPU with leaves
	int plSize = branches.size();
	const int N_ELEMENTS = 1024 * 1024;
	int platform_id = 0, device_id = 0;

	std::vector<int> Pl;  // Or you can use simple dynamic arrays like: int* A = new int[N_ELEMENTS];
	int* M = new int[plSize * 9 * moves];
	bool* B = new bool[plSize * moves];

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
	cl::Buffer bufferMoves = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int)); // Estaba como Write Only
	cl::Buffer bufferM = cl::Buffer(context, CL_MEM_WRITE_ONLY, plSize * sizeof(int) * 9 * moves);
	cl::Buffer bufferB = cl::Buffer(context, CL_MEM_WRITE_ONLY, plSize * sizeof(bool) * moves);

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
	lckernel.setArg(2, bufferB);
	lckernel.setArg(3, bufferPIndex);
	lckernel.setArg(4, bufferMoves);

	// Execute the kernel
	cl::NDRange global(plSize);
	cl::NDRange local(1);

	queue.enqueueNDRangeKernel(lckernel, cl::NullRange, global, local);

	// Copy the output data back to the host
	queue.enqueueReadBuffer(bufferM, CL_TRUE, 0, plSize * sizeof(int) * 9 * moves, M);
	queue.enqueueReadBuffer(bufferB, CL_TRUE, 0, plSize * sizeof(bool) * moves, B);

	return std::make_pair(M, B);
}

void updateTree(std::vector<TreeNode*> evaluatedNodes, TreeNode** root, int* values, int* bestBoard) {
	for (int i = 0; i < evaluatedNodes.size(); i++) {
		int selectedValue = values[i];
		TreeNode* selectedNode = evaluatedNodes[i];
		TreeNode* parentNode = selectedNode->getParent();

		// Updating parent nodes till root
		while (parentNode != NULL) {
			int parentBest = parentNode->getValue();
			bool parentIsMax = parentNode->getIsMax();

			if (parentIsMax) {
				int best = std::max(parentBest, selectedValue);

				if (best > parentBest) {
					parentNode->setValue(best);

					if (parentNode == *root) {
						int* newBestBoard = selectedNode->getBoard();

						for (int i = 0; i < 9; i++) {
							bestBoard[i] = newBestBoard[i];
						}
					}
				}
			}
			else {
				int best = std::min(parentBest, selectedValue);

				if (best < parentBest) {
					parentNode->setValue(best);

					if (parentNode == *root) {
						int* newBestBoard = selectedNode->getBoard();

						for (int i = 0; i < 9; i++) {
							bestBoard[i] = newBestBoard[i];
						}
					}
				}

			}

			// Prune selected node from parent if has no children left
			if (!selectedNode->hasChildren()) {
				parentNode->removeChild(selectedNode);
			}
			// Set next parent node
			selectedNode = parentNode;
			selectedValue = selectedNode->getValue();
			parentNode = parentNode->getParent();
		}

		if (!(*root)->hasChildren()) {
			*root = NULL;
		}
	}
}

// Algoritmo 2
int* gameTreeMax(TreeNode *P0, int pindex, int eindex, int moves) {

	// Alpha-Beta Pruning
	//int alpha = MIN;
	//int beta = MAX;

	// Setting P0 as the root of the game tree T
	TreeNode *T = P0;
	int depth = 0;
	int* M_best = P0->getBoard();

	int remLeaves = 0; // remaining leaves
	int remBranches = 1; // remaining branches
	int acumLeaves = 0;
	int acumBranches = 0;

	int Lmin = 5;
	int Bmin = 5;

	while (T != NULL) {
		if (remLeaves > Lmin) {
			std::vector<TreeNode*> leaves = T->getLeaves(100000, depth); // Get l leaves from tree T at depth

			// Call leafCalculationFunction on GPU with leaves
			int* V = leafCalculationGPUCall(leaves, pindex, eindex);

			updateTree(leaves, &T, V, M_best);

			remLeaves -= leaves.size();
		}
		else if (remLeaves > 0) {
			std::vector<TreeNode*> leaf = T->getLeaves(1, depth); // Get one leaf node from tree T

			leaf[0]->setIsLeaf(false);

			if (leaf[0]->getIsLeaf()) {
				for (int i = 0; i < 9; i++) {
				}
			}
			
			// Call leafCalculationFunction on CPU
			int v = leafCalculationCPU(leaf[0]->getBoard(), pindex, eindex);

			// Update the tree T by the evaluated leaf node
			int V[1] = { v };

			updateTree(leaf, &T, V, M_best);

			remLeaves -= 1;
		}
		else if (remBranches > Bmin) {
			int remMoves = moves - depth;
			std::vector<TreeNode*> branches = T->getBranches(100000, depth);  // Get b branches from tree T

			for (TreeNode* b : branches) {
				b->setIsBranch(false);
			}
			
			// Call branchCalculationFunction in GPU
			std::pair<int*, bool*> childNodeList;

			if (depth % 2 == 0) {
				childNodeList = branchCalculationGPUCall(branches, remMoves, pindex);
			}
			else {
				childNodeList = branchCalculationGPUCall(branches, remMoves, eindex);
			}
			
			// Get childNodeList from the GPU
			int* boardList = childNodeList.first;
			bool* booleanList = childNodeList.second;

			// Update T by generated child node c
			for (int i = 0; i < branches.size(); i++) {
				for (int j = 0; j < remMoves; j++) {
					int* board = new int[9];

					for (int k = 0; k < 9; k++) {
						board[k] = boardList[(remMoves * i + j) * 9 + k];
					}

					bool isLeaf = booleanList[remMoves * i + j];

					if (depth % 2 == 0) {
						branches[i]->appendChild(new TreeNode(board, isLeaf, false));
					}
					else {
						branches[i]->appendChild(new TreeNode(board, isLeaf, true));
					}

					if (isLeaf) {
						acumLeaves += 1;
					}
					else {
						acumBranches += 1;
					}
				}
			}
			remBranches -= branches.size();
		}
		else if (remBranches > 0) {
			int remMoves = moves - depth;
			std::vector<TreeNode*> branch = T->getBranches(1, depth); // Get one branch node from tree T

			branch[0]->setIsBranch(false);

			// Call branchCalculationFunction on CPU
			std::pair<int*, bool*> newNodes;

			if (depth % 2 == 0) {
				newNodes = branchCalculationCPU(branch[0]->getBoard(), pindex, remMoves);
			}
			else {
				newNodes = branchCalculationCPU(branch[0]->getBoard(), eindex, remMoves);
			}

			// Update T by new child nodes from node
			int* boardList = newNodes.first;
			bool* booleanList = newNodes.second;

			// Update T by generated child node c
			for (int j = 0; j < remMoves; j++) {
				int* board = new int[9];

				for (int k = 0; k < 9; k++) {
					board[k] = boardList[j * 9 + k];
				}

				bool isLeaf = booleanList[j];

				if (depth % 2 == 0) {
					branch[0]->appendChild(new TreeNode(board, isLeaf, false));
				}
				else {
					branch[0]->appendChild(new TreeNode(board, isLeaf, true));
				}

				if (isLeaf) {
					acumLeaves += 1;
				}
				else {
					acumBranches += 1;
				}
			}
			remBranches -= 1;
		}
		else {
			// Actualizar profundidad
			depth++;
			remLeaves = acumLeaves;
			remBranches = acumBranches;
			acumLeaves = 0;
			acumBranches = 0;
		}
	}
	return M_best;
}

int main() {
	int moves = 8;
	int board[9] = { 0, 0, 0, 0, 1, 0, 0, 0, 0 };
	TreeNode* test = new TreeNode(board, false, true);
	int* testGTS = gameTreeMax(test, 2, 1, moves);

	//int* best = updateTree(eval, test, values);
	std::cout << "\njugada: ";
	for (int i = 0; i < 9; i++) {
		std::cout << testGTS[i];
	}
	std::cout << "\ntest-end";
	return 0;
}