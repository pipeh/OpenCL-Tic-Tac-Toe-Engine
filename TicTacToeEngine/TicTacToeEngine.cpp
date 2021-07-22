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

/*
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

*/
std::vector<int*> leafCalculationGPUCall(std::vector<TreeNode*> leaves) {
	// Call leafCalculationFunction on GPU with leaves
	int plSize = leaves.size();

	const int N_ELEMENTS = 1024 * 1024;
	int platform_id = 0, device_id = 0;

	std::vector<int*> Pl;  // Or you can use simple dynamic arrays like: int* A = new int[N_ELEMENTS];
	std::vector<int*> V;
	int pindex = 1;
	int eindex = 0;

	for (int i = 0; i < plSize; ++i) {
		Pl.push_back(leaves[i]->getBoard());
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
	std::cout << plSize;

	// Read the program source
	std::ifstream sourceFile("mykernel.cl");
	std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()));

	// Make program from the source code
	cl::Program program = cl::Program(context, source);

	// Build the program for the devices
	program.build(devices);
	std::cout << "tast";
	// Make kernel
	cl::Kernel lckernel(program, "leafCalculation");
	std::cout << "test";

	// Set the kernel arguments
	lckernel.setArg(0, bufferPl);
	lckernel.setArg(1, bufferV);
	lckernel.setArg(2, bufferPIndex);
	lckernel.setArg(3, bufferEIndex);

	// Execute the kernel
	cl::NDRange global(plSize);
	cl::NDRange local(256);
	queue.enqueueNDRangeKernel(lckernel, cl::NullRange, global, local);

	// Copy the output data back to the host
	queue.enqueueReadBuffer(bufferV, CL_TRUE, 0, plSize * sizeof(int), &V[0]);
	std::cout << "tost";
	return V;
}

/*
// Algoritmo 2
int gameTreeMax(TreeNode P0, int pindex, int eindex) {

	// Alpha-Beta Pruning
	alpha = MIN;
	beta = MAX;
	depth = 0;  // O que cada nodo guarde su altura.
	int M_best[9];

	// Setting P0 as the root of the game tree T
	TreeNode T = P0;

	int Lmin = 3;
	int Bmin = 3;
	int Lmax = 6;
	int Bmax = 6;
	int l = 0;
	int b = 0;

	int remLeaves = 0; // remaining leaves
	int remBranches = 1; // remaining branches

	bool maximizingPlayer = true;  // O puede depender de la altura a la que se haga el cálculo. depth % 2 == 0 -> Max || == 1 -> Min

	while (T != NULL) {
		if (remLeaves > Lmin) {
			TreeNode* leaves = T.getChilds(); // Get l leaves from tree T
			
			// Call leafCalculationFunction on GPU with leaves
			std::unique_ptr<int[]> V = leafCalculationGPUCall(leaves); // Get evaluatedValueList from GPU: V

			// Quizás aquí debería haber un for por cada hoja analizada
			for (int i = 0; i < V.length(); i++) {
				// Get value, parentNode, and depth
				int v = V[i];
				TreeNode parentNode = leaves[i].getParent();
				int depth = parentNode.getDepth();

				if (depth % 2 == 0) {
					// Maximizing player
					int best = max(best, v);
					alpha = max(alpha, best);

					// Alpha Beta Pruning
					if (beta <= alpha) {
						break;
					}
				} else {
					// Minimizing player
					int best = min(best, v);
					beta = min(beta, best);

					// Alpha Beta Pruning
					if (beta <= alpha) {
						break;
					}
				}

				// If parent node is NULL, set M_best to movement
				if (parentNode == P0) {
					best = v;
				}
				// Pruning
				parentNode.removeChild(i);
			}
		}

		else if (remLeaves > 0) {
			TreeNode leaf = T.getChild(0); // Get one leaf node from tree T
			
			// Call leafCalculationFunction on CPU
			int v = leafCalculationFunction(leaf);
			// Update the tree T by the evaluated leaf node
			TreeNode parentNode = T;
			int depth = parentNode.getDepth();

			if (depth % 2 == 0) {
				// Maximizing player
				int best = max(best, v);
				alpha = max(alpha, best);

				// Alpha Beta Pruning
				if (beta <= alpha) {
					break;
				}
			} else {
				// Minimizing player
				int best = min(best, v);
				beta = min(beta, best);

				// Alpha Beta Pruning
				if (beta <= alpha) {
					break;
				}
			}

			if (parentNode == P0) {
				best = v;
			}
			// Pruning
			parentNode.removeChild(0);
		}

		else if (remBranches > Bmin) {
			TreeNode* branches = T.getBranches(b);  // Get b branches from tree T
			// Call branchCalculationFunction in GPU

			// Get childNodeList from the GPU
			int*** childNodeList;  // Could be any size

			for (int i = 0; i < childNodeList.length(); i++) {
				for (int* c : childNodeList[i]) { // Cada c es una board
					// Update T by generated child node c
					branches[i].appendChild(new TreeNode(c));
				}
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
*/

int main() {
	int board[9] = {0, 1, 2, 0, 2, 1, 0, 0, 0};
	TreeNode* test = new TreeNode(board);
	std::vector<TreeNode*> v;
	v.push_back(test);
	std::vector<int*> x = leafCalculationGPUCall(v);

	std::cout << "test";
	return 0;
}