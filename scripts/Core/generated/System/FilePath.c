/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

sealed class FilePath
{
	private void FilePath();
	private void ~FilePath();

	/*!
	Checks if given char is valid path delimiter.
	\param c Single char string to check
	\return true if c is path delimiter, false otherwise
	*/
	static proto bool IsDelimiter(string c);
	/*!
	Checks if given char is valid system path delimiter.
	\param c Single char string to check
	\return true if c is system path delimiter, false otherwise
	*/
	static proto bool IsSystemDelimiter(string c);
	/*!
	Converts given delimiter to system delimiter.
	\param c Single char string to convert
	\return System delimiter if IsDelimiter(c), c otherwise
	*/
	static proto string ToSystemDelimiter(string c);
	/*!
	Removes extension from given path.
	\param path Path to remove extension from
	\param[out] ext Extension which was removed
	\return path without an extension
	*/
	static proto string StripExtension(string path, out string ext = "");
	/*!
	Concatenates two strings two parts of path together.
	\param firstPart First part of path to concantenate
	\param secondPart Second part of path to concantenate
	\return A path given by concatenating firstPart and secondPart
	*/
	static proto string Concat(string firstPart, string secondPart);
	/*!
	Removes filename from given path.
	\param path Path to remove the file name from
	\return Path without a file name
	*/
	static proto string StripFileName(string path);
	/*!
	Extracts a file name from given path.
	\param path Path to extract the file name from
	\return File name extracted from given path
	*/
	static proto string StripPath(string path);
	/*!
	Converts given path to system compatible format.
	\param path Path to be converted
	\return Converted path
	*/
	static proto string ToSystemFormat(string path);
	/*!
	Converts path to internal compatible format.
	\param path Path to be converted
	\return Converted path
	*/
	static proto string ToInternalFormat(string path);
	/*!
	Replaces an extension in given path.
	\param path Path to replace the extension in
	\param extension New extension to place to path
	\return Path with the extension replaced
	*/
	static proto string ReplaceExtension(string path, string extension);
	/*!
	Appends an extension to the path.
	\param path Path to append the extension to
	\param extension The extension to append
	\return Path with the extension appended
	*/
	static proto string AppendExtension(string path, string extension);
	/*!
	Checks whether a path is absolute or not.
	\param path Path to check
	\return true if path is absolute, false otherwise
	*/
	static proto bool IsAbsolutePath(string path);
	/*!
	Checks wheter a path matches a pattern.
	\param path Path to be tested
	\param pattern Standard wildcards are supported (?,*)
	\return true if the path matches the pattern, false otherwise
	*/
	static proto bool MatchesPattern(string path, string pattern);
	/*!
	Gets FileSystemName part of given path. For example, `$mypath:File` would return `mypath`.
	\param path Full path to file
	\return Resulting File system name
	*/
	static proto string FileSystemNameFromFileName(string path);
}

/*!
\}
*/
