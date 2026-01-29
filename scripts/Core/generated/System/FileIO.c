/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

sealed class FileIO
{
	private void FileIO();
	private void ~FileIO();

	[Obsolete("use FileIO.FileExists")] static bool FileExist(string name) { return FileExists(name); }

	/*!
	Opens a File.
	\param name Name of a file to open. You can use filesystem prefixes. For accessing profile dir use '$profile', e.g. '$profile:myFileName.txt'.
	\param mode Constants FileMode.WRITE, FileMode.READ or FileMode.APPEND flag can be used
	\return file handle ID or 0 if fails
	*/
	static proto ref FileHandle OpenFile(string name, FileMode mode);
	/*!
	Creates a parser for a given `filename`
	It can be then used with `ParseLine` and `EndParse`.
	*/
	static proto ParseHandle BeginParse(string filename);
	//! Check existence of file
	static proto bool FileExists(string name);
	//! Makes a directory tree
	static proto bool MakeDirectory(string name);
	//! delete file. Works only on "$profile:", "$logs:" and "$saves:" locations
	static proto bool DeleteFile(string name);
	//! copy file. destName must be "$profile:", "$logs:" or "$saves:" location
	static proto bool CopyFile(string sourceName, string destName);
	/*!
	\brief Find files with extension in given path on all FileSystems accessible for game
	@code
		array<string> files = {};
		FileIO.FindFiles(files.Insert, "configs/", ".conf");
		files.Debug();
	@endcode
	*/
	static proto bool FindFiles(FindFilesCallback callback, string path, string ext);
}

/*!
\}
*/
