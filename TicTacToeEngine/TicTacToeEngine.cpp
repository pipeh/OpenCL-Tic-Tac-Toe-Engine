#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <stdlib.h>
#include <chrono>
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

#define MAX 1000;
#define MIN -1000;

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


int* leafCalculationGPUCall(std::vector<TreeNode*> leaves, int pindex, int eindex, cl::Program program, cl::CommandQueue queue, cl::Context context) {
	// Call leafCalculationFunction on GPU with leaves
	int plSize = leaves.size();

	std::vector<int> Pl;
	int* V = new int[plSize];

	for (int i = 0; i < plSize; ++i) {
		int* board = leaves[i]->getBoard();
		for (int j = 0; j < 9; j++) {
			Pl.push_back(board[j]);
		}
	}

	// Create the memory buffers
	cl::Buffer bufferPl = cl::Buffer(context, CL_MEM_READ_ONLY, plSize * sizeof(int) * 9);
	cl::Buffer bufferPIndex = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
	cl::Buffer bufferEIndex = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int));
	cl::Buffer bufferV = cl::Buffer(context, CL_MEM_WRITE_ONLY, plSize * sizeof(int));

	// Copy the input data to the input buffers using the command queue.
	queue.enqueueWriteBuffer(bufferPl, CL_FALSE, 0, plSize * sizeof(int) * 9, &Pl[0]);
	queue.enqueueWriteBuffer(bufferPIndex, CL_FALSE, 0, sizeof(int), &pindex);
	queue.enqueueWriteBuffer(bufferEIndex, CL_FALSE, 0, sizeof(int), &eindex);

	// Make kernel
	cl::Kernel lckernel(program, "leafCalculation");

	// Set the kernel arguments
	lckernel.setArg(0, bufferPl);
	lckernel.setArg(1, bufferV);
	lckernel.setArg(2, bufferPIndex);
	lckernel.setArg(3, bufferEIndex);

	// Execute the kernel
	cl::NDRange global(131072);
	cl::NDRange local(256);

	queue.enqueueNDRangeKernel(lckernel, cl::NullRange, global, local);

	// Copy the output data back to the host
	queue.enqueueReadBuffer(bufferV, CL_TRUE, 0, plSize * sizeof(int), V);

	return V;
}

std::pair<int*, bool*> branchCalculationGPUCall(std::vector<TreeNode*> branches, int moves, int pindex, cl::Program program, cl::CommandQueue queue, cl::Context context) {
	// Call leafCalculationFunction on GPU with leaves
	int plSize = branches.size();

	std::vector<int> Pl;
	int* M = new int[plSize * 9 * moves];
	bool* B = new bool[plSize * moves];

	for (int i = 0; i < plSize; ++i) {
		int* board = branches[i]->getBoard();
		for (int j = 0; j < 9; j++) {
			Pl.push_back(board[j]);
		}
	}

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

	// Make kernel
	cl::Kernel lckernel(program, "branchCalculation");

	// Set the kernel arguments
	lckernel.setArg(0, bufferPl);
	lckernel.setArg(1, bufferM);
	lckernel.setArg(2, bufferB);
	lckernel.setArg(3, bufferPIndex);
	lckernel.setArg(4, bufferMoves);

	// Execute the kernel
	cl::NDRange global(131072);
	cl::NDRange local(256);

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

int* gameTreeMax(TreeNode *P0, int pindex, int eindex, int moves) {
	// Setting P0 as the root of the game tree T
	TreeNode *T = P0;
	int depth = 0;
	int* M_best = P0->getBoard();

	int remLeaves = 0; // remaining leaves
	int remBranches = 1; // remaining branches
	int acumLeaves = 0;
	int acumBranches = 0;

	int Lmin = 50;
	int Bmin = 50;
	int l = 131072;
	int b = 131072;
	
	// Initialize GPU Kernel
	int platform_id = 0, device_id = 0;

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

	// Read the program source
	std::ifstream sourceFile("mykernel.cl");
	std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()));

	// Make program from the source code
	cl::Program program = cl::Program(context, source);

	program.build(devices);

	while (T != NULL) {
		if (remLeaves > Lmin) {
			std::vector<TreeNode*> leaves = T->getLeaves(l, depth); // Get l leaves from tree T at depth

			// Call leafCalculationFunction on GPU with leaves
			int* V = leafCalculationGPUCall(leaves, pindex, eindex, program, queue, context);

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
			std::vector<TreeNode*> branches = T->getBranches(b, depth);  // Get b branches from tree T

			for (TreeNode* b : branches) {
				b->setIsBranch(false);
			}
			
			// Call branchCalculationFunction in GPU
			std::pair<int*, bool*> childNodeList;

			if (depth % 2 == 0) {
				childNodeList = branchCalculationGPUCall(branches, remMoves, pindex, program, queue, context);
			}
			else {
				childNodeList = branchCalculationGPUCall(branches, remMoves, eindex, program, queue, context);
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
			// Update depth
			depth++;
			remLeaves = acumLeaves;
			remBranches = acumBranches;
			acumLeaves = 0;
			acumBranches = 0;
		}
	}
	return M_best;
}

void printBoard(int *board) {
	std::cout << "\nTablero: \n";
	std::cout << "\n-------\n";
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			int s = board[i * 3 + j];
			if (s == 1) {
				std::cout << "|X";
			}
			else if (s == 2) {
				std::cout << "|O";
			}
			else {
				std::cout << "| ";
			}
		}
		std::cout << "|\n-------\n";
	}
}

int hasWon(int *board) {
	for (int i = 0; i < 8; i++) {
		const int *l = lines[i];

		if (board[l[0]] == board[l[1]] && board[l[1]] == board[l[2]]) {
			return board[l[0]];
		}
	}

	return 0;
}

void startGame() {
	int moves = 9;
	int* board = new int[9];
	for (int i = 0; i < 9; i++) {
		board[i] = 0;
	}
	int pindex;
	int eindex;
	int winner = 0;
	
	std::srand(std::time(0));
	bool turn = true;

	std::cout << "Selecciona una opcion:\nX (1)\nO (2)\nOpcion: ";

	int choice;
	std::cin >> choice;

	if (choice == 1) {
		pindex = 1;
		eindex = 2;
	}
	else {
		pindex = 2;
		eindex = 1;
	}

	while (moves > 0) {
		printBoard(board);

		if (turn) {
			std::cout << "\nIngrese una jugada: ";
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
			int m;
			std::cin >> m;
			board[m - 1] = pindex;
		}
		else {
			TreeNode* root = new TreeNode(board, false, true);
			board = gameTreeMax(root, eindex, pindex, moves);
		}
		moves--;
		turn = !turn;

		winner = hasWon(board);

		if (winner) {
			break;
		}
	}

	printBoard(board);

	if (winner) {
		if (winner == pindex) {
			std::cout << "\nHas ganado!\n";
		}
		else {
			std::cout << "\nHas perdido!\n";
		}
	}
	else {
		std::cout << "\nEmpate!\n";
	}
}

std::pair<int*, int> sequentialMinimax(TreeNode* node, int moves, int pindexEv, int eindexEv, int pindex, int eindex) {
	int* board = node->getBoard();

	if (node->getIsLeaf()) {
		return std::make_pair(board, leafCalculationCPU(board, pindexEv, eindexEv));
	}

	std::pair<int*, bool*> newNodes = branchCalculationCPU(board, pindex, moves);

	int* boards = newNodes.first;
	bool* leaves = newNodes.second;

	if (node->getIsMax()) {
		int bestVal = MIN;
		int* bestBoard = board;

		for (int i = 0; i < moves; i++) {
			int* newBoard = new int[9];
			for (int j = 0; j < 9; j++) {
				newBoard[j] = boards[i * 9 + j];
			}
			TreeNode* newNode = new TreeNode(newBoard, leaves[i], false);
			std::pair<int*, int> p = sequentialMinimax(newNode, moves - 1, pindexEv, eindexEv, eindex, pindex);
			
			if (p.second > bestVal) {
				bestBoard = newBoard;
				bestVal = p.second;
			}
		}
		return std::make_pair(bestBoard, bestVal);
	}
	else {
		int bestVal = MAX;
		int* bestBoard = board;

		for (int i = 0; i < moves; i++) {
			int* newBoard = new int[9];
			for (int j = 0; j < 9; j++) {
				newBoard[j] = boards[i * 9 + j];
			}
			TreeNode* newNode = new TreeNode(newBoard, leaves[i], true);
			std::pair<int*, int> p = sequentialMinimax(newNode, moves - 1, pindexEv, eindexEv, eindex, pindex);
			
			if (p.second < bestVal) {
				bestBoard = newBoard;
				bestVal = p.second;
			}
		}
		return std::make_pair(bestBoard, bestVal);
	}
}

int main() {
	startGame();
	return 0;
}
