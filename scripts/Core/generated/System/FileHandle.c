/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

/*!
@code
// binary file write
FileHandle fileW = FileIO.OpenFile("$profile:file.bin", FileMode.WRITE);
if (fileW)
{
	int valInt = 4679;
	float valFloat = 87.79;
	string valStr = "Hello";
	fileW.Write(valInt);
	fileW.Write(valFloat);
	fileW.Write(valStr);
	fileW.Close();
}

// binary file read
FileHandle fileR = FileIO.OpenFile("$profile:file.bin", FileMode.READ);
if (fileR)
{
	int valInt;
	float valFloat;
	string valStr;
	fileR.Read(valInt, 4);
	fileR.Read(valFloat, 4);
	fileR.Read(valStr, 5);
	fileR.Close();

	Print(valInt);
	Print(valFloat);
	Print(valStr);
}

// text file write
FileHandle textFileW = FileIO.OpenFile("$profile:file.txt", FileMode.WRITE);
if (textFileW)
{
	for (int i = 0; i < 10; i++)
	{
		textFileW.WriteLine("Line " + i);
	}
	textFileW.Close();
}

// text file read
FileHandle textFileR = FileIO.OpenFile("$profile:file.txt", FileMode.READ);
if (textFileR)
{
	string line;
	while(textFileR.ReadLine(line) >= 0)
	{
		Print(line);
	}
	textFileR.Close();
}
@endcode
*/
sealed class FileHandle: Managed
{
	private void FileHandle();

	/*!
	Read raw data.
	\param data supported types: int, float, string
	\param length number of bytes to read. For int type is clamped [0,4]. For float its not used (always 4). For string it's not limited.
	\return the total number of bytes successfully read.
	*/
	proto int Read(out void data, int length);
	/*!
	Get line from file, every next call of this function returns next line.
	\return Count of characters or -1 if there is nothing to read
	*/
	proto int ReadLine(out string data);
	/*!
	Read raw data.
	\param dataArray supported types: array<int>, array<float> (array content will be overwritten)
	\param elementLength number of bytes from each array element to read (e.g. for reading array of bytes use 1, for shorts use 2, for int-s use 4). For int type its clamped [0,4] For float its not used (always 4).
	\param numElements number of array elements to read.
	\return the total number of bytes successfully read.
	*/
	proto int ReadArray(out notnull Managed dataArray, int elementLength, int numElements);
	/*!
	Write raw data.
	\param data supported types: int, float, string
	\param length number of bytes to write. For int type its clamped [0,4] For float its not used. For string its clamped [0, size of string]. If -1 value is used, the whole content of data is written.
	\return the total number of bytes successfully written.
	*/
	proto int Write(void data, int length = -1);
	/*!
	Write to file and add newline (CARRIAGE RETURN + LINE FEED).
	\param data Value to write
	*/
	proto void WriteLine(string data);
	/*!
	Write raw data.
	\param dataArray supported types: array<int>, array<float>
	\param elementLength number of bytes from each array element to write. For int type its clamped [0,4] For float its not used (always 4).
	\param numElements number of array elements to write. If -1 value is used, all elements from array are written.
	\return the total number of bytes successfully written.
	*/
	proto int WriteArray(out notnull Managed dataArray, int elementLength = 4, int numElements = -1);
	/*!
	Set current position in file.
	\param pos offset from the file beginning
	*/
	proto void Seek(int pos);
	/*!
	Get current position in file.
	\return offset from the file beginning
	*/
	proto int GetPos();
	/*!
	Get file size.
	\return file size in bytes
	*/
	proto int GetLength();
	//! Close the File.
	proto void Close();
	proto bool IsOpen();
	//! Indicate that the End-of-File has been reached
	proto bool IsEOF();
}

/*!
\}
*/
