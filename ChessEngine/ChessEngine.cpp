#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <stdlib.h>

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

//Tratando de crear una arbol con arrays

// Idea 1

	//				  [4]
	//				/  |  \
	//			[2]   [3]  [5]
	//		    / \	   |	|
	// 		  [1] [6] [7]  [0]

	// count = 0

	// agregar root
	// [ [4, [] ] ] count = 1
	
	// agregar hijos
	// [ [4, [1] ], [2, [] ] ] count = 2
	// [ [4, [1, 2] ], [2, [] ], [3, [] ] ] count = 3
	// [ [4, [1, 2, 3] ], [2, [] ], [3, [] ], [5, [] ] ] count = 4

	// agregarle los hijos a cada hijo

	// hijo 1
	// [ [4, [1, 2, 3] ], [2, [4] ], [3, [] ], [5, [] ], [1, [] ] ] count = 5
	// [ [4, [1, 2, 3] ], [2, [4, 5] ], [3, [] ], [5, [] ], [1, [] ], [6, [] ] ] count = 6

	// hijo 2
	// [ [4, [1, 2, 3] ], [2, [4, 5] ], [3, [6] ], [5, [] ], [1, [] ], [6, [] ], [7, [] ] ] count = 7

	// hijo 3
	// [ [4, [1, 2, 3] ], [2, [4, 5] ], [3, [6] ], [5, [7] ], [1, [] ], [6, [] ], [7, [] ], [0, [] ] ] count = 8

	//agregarle los hijos a cada hjo
	// [ [4, [1, 2, 3] ], [2, [4, 5] ], [3, [6] ], [5, [7] ], [1, Null ], [6, [] ], [7, [] ], [0, [] ] ] count = 8
	// [ [4, [1, 2, 3] ], [2, [4, 5] ], [3, [6] ], [5, [7] ], [1, Null ], [6, Null ], [7, [] ], [0, [] ] ] count = 8
	// [ [4, [1, 2, 3] ], [2, [4, 5] ], [3, [6] ], [5, [7] ], [1, Null ], [6, Null ], [7, Null ], [0, [] ] ] count = 8
	// [ [4, [1, 2, 3] ], [2, [4, 5] ], [3, [6] ], [5, [7] ], [1, Null ], [6, Null ], [7, Null ], [0, Null ] ] count = 8

	// Para accedeer a los hijos se revisa en el nodo que posicion dice que tiene el hijo y arraytree[pos]

	//fin :)

	int* TreeToList(int* tree){
		int* arraytree = [];											// initial arraytree
		int count = 0;
		count = AddNode(count, tree, arraytree);  						// add root
 		AddChilds(count, tree, *arraytree); 							// add root childs
		return arraytree;
	}

	int* AddChilds(int count, int* tree, int* arraytree){
		if(tree.childs == Null){										// if node doest have childs 
			*arraytree[1] = Null; 										// the child positions in the node are null
			return;
		}
		for(int i = 0; i < tree.childs.len(); i++){ 					// for each child
			*arraytree[1] += [count]; 									// save the position where are you going to put it
			count = AddNode(int count, int* tree, int* arraytree); 		// add the node, increasing de counting for the next one
		}
		for(int i = 0; i < tree.childs.len(); i++){						// for each already added child
			AddChilds(int count, int* tree, int* arraytree);			// add its childs
		}
	}

	int AddNode(int count, int* tree, int* arraytree){
		*arraytree = [tree.value, []];									// add the node with empty child positions
		count++;														// increase the counter
		arraytree++;													// move to the next position
		return count;													// return the new position
	}

// Idea 2

	//				  [4]
	//				/  |  \
	//			[2]   [3]  [5]
	//		    / \	   |	|
	// 		  [1] [6] [7]  [0]

	// count = 0

	// agregar root, tiene 3 hijos
	// [(4,3)] count = 1

	// agregar hijos 
	// [(4,3), (2,2), (3,1), (5,1)]  count = 4

	// hijo 1, tiene dos hijos, sus indices serán count +1 y count +2
	// [(4,3), (2,2), (3,1), (5,1), (1,0), (6,0)]  count = 6

	// hijo 2
	// [(4,3), (2,2), (3,1), (5,1), (1,0), (6,0), (7,0)]  count = 7

	// hijo 3
	// [(4,3), (2,2), (3,1), (5,1), (1,0), (6,0), (7,0), (0,0)]  count = 8

	// Para acceder en este caso hay que ir recorriendo la lista y sumando cuantos hijos hay 

	// [(4,3), (2,2), (3,1), (5,1), (1,0), (6,0), (7,0), (0,0)]  count = 0
	// quiero acceder al primer hijo de la raiz count += 1
	// accedo a arraytree[count]
	// ahora quiero ir a el segundo hijo de este nodo, cuantos hijos tiene -> empiezo count del principio 
	// count = 0
	// count += 3 -> para pasar a los hijos de los hijos 
	// count += 2 -> segundo hijo del primer hijo 
	// arraytree[count] es la posicion del hijo 


	int* TreeToList(int* tree){
		int* arraytree = [];											// initial arraytree
		AddNode(tree, arraytree);  										// add root
 		AddChilds(tree, *arraytree); 									// add root childs
		return arraytree;
	}

	int* AddChilds(int* tree, int* arraytree){
		if(tree == Null){												// the previous one was a leaf
			return;
		}
		for(int i = 0; i < tree.childs.len(); i++){ 					// for each child
			AddNode(int* tree, int* arraytree); 						// add the node
		}
		for(int i = 0; i < tree.childs.len(); i++){						// for each already added child
			AddChilds(int* tree, int* arraytree);						// add its childs
		}
	}

	int AddNode(int* tree, int* arraytree){
		*arraytree = (tree.value, tree.childs.len());					// add the node the amount of childs
		arraytree++;													// move to the next position
		return;											
	}

// Idea 3

	//				  [4]
	//				/  |  \
	//			[2]   [3]  [5]
	//		    / \	   |	|
	// 		  [1] [6] [7]  [0]

	// count = 0

	// agregar root
	// [(4,Null)] count = 1
	
	// agregar hijos
	// [ (4, 1), (2, Null) ] count = 2
	// [ (4, 1), (2, Null), (3, Null) ] count = 3
	// [ (4, 1), (2, Null), (3, Null), (5, Null) ] count = 4

	// agregarle los hijos a cada hijo

	// hijo 1
	// [ (4, 1), (2, 4), (3, Null), (5, Null), (1, Null) ] count = 5
	// [ (4, 1), (2, 4), (3, Null), (5, Null), (1, Null), (6, Null) ] count = 6

	// hijo 2
	// [ (4, 1), (2, 4), (3, 6), (5, Null), (1, Null), (6, Null), (7, Null) ] count = 7

	// hijo 3
	// [ (4, 1), (2, 4), (3, 6), (5, 7), (1, Null), (6, Null), (7, Null), (0, Null) ] count = 8

	// Para accedeer a los hijos se revisa en el nodo que posicion dice que tiene el primer hijo y arraytree[pos]
	// Para segudo hijo será pos +1 y así

	int* TreeToList(int* tree){
		int* arraytree = [];											// initial arraytree
		int count = 0;
		count = AddNode(count, tree, arraytree);  						// add root
 		AddChilds(count, tree, *arraytree); 							// add root childs
		return arraytree;
	}

	int* AddChilds(int count, int* tree, int* arraytree){
		if(tree.childs == Null){										// if node doest have childs it ends
			return;
		}
		*arraytree[1] = count; 											// if have childs, save the position where are you going to put the first one

		for(int i = 0; i < tree.childs.len(); i++){ 					// for each child
			count = AddNode(int count, int* tree, int* arraytree); 		// add the node, increasing de counting for the next one
		}
		for(int i = 0; i < tree.childs.len(); i++){						// for each already added child
			AddChilds(int count, int* tree, int* arraytree);			// add its childs
		}
	}

	int AddNode(int count, int* tree, int* arraytree){
		*arraytree = [tree.value, Null];								// add the node with empty child positions
		count++;														// increase the counter
		arraytree++;													// move to the next position
		return count;													// return the new position
	}

	//Fin de tratando 

int gameTreeMax(int* P0) {

	// Setting P0 as the root of the game tree T
	int* T = P0;
	int M_best = 0;

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
			int leaves [l]; // Get l leaves from tree T
			
			// Call leafCalculationFunction on GPU with leaves
			
			// Get evaluatedValueList from GPU

			float evaluatedValueList [l];

			for (float v : evaluatedValueList) {
				// Update parent node in T
				int* parentNode;

				// If parent node is root, set M_best to value
				if (parentNode == P0) {
					M_best = v;
				}
				// Pruning
			}
		}
		else if (remLeaves > 0) {
			int leaf = 0; // Get one leaf node from tree T
			
			// Call leafCalculationFunction on CPU
			// Update the tree T by the evaluated leaf node
			int v = 0; // int v = leafCalculationFunction(leaf);
			int* parentNode;
			if (parentNode == P0) {
					M_best = v;
				}
			// Pruning
		}
		else if (remBranches > Bmin) {
			int branches [b];  // Get b branches from tree T
			// Call branchCalculationFunction in GPU
			// Get childNodeList from the GPU
			int childNodeList [5];  // Could be any size
			for (int c : childNodeList) {
				// Update T by generated child node c
			}
		}
		else if (remBranches > 0) {
			int leaf = 0; // Get one leaf node from tree T
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
