class SCR_FileIOHelper
{
	protected static const string PATH_DELIMITER = SCR_StringHelper.SLASH; // accepted under Windows too
	protected static const string FORBIDDEN_FILENAME_CHARS_WINDOWS = "*/\\<>:|?\t\r\n\"";
	protected static const string FORBIDDEN_FILENAME_CHARS_LINUX = SCR_StringHelper.SLASH;

	protected static const ref array<ref SCR_FileInfo> FOUND_FILEINFOS = {};

	//------------------------------------------------------------------------------------------------
	//! OBSOLETE - use FileIO.MakeDirectory instead\n
	//! Create sub-directories in the proper order, circumventing a FileIO.MakeDirectory limitation
	//! \param[in] absoluteDirectory e.g C:/Arma4/Data/scripts/My/Sub/Directory
	//! \return true if the whole directory structure was created or already exists, false otherwise
	[Obsolete("Use FileIO.MakeDirectory instead")]
	static bool CreateDirectory(string absoluteDirectory)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(absoluteDirectory))
			return false;

		if (FileIO.FileExists(absoluteDirectory))
			return true;

		absoluteDirectory.Replace("\\", PATH_DELIMITER);
		absoluteDirectory = SCR_StringHelper.ReplaceRecursive(absoluteDirectory, PATH_DELIMITER + PATH_DELIMITER, PATH_DELIMITER);

		if (FileIO.FileExists(absoluteDirectory))
			return true;

		array<string> pieces = {};
		absoluteDirectory.Split(PATH_DELIMITER, pieces, true);

		if (pieces.Count() < 2)
		{
			Print("Cannot create directory " + absoluteDirectory + "; not enough directories", LogLevel.DEBUG);
			return false;
		}

		string path = pieces[0]; // C: part on Windows
		for (int i = 1, count = pieces.Count(); i < count; i++)
		{
			path += PATH_DELIMITER + pieces[i];

			if (FileIO.FileExists(path))
				continue;

			if (!FileIO.MakeDirectory(path))
			{
				Print("Cannot create directory " + absoluteDirectory + "; blocked at " + path, LogLevel.DEBUG);
				return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] directoryPath
	//! \param[in] extension
	//! \return
	static array<string> FindFiles(string directoryPath, string extension)
	{
		array<string> result = {};
		FileIO.FindFiles(result.Insert, directoryPath, extension);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Copy source file to destination
	//! \param[in] source file to copy
	//! \param[in] destination copy destination
	//! \param[in] overwrite set to false to prevent an accidental overwrite
	//! \return true on success, false otherwise
	[Obsolete("Use CopyFile instead")] // 2025-05-14
	static bool Copy(string source, string destination, bool overwrite = true)
	{
		array<string> content = ReadFileContent(source);
		if (!content)
		{
			Print("Cannot get file content - " + source, LogLevel.ERROR);
			return false;
		}

#ifdef WORKBENCH
		string absolutePath;
		if (!FilePath.IsAbsolutePath(destination) && !Workbench.GetAbsolutePath(destination, absolutePath, false))
		{
			Print("Cannot get destination's absolute file path - " + destination, LogLevel.WARNING);
			return false;
		}
#endif // WORKBENCH

		if (!overwrite && FileIO.FileExists(destination))
		{
			Print("Not allowed to overwrite file - " + destination, LogLevel.WARNING);
			return false;
		}

		if (!WriteFileContent(destination, content))
		{
			Print("Cannot write file - " + destination, LogLevel.ERROR);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Copy source file to destination - creating the destination directory if needed
	//! \param[in] sourceFile
	//! \param[in] destinationFile
	//! \return true on success, false otherwise
	static bool CopyFile(string sourceFile, string destinationFile)
	{
		if (!FileIO.FileExists(sourceFile))
			return false;

		if (FileIO.CopyFile(sourceFile, destinationFile))
			return true;

		if (!FileIO.MakeDirectory(FilePath.StripFileName(destinationFile)))
			return false;

		return FileIO.CopyFile(sourceFile, destinationFile);
	}

	//------------------------------------------------------------------------------------------------
	//! Recursively copy a directory to another, existing or not. Overwrite is allowed so check before copying
	//! \param[in] sourceDirectory can be relative or absolute
	//! \param[in] destinationDirectory can be relative or absolute
	//! \return true on success, false otherwise - it will (try to) copy all files, not halt on the first error
	static bool CopyDirectory(string sourceDirectory, string destinationDirectory)
	{
#ifdef WORKBENCH
		if (!Workbench.GetAbsolutePath(sourceDirectory, sourceDirectory, true))
		{
			Print("Wrong sourceDirectory provided", LogLevel.WARNING);
			return false;
		}

		if (!Workbench.GetAbsolutePath(destinationDirectory, destinationDirectory, false))
		{
			Print("Wrong destinationDirectory provided", LogLevel.WARNING);
			return false;
		}
#else // WORKBENCH
		if (!FileIO.FileExists(sourceDirectory))
		{
			Print("Wrong sourceDirectory provided", LogLevel.WARNING);
			return false;
		}

		if (!FileIO.FileExists(destinationDirectory))
		{
			Print("Wrong destinationDirectory provided", LogLevel.WARNING);
			return false;
		}
#endif // WORKBENCH

		array<ref SCR_FileInfo> directoryContent = GetDirectoryContent(sourceDirectory);
		if (!directoryContent)
		{
			Print("Cannot get files from directory " + sourceDirectory, LogLevel.WARNING);
			return false;
		}

		if (directoryContent.IsEmpty())
		{
			if (FileIO.FileExists(destinationDirectory))
				return true;

			return FileIO.MakeDirectory(destinationDirectory);
		}

		if (!FileIO.MakeDirectory(destinationDirectory))
		{
			Print("Cannot create " + destinationDirectory, LogLevel.ERROR);
			return false;
		}

		bool result = true;

		int sourceDirectoryLength = sourceDirectory.Length();
		array<string> isDirCheck = {};
		foreach (SCR_FileInfo fileInfo : directoryContent)
		{
			string newAbsPath = destinationDirectory + fileInfo.m_sFilePath.Substring(sourceDirectoryLength, fileInfo.m_sFilePath.Length() - sourceDirectoryLength);

			if ((fileInfo.m_eAttributes | FileAttribute.DIRECTORY) == fileInfo.m_eAttributes)
			{
				if (!FileIO.MakeDirectory(fileInfo.m_sFilePath)) // returns true if the directory exists
				{
					PrintFormat("Cannot create directory %1", fileInfo.m_sFilePath, level: LogLevel.ERROR);
					result = false;
				}

				continue;
			}

			// it is a file

			string newAbsFileDir = FilePath.StripFileName(newAbsPath);
			if (!FileIO.MakeDirectory(newAbsFileDir)) // returns true if the directory exists
			{
				PrintFormat("Cannot create directory %1", newAbsFileDir, level: LogLevel.ERROR);
				result = false;
			}

			if (!FileIO.CopyFile(fileInfo.m_sFilePath, newAbsPath)) // CopyFile requires the directory to exist
			{
				PrintFormat("Cannot copy '%1' to '%2'", fileInfo.m_sFilePath, newAbsPath, level: LogLevel.ERROR);
				result = false;
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Fake a renaming by copying then deleting the source file
	//! \param[in] sourceFile
	//! \param[in] destinationFile
	//! \return true on success, false on failure (no
	static bool RenameFile(string sourceFile, string destinationFile)
	{
		return FileIO.CopyFile(sourceFile, destinationFile) && FileIO.DeleteFile(sourceFile);
	}

	//------------------------------------------------------------------------------------------------
	//! Get file content as one big string
	//! \param[in] filePath relative or absolute
	//! \param[in] printWarning true to print warning on issue, false otherwise
	//! \return raw string file content or empty string if file does not exist or cannot be opened
	static string GetFileStringContent(string filePath, bool printWarning = true)
	{
		FileHandle fileHandle = FileIO.OpenFile(filePath, FileMode.READ);
		if (!fileHandle)
		{
			if (printWarning)
				Print("Cannot open/read " + filePath, LogLevel.WARNING);

			return string.Empty;
		}

		string result;
		fileHandle.Read(result, -1);
		fileHandle.Close();

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] directory the directory to browse
	//! \param[in] extension the extension of files for which to look
	//! \return array of file information or null on error
	static array<ref SCR_FileInfo> GetDirectoryContent(string directory, string extension = "")
	{
		if (!FileIO.FindFiles(FindFilesCallbackMethod, directory, extension))
			return null;

		array<ref SCR_FileInfo> result = SCR_ArrayHelperRefT<SCR_FileInfo>.GetCopy(FOUND_FILEINFOS);
		FOUND_FILEINFOS.Clear();

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get file content as array of lines
	//! \param[in] filePath relative or absolute
	//! \param[in] printWarning true to print warning on issue, false otherwise
	//! \return array of lines or null if file does not exist or cannot be opened
	static array<string> ReadFileContent(string filePath, bool printWarning = true)
	{
		FileHandle fileHandle = FileIO.OpenFile(filePath, FileMode.READ);
		if (!fileHandle)
		{
			if (printWarning)
				Print("Cannot open/read " + filePath, LogLevel.WARNING);

			return null;
		}

		string lineContent;
		array<string> result = {};
		while (fileHandle.ReadLine(lineContent) > -1)
		{
			result.Insert(lineContent);
		}

		fileHandle.Close();

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Write all lines in the file, replacing all its content
	//! Overwrites the file if it exists
	//! \param[in] filePath relative or absolute
	//! \param[in] lines
	//! \return true on success, false otherwise
	static bool WriteFileContent(string filePath, notnull array<string> lines)
	{
		FileHandle fileHandle = FileIO.OpenFile(filePath, FileMode.WRITE);
		if (!fileHandle)
		{
			Print("Cannot open/write " + filePath, LogLevel.WARNING);
			return false;
		}

		int linesCountMinusOne = lines.Count() - 1;
		if (linesCountMinusOne == -1)
		{
			fileHandle.Write(string.Empty); // needed?
		}
		else
		{
			foreach (int lineNumber, string line : lines)
			{
				if (lineNumber < linesCountMinusOne)
					fileHandle.WriteLine(line);
			}

			// avoid final line return due to WriteLine
			fileHandle.Write(lines[linesCountMinusOne]);
		}

		fileHandle.Close();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Write all lines in the file after its existing content
	//! Adds to the file if it exists, creates it otherwise
	//! \param[in] filePath relative or absolute
	//! \param[in] lines
	//! \return true on success, false otherwise
	static bool AppendFileContent(string filePath, notnull array<string> lines)
	{
		if (!FileIO.FileExists(filePath))
			return WriteFileContent(filePath, lines);

		FileHandle fileHandle = FileIO.OpenFile(filePath, FileMode.APPEND);
		if (!fileHandle)
		{
			Print("Cannot open/append " + filePath, LogLevel.WARNING);
			return false;
		}

		int linesCountMinusOne = lines.Count() - 1;
		if (linesCountMinusOne == -1)
		{
			fileHandle.Write(string.Empty); // needed?
		}
		else
		{
			// add a line return before appending the provided lines
			fileHandle.Write("\n");

			foreach (int lineNumber, string line : lines)
			{
				if (lineNumber < linesCountMinusOne)
					fileHandle.WriteLine(line);
			}

			// avoid final line return due to WriteLine
			fileHandle.Write(lines[linesCountMinusOne]);
		}

		fileHandle.Close();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get whether or not a file name is valid for the filesystem
	//! \param[in] fileName the file name to be sanitised - no file path accepted
	//! \return true if the provided file name is valid under the current OS, false otherwise
	static bool IsValidFileName(string fileName)
	{
		bool isWindowsBased = IsWindowsBased();
		if (isWindowsBased)
		{
			if (fileName.EndsWith("."))
				return false;

			if (fileName.EndsWith(" "))
				return false;
		}

		string forbiddenCharacters;
		if (isWindowsBased)
			forbiddenCharacters = FORBIDDEN_FILENAME_CHARS_WINDOWS;
		else
			forbiddenCharacters = FORBIDDEN_FILENAME_CHARS_LINUX;

		for (int i, len = forbiddenCharacters.Length(); i < len; i++)
		{
			if (fileName.Contains(forbiddenCharacters[i]))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Sanitise the provided file name (removes invalid characters depending on the current OS, directory information, etc)
	//! \param[in] fileName the file name to be sanitised - can take a file path that will be stripped
	//! \return the sanitised, trimmed file name or empty string if no characters are viable
	static string SanitiseFileName(string fileName)
	{
		string forbiddenCharacters;
		bool isWindowsBased = IsWindowsBased();
		if (isWindowsBased)
			forbiddenCharacters = FORBIDDEN_FILENAME_CHARS_WINDOWS;
		else
			forbiddenCharacters = FORBIDDEN_FILENAME_CHARS_LINUX;

		fileName = FilePath.StripPath(fileName);
		fileName = SCR_StringHelper.Filter(fileName, forbiddenCharacters, true);

		fileName.TrimInPlace();
		if (fileName.IsEmpty())
			return string.Empty;

		if (isWindowsBased)
		{
			int originalLength = fileName.Length();
			int properLength = originalLength;
			string lastChar = fileName[properLength - 1];

			while (lastChar == "." || lastChar == " " || lastChar == "\t")
			{
				--properLength;
				if (properLength < 1)
					break;

				lastChar = fileName[properLength - 1];
			}

			if (properLength < 1)
				return string.Empty;

			if (properLength != originalLength)
				fileName = fileName.Substring(0, properLength);
		}

		fileName.TrimInPlace();

		return fileName;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if Windows or Xbox, false otherwise
	protected static bool IsWindowsBased()
	{
		EPlatform platform = System.GetPlatform();

		return platform == EPlatform.WINDOWS
			|| platform == EPlatform.XBOX_ONE
			|| platform == EPlatform.XBOX_ONE_S
			|| platform == EPlatform.XBOX_ONE_X
			|| platform == EPlatform.XBOX_SERIES_S
			|| platform == EPlatform.XBOX_SERIES_X;
	}

	//------------------------------------------------------------------------------------------------
	// FindFilesCallback
	protected static void FindFilesCallbackMethod(string fileName, FileAttribute attributes = 0, string filesystem = string.Empty)
	{
		SCR_FileInfo fileInfo = new SCR_FileInfo();
		fileInfo.m_sFilePath = fileName;
		fileInfo.m_eAttributes = attributes;
		fileInfo.m_sFileSystem = filesystem;
		FOUND_FILEINFOS.Insert(fileInfo);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void SCR_FileIOHelper();
}

class SCR_FileInfo
{
	string m_sFilePath;
	FileAttribute m_eAttributes;
	string m_sFileSystem;
}
