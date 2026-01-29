/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Serialization
\{
*/

/*!
Utility context used for temporarily buffering serialization data to apply sequentially later.
*/
sealed class BufferSerializationSaveContext: BaseSerializationSaveContext
{
	//! Utility to create a preconfigured buffer context
	static BufferSerializationSaveContext Create(notnull BaseSerializationSaveContext other)
	{
		BufferSerializationSaveContext context();
		context.Configure(other);
		return context;
	}

	//! Configure the bufffer with the same characteristics as another "main" context. Primarily relevant for the ability to seek members.
	proto external void Configure(BaseSerializationSaveContext other);
	//! Apply all buffered instructions on the target save context.
	proto external void Apply(BaseSerializationSaveContext other);
	//! Get amount of instructions buffered
	proto external int GetInstructionCount();
}

/*!
\}
*/
