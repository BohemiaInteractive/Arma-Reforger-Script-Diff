/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Debug
\{
*/

sealed class ShapeUtils
{
	private void ShapeUtils();
	private void ~ShapeUtils();

	/*!
	Creates a shape from a given mesh object
	\param mo						Pointer to the MeshObject from which the shape will be created.
	\param lodIndex			Level of detail index to be used for the shape.
	\param meshName			The name of the mesh.
	\param shapeColor		The color of the shape
	\param flags				Flags that determine the properties and behavior of the shape.
	*/
	static proto ref Shape CreateShapeFromMesh(notnull MeshObject mo, int lodIndex, string meshName, int shapeColor, ShapeFlags flags);
	/*!
	Creates a shape from a given mesh object
	\param mo						Pointer to the MeshObject from which the shape will be created.
	\param lodIndex			Level of detail index to be used for the shape.
	\param meshIndex		Index of the mesh.
	\param shapeColor		The color of the shape
	\param flags				Flags that determine the properties and behavior of the shape.
	*/
	static proto ref Shape CreateShapeFromMeshByIndex(notnull MeshObject mo, int lodIndex, int meshIndex, int shapeColor, ShapeFlags flags);
	/*!
	Creates a shape from a given mesh object
	\param xobFilePath	ResourceName of the MeshObject from which the shape will be created.
	\param lodIndex			Level of detail index to be used for the shape.
	\param meshName			Name of the mesh.
	\param shapeColor		The color of the shape
	\param flags				Flags that determine the properties and behavior of the shape.
	*/
	static proto ref Shape CreateShapeFromXOB(ResourceName xobFilePath, int lodIndex, string meshName, int shapeColor, ShapeFlags flags);
}

/*!
\}
*/
