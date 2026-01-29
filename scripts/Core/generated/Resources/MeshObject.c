/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Resources
\{
*/

/*!
\brief MeshObject
@code
void GenerateModel(IEntity obj)
{
	vector verts[3] = {Vector(0, 0, 0), Vector(0, 0, 1000), Vector(1000, 0, 0)};
	float uvs[6] = {0.0,0.0, 1.0,0.0, 0.0,1.0};
	int indices[3] = {0,1,2};
	int numVertices[] = {3};
	int numIndices[] = {3};
	string materials[] = {"{726C1969B0B78245}defMat.emat"};

	Resource res = MeshObject.Create(1, numVertices, numIndices, materials, 0);
	MeshObject meshObject = res.GetResource().ToMeshObject();
	meshObject.UpdateVerts(0, verts, uvs);
	meshObject.UpdateIndices(0, indices);

	obj.SetObject(meshObject,"");
}
@endcode
*/
sealed class MeshObject: VObject
{
	static proto ref Resource Create(int numMeshes, int numVerts[], int numIndices[], string materials[], MeshObjectFlags flags);
	proto external void UpdateVerts(int meshIdx, vector verts[], float uv[]);
	proto external void UpdateIndices(int meshIdx, int indices[]);
	proto external int GetNumGeoms();
	proto external bool HasValidMask();
	proto external bool HasLayerMask(int layerMask, int layerFilter);
}

/*!
\}
*/
