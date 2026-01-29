/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Types
\{
*/

sealed class StaticArray
{
	private void StaticArray();
	private void ~StaticArray();

	/*!
	Sorts static array of integers(ascending) / floats(ascending) / strings(alphabetically) / pointers(by address ascending)
	\param paramArray Array to sort
	\code
		string	arrStr[3] = {"Dog", "Car", "Apple"};
		StaticArray.Sort(arrStr);
		Print(arrStr);
		>>  {'Apple','Car','Dog'}
	\endcode
	*/
	static proto void Sort(void paramArray[]);
	/*!
	\brief Revers order of static array elements
		@code
			int arr1[] = {1,2,3};
			Print(arr1);
			StaticArray.Reverse(arr1);
			Print(arr1);
			>>  {1,2,3}
			>>	{3,2,1}
		@endcode
	*/
	static proto void Reverse(void paramArray[]);
	/*!
	\brief Copy elements from one static array into another (of same type)
	\param destArray destination array
	\param srcArray source array
	\param dstOffset element offset in destination array
	\param srcOffset element offset in source array
	\param num number of elements to copy (-1 for all of them)
	\returns \p int number of elements copied
		@code
			int arr1[3] = {1,2,3};
			int arr2[3];
			int arr3[2];
			// copy all elements from arr1 to arr2
			StaticArray.Copy(arr2, arr1);

			// copy last two elements from arr1 at the beginning of arr3
			StaticArray.Copy(arr3, arr1, 0, 1, 2);

			Print(arr1);
			Print(arr2);
			Print(arr3);
			>>  {1,2,3}
			>>	{1,2,3}
			>>	{2,3}
		@endcode
	*/
	static proto int Copy(void destArray[], void srcArray[], int dstOffset = 0, int srcOffset = 0, int num = -1);
	//! Return length of static array
	static proto int Length(void paramArray[]);
}

/*!
\}
*/
