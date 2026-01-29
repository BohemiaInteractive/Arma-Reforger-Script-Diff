[EntityEditorProps(category: "GameScripted/Shapes", description: "Procedural box", color: "255 0 0 255", style: "box", dynamicBox: true)]
class SCR_GenericBoxEntityClass: GenericEntityClass
{
};

/*!
Procedural box entity.
*/
class SCR_GenericBoxEntity : GenericEntity
{
	[Attribute("1 1 1", desc: "Size of the box", category: "Generic Box")]
	private vector m_vSize;
	
	[Attribute(desc: "Material mapped on outside and inside of the mesh. Inside mapping is mirrored.", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "emat", category: "Generic Box")]
	private ResourceName m_Material;
	
	void Generate(vector size, ResourceName material)
	{		
		private float vX = size[0] / 2;
		private float vY = size[1] / 2;
		private float vZ = size[2] / 2;
		
		/*
		        7 - 6
		       /|  /|
			    4 - 5
		       /   /
		    3 - 2
		    |   |
			0 - 1
		*/
		vector vert0 = Vector(-vX, -vY, -vZ);
		vector vert1 = Vector( vX, -vY, -vZ);
		vector vert2 = Vector( vX, -vY,  vZ);
		vector vert3 = Vector(-vX, -vY,  vZ);
		vector vert4 = Vector(-vX,  vY, -vZ);
		vector vert5 = Vector( vX,  vY, -vZ);
		vector vert6 = Vector( vX,  vY,  vZ);
		vector vert7 = Vector(-vX,  vY,  vZ);
		
		vector verts[] =
		{
			vert2, vert3, vert0, vert1, //--- Bottom:  0,  1,  2,  3
			vert7, vert6, vert5, vert4, //--- Top:     4,  5,  6,  7
			vert4, vert5, vert1, vert0, //--- Front:   8,  9, 10, 11
			vert6, vert7, vert3, vert2, //--- Back:   12, 13, 14, 15
			vert7, vert4, vert0, vert3, //--- Left:   16, 17, 18, 19
			vert5, vert6, vert2, vert1, //--- Right:  20, 21, 22, 23
		};
		
		int indices[] =
		{	
			 0,  1,  2,		 2,  3,  0, //--- Bottom
			 4,  5,  6,		 6,  7,  4, //--- Top
			 8,  9, 10,		10, 11,  8, //--- Front
			12, 13, 14,		14, 15, 12, //--- Back
			16, 17, 18,		18, 19, 16, //--- Left
			20, 21, 22,		22, 23, 20, //--- Right
		};
		float uvs[] =
		{
			0,0, 1,0, 1,1, 0,1,
			0,0, 1,0, 1,1, 0,1,
			0,0, 1,0, 1,1, 0,1,
			0,0, 1,0, 1,1, 0,1,
			0,0, 1,0, 1,1, 0,1,
			0,0, 1,0, 1,1, 0,1
		};
		
		int numVertices[] = {24};
		int numIndices[] = {36};
		string materials[1] = {material};
		
		Resource res = MeshObject.Create(1, numVertices, numIndices, materials, 0);
		MeshObject meshObject = res.GetResource().ToMeshObject();
		meshObject.UpdateVerts(0, verts, uvs);
		meshObject.UpdateIndices(0, indices);
		if (meshObject) SetObject(meshObject,"");
	}
	
	void SCR_GenericBoxEntity(IEntitySource src, IEntity parent)
	{
		Generate(m_vSize, m_Material);
	}
	
#ifdef WORKBENCH
	override void _WB_GetBoundBox(inout vector min, inout vector max, IEntitySource src)
	{
		min = -m_vSize / 2;
		max = m_vSize / 2;
	}
#endif
};