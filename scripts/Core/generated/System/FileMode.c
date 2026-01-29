/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

//!Mode for opening file. See FileSystem::Open
enum FileMode
{
	//!File is readonly
	READ,
	//!File is just opened for write
	OPENW,
	//!File is read-write. When do not exists, it's created
	WRITE,
	//!File pointer is set to end of file
	APPEND,
	MASK,
	FILEFLAGS_NO_PREREAD,
}

/*!
\}
*/
