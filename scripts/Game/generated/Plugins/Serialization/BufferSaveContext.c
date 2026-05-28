/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Serialization
\{
*/

/*!
Utility context used for temporarily buffering serialization data to apply sequentially later.
*/
sealed class BufferSaveContext: SaveContext
{
	//! Utility to create a preconfigured buffer context
	static BufferSaveContext Create(notnull SaveContext other)
	{
		BufferSaveContext context();
		context.Configure(other);
		return context;
	}

	//! Configure the bufffer with the same characteristics as another "main" context. Primarily relevant for the ability to seek members.
	proto external void Configure(SaveContext other);
	//! Apply all buffered instructions on the target save context.
	proto external void Apply(SaveContext other);
	//! Get amount of instructions buffered
	proto external int GetInstructionCount();
}

/*!
\}
*/
