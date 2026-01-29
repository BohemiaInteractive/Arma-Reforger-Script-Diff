/*
Base class for compare ooepartion for sorting.

Example:

class SCR_TestSortObject_CompareCost : SCR_SortCompare<SCR_TestSortObject>
{
	override int Compare(SCR_TestSortObject left, SCR_TestSortObject right)
	{
		return left.cost < right.cost;
	}
};
*/

class SCR_SortCompare<Class T>
{
	//! Must return result of comparison operation.
	//! For ascending order results with invert = false, "left < right" operation must be performed.
	//! It must return exactly 1 or 0! 
	static int Compare(T left, T right);
};


/*
Collection of functions related to sorting.
*/
class SCR_Sorting<Class T, SCR_SortCompare TCompare>
{
	//---------------------------------------------------------------------------------------------------
	//! Heapsort implementation
	//! Example usage: SCR_Sorting<MyClass>.HeapSort(myArray, myCompare, false);
	static void HeapSort(array<T> a, bool inverse = false)
	{
		// Heapify the array
		Heap_Init(a, inverse);
		
		int endId = a.Count() - 1;
		
		while (endId > 0)
		{
			// Root is max now, swap it with last element
			T temp = a[endId];
			a[endId] = a[0];
			a[0] = temp;
			
			endId--;
			
			Heap_SiftDown(a, 0, endId, inverse);
		}	
	}
	
	
	
	
	
	//---------------------------------------------------------------------------------------------------
	// Help functions for heap sorting
	//---------------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------------
	static int Heap_IdParent(int nodeId)
	{
		// floor ( (nodeId - 1) / 2 )
		return (nodeId - 1) >> 1;
	}
	
	//---------------------------------------------------------------------------------------------------
	static int Heap_IdLeftChild(int nodeId)
	{
		return 2*nodeId + 1;
	}
	
	
	//---------------------------------------------------------------------------------------------------
	static void Heap_SiftDown(array<T> a, int startId, int endId, bool inverse)
	{
		int rootId = startId;
		
		// 1 when inverse, 0 otherwise. We do XOR operation with it to invert compare result.
		int invertMask = inverse;
		
		while (Heap_IdLeftChild(rootId) <= endId)
		{
			int leftChildId = Heap_IdLeftChild(rootId);
			int rightChildId = leftChildId + 1;
			int maxId = rootId;
			
			// Find which of the three (root, left child, right child) is max
			if (TCompare.Compare(a[maxId], a[leftChildId]) ^ invertMask)
				maxId = leftChildId;
			if (rightChildId <= endId)
			{
				if (TCompare.Compare(a[maxId], a[rightChildId]) ^ invertMask)
					maxId = rightChildId;
			}
			
			if (maxId == rootId)
				return;	// Root is max, and same for children relative to their children, ...
			else
			{
				// Swap root with the max element,
				// Evaluate the swapped child's tree
				T temp = a[maxId];
				a[maxId] = a[rootId];
				a[rootId] = temp;
				rootId = maxId;
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Creates a heap in provided array
	static void Heap_Init(array<T> a, bool inverse)
	{
		int count = a.Count();
		
		int startId = Heap_IdParent(count - 1);
		
		while (startId >= 0)
		{
			Heap_SiftDown(a, startId, count - 1, inverse);
			startId--;
		}
	}
};