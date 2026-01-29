[EntityEditorProps(category: "GameScripted/Editor", description: "Core Editor manager", color: "251 91 0 255", dynamicBox: true)]
class SCR_VirtualAreaEntityClass: GenericEntityClass
{
};

class SCR_VirtualAreaEntity : GenericEntity
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, category: "Virtual Area", enums: { ParamEnum("Ellipse (Inside Radius)", "0", ""), ParamEnum("Ellipse (Outside Radius)", "1", ""), ParamEnum("Rectangle", "2", "") }, desc: "Area shape")]
	private int m_Shape;
	
	[Attribute("10 10 10", desc: "Minor axes (i.e., XZ values are radius, not diameter, and Y value is half the height).", category: "Virtual Area")]
	private vector m_vSize;
	
	[Attribute("12", desc: "How many segments in the ellipse.", uiwidget: UIWidgets.Slider, params: "4 60 1", category: "Virtual Area")]
	private int m_vResolution;
	
	[Attribute(desc: "True to let the border copy terrain, creating a 'wall'.", category: "Virtual Area")]
	private bool m_bFollowTerrain;
	
	[Attribute(desc: "True to stretch the material along the whole circumference instead of mapping it on each segment.", category: "Virtual Area")]
	private bool m_bStretchMaterial;
	
	[Attribute(desc: "Material mapped on outside and inside of the mesh. Inside mapping is mirrored.", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "emat", category: "Virtual Area")]
	private ResourceName m_Material;
	
	/*!
	Get area shape.
	\return 0 for inside radius ellipse, 1 for outside radius ellipse, 2 for rectangle
	*/
	int GetShape()
	{
		return m_Shape;
	}
	/*!
	Get area size.
	\return Minor axes
	*/
	vector GetSize()
	{
		return m_vSize;
	}
	/*!
	Check if given position is inside the area.
	\return True if in the area
	*/
	bool InArea(vector pos)
	{
		pos = CoordToLocal(pos);
		float sizeX = m_vSize[0];
		float sizeY = m_vSize[1];
		float sizeZ = m_vSize[2];
		switch (m_Shape)
		{
			//--- Ellipse
			case 0:
			case 1:
			{
				if (m_Shape == 1)
				{
					//--- Outside
					float coef = Math.Sqrt(Math.Pow(sizeX, 2) + Math.Pow(sizeZ, 2)) / ((sizeX + sizeZ) / 2); //--- Not accurate for ellipse (ToDo: Better calculation)
					sizeX *= coef;
					sizeZ *= coef;
				}
				if (sizeX == sizeZ)
				{
					//--- Circle
					return vector.Distance(Vector(pos[0], 0, pos[2]), vector.Zero) < sizeX;
				}
				//--- ToDo: Ellipse
				break;
			}
			case 2:
			{
				//--- Rectangle
				return (pos[0] > -sizeX && pos[0] < sizeX) && (pos[2] > -sizeZ && pos[2] < sizeZ);
			}
		}
		return false;
	}
	/*!
	Set area material.
	\return material New material
	*/
	void SetMaterial(ResourceName material)
	{
		if (material == m_Material) return;
		
		m_Material = material;
		Generate(m_vSize, m_vResolution, m_bFollowTerrain);
	}

	/*!
	Generate the mesh with given params.
	\param size Minor axes (i.e., XZ values are radius, not diameter, and Y value is half the height).
	\param resolution How many segments in the ellipse.
	*/
	void Generate(vector size, int resolution)
	{
		Generate(size, resolution, m_bFollowTerrain);
	}
	/*!
	Generate the mesh with given params.
	\param size Minor axes (i.e., XZ values are radius, not diameter, and Y value is half the height).
	\param resolution How many segments in the ellipse.
	\param followTerrain True to let the border copy terrain, creating a 'wall'.
	*/
	void Generate(vector size, int resolution, bool followTerrain = false)
	{
		m_vSize = size;
		m_vResolution = resolution;
		m_bFollowTerrain = followTerrain;
		
		if (resolution * 12 > 720)
		{
			Print(string.Format("Cannot generate shape, resolution %1 is above maximum value %2!", resolution, 720 / 12), LogLevel.ERROR);
			return;
		}
		
		float sizeX = m_vSize[0];
		float sizeY = m_vSize[1];
		float sizeZ = m_vSize[2];
		
		array<vector> positions = new array<vector>;
		switch (m_Shape)
		{
			//--- Ellipse
			case 0:
			case 1:
			{
				if (m_Shape == 1)
				{
					//--- Outside
					float coef = Math.Sqrt(Math.Pow(sizeX, 2) + Math.Pow(sizeZ, 2)) / ((sizeX + sizeZ) / 2); //--- Not accurate for ellipse (ToDo: Better calculation)
					sizeX *= coef;
					sizeZ *= coef;
				}
				float dirStep = Math.PI2 / resolution;
				for (int v = 0; v < resolution; v++)
				{
					float dir = dirStep * v;
					vector pos = Vector(Math.Sin(dir) * sizeX, -sizeY, Math.Cos(dir) * sizeZ);
					positions.Insert(pos);
				}
				break;
			}
			//--- Rectangle
			case 2:
			{
				float perimeter = (sizeX + sizeZ) * 2;
				float segmentsX = Math.Max(resolution * (sizeX / perimeter), 1);
				float segmentsY = Math.Max(resolution * (sizeZ / perimeter), 1);
				
				array<vector> corners = {
					Vector(-sizeX, -sizeY, -sizeZ),
					Vector(sizeX, -sizeY, -sizeZ),
					Vector(sizeX, -sizeY, sizeZ),
					Vector(-sizeX, -sizeY, sizeZ)
				};
				for (int i = 0; i < 4; i++)
				{
					vector start = corners[i];
					vector end = corners[(i + 1) % 4];
					
					int segments = segmentsX;
					if (i % 2 == 1) segments = segmentsY;
					for (float s = 0; s < segments; s++)
					{
						vector pos = vector.Lerp(start, end, s / segments);
						positions.Insert(pos);
					}
				}
				break;
			}
		}
		
		//--- Snap all positions to ground
		if (m_bFollowTerrain)
		{
			vector transform[4];
			float entitySurfaceY;
			GetTransform(transform);
			entitySurfaceY = Math.Max(GetWorld().GetSurfaceY(transform[3][0], transform[3][2]), 0);
			foreach (int i, vector pos: positions)
			{
				vector worldPos = transform[3] + transform[0] * pos[0] + transform[2] * pos[2];
				pos[1] = Math.Max(GetWorld().GetSurfaceY(worldPos[0], worldPos[2]), 0) - entitySurfaceY;
				positions[i] = pos;
			}
		}
		
		Resource res = SCR_Shape.CreateAreaMesh(positions, sizeY * 2, m_Material, m_bStretchMaterial);
		MeshObject meshObject = res.GetResource().ToMeshObject();
		if (meshObject) SetObject(meshObject,"");
	}
	
	void SCR_VirtualAreaEntity(IEntitySource src, IEntity parent)
	{
		Generate(m_vSize, m_vResolution, m_bFollowTerrain);
	}
	
#ifdef WORKBENCH
	override void _WB_GetBoundBox(inout vector min, inout vector max, IEntitySource src)
	{
		min = -m_vSize;
		max = m_vSize;
		
		/*
		if (m_Shape == 1)
		{
			float coef = Math.Sqrt(Math.Pow(m_vSize[0], 2) + Math.Pow(m_vSize[2], 2)) / Math.Max(m_vSize[0], m_vSize[2]);
			min *= coef;
			max *= coef;
		}
		*/
	}
#endif
};