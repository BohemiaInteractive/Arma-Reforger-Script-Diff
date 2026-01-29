[EntityEditorProps(category: "GameScripted/Shapes", description: "Procedural box", color: "255 0 0 255", style: "box", dynamicBox: true)]
class SCR_TerrainMeshEntityClass: GenericEntityClass
{
};

/*!
Miniature terrain mesh.
*/
class SCR_TerrainMeshEntity : GenericEntity
{
	[Attribute(category: "Terrain Mesh", defvalue: "4", params: "2 32 1", uiwidget: UIWidgets.Slider)]
	protected int m_iResolution;
	
	[Attribute(category: "Terrain Mesh", defvalue: "0.01", params: "0 1 0.01", uiwidget: UIWidgets.Slider)]
	protected float m_fHeightCoef;
	
	[Attribute(category: "Terrain Mesh")]
	protected ref array<float> m_aHeights;
	
	[Attribute(category: "Terrain Mesh", desc: "Material mapped on the mesh.", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "emat")]
	private ResourceName m_Material;
	
	void Generate(array<float> heights, ResourceName material)
	{
		int heightsCount = heights.Count();
		if (heightsCount != m_iResolution * m_iResolution)
		{
			Print(string.Format("Resolution of height values (%1) doesn't match the resolution attribute (%2)", Math.Sqrt(heightsCount), m_iResolution), LogLevel.WARNING);
			return;
		}
		int resolution1 = m_iResolution - 1;
		
		vector verts[2048];
		float uvs[2048];
		int indices[2048];
		int vI, iI, uI, row, column;

		//--- Get positions
		for (int y = 0; y < m_iResolution; y++)
		{
			for (int x = 0; x < m_iResolution; x++)
			{
				verts[vI] = Vector(
					x / resolution1 - 0.5,
					heights[vI] * m_fHeightCoef,
					y / resolution1 - 0.5
				);
				vI++;
				
				uvs[uI] = x / resolution1; uI++;
				uvs[uI] = 1 - y / resolution1; uI++;
			}
		}
		
		/*
		    6 - 7 - 8
		    | \ | \ |
		    3 - 4 - 5
		    | \ | \ |
			0 - 1 - 2
		
			Vertex 1: 0-3-4
			Vertex 2: 4-1-0
			...
		*/
		for (int s = 0; s < vI - m_iResolution; s++)
		{
			column = s % m_iResolution;
			if (column == resolution1)
			{
				row++;
				continue;
			}
			
			int pointBottomLeft = s;
			int pointBottomRight = s + 1;
			int pointTopLeft = pointBottomLeft + m_iResolution;
			int pointTopRight = pointBottomRight + m_iResolution;
			
			indices[iI] = pointBottomLeft; iI++;
			indices[iI] = pointTopLeft; iI++;
			indices[iI] = pointTopRight; iI++;
			
			indices[iI] = pointTopRight; iI++;
			indices[iI] = pointBottomRight; iI++;
			indices[iI] = pointBottomLeft; iI++;
		}

		int numVertices[] = {vI};
		int numIndices[] = {iI};
		string materials[] = {material};
		
		Resource res = MeshObject.Create(1, numVertices, numIndices, materials, 0);
		MeshObject meshObject = res.GetResource().ToMeshObject();
		meshObject.UpdateVerts(0, verts, uvs);
		meshObject.UpdateIndices(0, indices);
		SetObject(meshObject,"");
		Physics.CreateStatic(this, 0xffffffff);
	}
	
	void SCR_TerrainMeshEntity(IEntitySource src, IEntity parent)
	{
		Generate(m_aHeights, m_Material);
	}
	
#ifdef WORKBENCH
	override array<ref WB_UIMenuItem> _WB_GetContextMenuItems()
	{
		return { new WB_UIMenuItem("Generate from the current terrain", 0) };
	}
	override void _WB_OnContextMenu(int id)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor) return;
		
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api) return;
		
		BaseWorld world = api.GetWorld();
		if (!world) return;
		
		vector min, max, size;
		//world.GetBoundBox(mins, maxs);
		worldEditor.GetTerrainBounds(min, max);
		size = max - min;
		float stepX = size[0] / m_iResolution;
		float stepY = size[2] / m_iResolution;
		float heightMax = max[1] - Math.Max(min[1], 0);
		
		int resolution1 = m_iResolution - 1;
		bool addComma = false;
		string value;
		float posX, posY, posZ;
		for (int y = 0; y < m_iResolution; y++)
		{
			for (int x = 0; x < m_iResolution; x++)
			{
				if (addComma) value += ",";
				posX = min[0] + size[0] * x / resolution1;
				posY = min[2] + size[2] * y / resolution1;
				posZ = Math.Max(world.GetSurfaceY(posX, posY), 0) / heightMax;
				value += posZ.ToString();
				addComma = true;
			}
		}
		Print(value);

		api.BeginEntityAction();
		api.SetVariableValue(api.EntityToSource(this), null, "m_aHeights", value);
		api.EndEntityAction();
	}
#endif
};