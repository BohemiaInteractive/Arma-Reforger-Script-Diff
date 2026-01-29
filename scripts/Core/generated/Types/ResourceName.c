/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Types
\{
*/

sealed class ResourceName: string
{
	proto external string GetPath();
	//!Has GUID and store-name with extension
	proto external bool IsExternal();
	//!Has only GUID, not store-name
	proto external bool IsInternal();
}

/*!
\}
*/
