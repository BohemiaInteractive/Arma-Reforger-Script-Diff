[ComponentEditorProps(category: "GameScripted/Area Mesh", description: "")]
class SCR_BaseAreaMeshComponentClass : ScriptComponentClass
{
}

class SCR_BaseAreaMeshComponent : ScriptComponent
{
	//~ Resolution of Ellipse mesh in preview (Rectangle by definition has a resolution of 4)
	static const int PREVIEW_RESOLUTION = 48;
	
	[Attribute(defvalue: "10", category: "Virtual Area", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EAreaMeshShape), desc: "Area shape. If Ellipse it will use Radius while Rectangle uses diameter (width and length) rather like ellipse using half the diameter)")]
	protected EAreaMeshShape m_eShape;
	
	[Attribute("10", category: "Virtual Area")]
	protected float m_fHeight;
	
	[Attribute("12", desc: "How many segments in the ellipse (ELLIPSE Shapes only as RECTANGLE has a set resolution of 4).", uiwidget: UIWidgets.Slider, params: "4 60 1", category: "Virtual Area")]
	protected int m_vResolution;
	
	[Attribute(desc: "True to let the border copy terrain, creating a 'wall'.", category: "Virtual Area")]
	protected bool m_bFollowTerrain;
	
	[Attribute(desc: "True to stretch the material along the whole circumference instead of mapping it on each segment.", category: "Virtual Area")]
	protected bool m_bStretchMaterial;
	
	[Attribute(desc: "Material mapped on outside and inside of the mesh. Inside mapping is mirrored.", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "emat", category: "Virtual Area")]
	protected ResourceName m_Material;
	
	[Attribute("false", desc: "When enabled, the component will be active from init", category: "Virtual Area")]
	protected bool m_bActiveEveryFrameOnInit;
	
	[Attribute("0", desc: "If true always hide the zone in workbench. This setting is ignored when playing even within workbench", category: "Workbench")]
	protected bool m_bHideInWorkbench;
	
	protected vector m_vLastPos;
	protected vector m_vLastDir;
	
	//------------------------------------------------------------------------------------------------
	//! Activate the area so it could be updated every frame
	void ActivateEveryFrame()
	{
		SetEventMask(GetOwner(), EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Deactivate the area
	void DeactivateEveryFrame()
	{
		ClearEventMask(GetOwner(), EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get radius of the area.
	//! To be overloaded by inherited classes.
	//! \return Radius
	float GetRadius();
	
	//------------------------------------------------------------------------------------------------
	//! Get height of the area.
	//! \return Height
	float GetHeight()
	{
		return m_fHeight;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return shape if areaMesh
	//! \return Shape Enum
	EAreaMeshShape GetShape()
	{
		return m_eShape;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Resolution of the area.
	//! Always 4 if Rectangle
	//! \return Resolution
	int GetResolution()
	{
		if (GetShape() == EAreaMeshShape.RECTANGLE)
			return 4;
		else 
			return m_vResolution;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns width and length
	//! To be overloaded by inherited classes.
	//! By default uses radius as width and length unless function is overwritten
	//! \param[out] width Width of Zone (x)
	//! \param[out] length length of Zone (z)
	void GetDimensions2D(out float width, out float length)
	{
		width = GetRadius();
		length =  width;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns dimensions in vector
	//! \param[out] dimensions width, height, length
	void GetDimensions3D(out vector dimensions)
	{
		float width, length;
		GetDimensions2D(width, length);
		dimensions[0] = width;
		dimensions[1] = GetHeight();
		dimensions[2] = length;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the offset of the AreaMesh. Can be overwritten to set custom position. Only x and z are used. Height is ignored
	//! \param[in] owner
	//! \return
	protected vector GetMeshOffset(IEntity owner)
	{
		return vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get material used for area mesh
	protected ResourceName GetMaterial()
	{
		return m_Material;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Generate area mesh based on its settings.
	void GenerateAreaMesh()
	{
		IEntity owner = GetOwner();
		
		#ifdef WORKBENCH
		if (m_bHideInWorkbench && owner.GetWorld() && owner.GetWorld().IsEditMode())
			return;
		#endif
		
		vector dimensions;
		GetDimensions3D(dimensions);
		
		if (dimensions[0] <= 0 || dimensions[1] <= 0 || dimensions[2] <= 0)
		{
			owner.SetObject(null, "");
			return;
		}	
		
		array<vector> positions = {};
		vector offset = GetMeshOffset(owner);
		
		if (m_eShape == EAreaMeshShape.ELLIPSE)
		{
			int resolution = GetResolution();
			float dirStep = Math.PI2 / resolution;

			//--- Get positions
			for (int v = 0; v < resolution; v++)
			{
				float dir = dirStep * v;
				vector pos = Vector((Math.Sin(dir) * dimensions[0]) + offset[0], -dimensions[1], (Math.Cos(dir) * dimensions[2]) + offset[2]);
				positions.Insert(pos);
			}
		}
		else if (m_eShape == EAreaMeshShape.RECTANGLE)
		{
			//~ Make sure it uses half of the width and length
			float width = dimensions[0] * 0.5;
			float length = dimensions[2] * 0.5;
			
			array<vector> corners = 
			{
				{ -width + offset[0], -dimensions[1], -length + offset[2] },
				{ width + offset[0], -dimensions[1], -length + offset[2] },
				{ width + offset[0], -dimensions[1], length + offset[2] },
				{ -width + offset[0], -dimensions[1], length + offset[2] }
			};
			
			//~ Set positions
			for (int i = 0; i < 4; i++)
			{
				vector start = corners[i];
				vector end = corners[(i + 1) % 4];
				
				for (float s = 0; s < 4; s++)
				{
					vector pos = vector.Lerp(start, end, s * 0.25);
					positions.Insert(pos);
				}
			}
		}
		else 
		{
			Print(string.Format("'SCR_BaseAreaMeshComponent' does not support shape type %1 (%2)", typename.EnumToString(EAreaMeshShape, m_eShape), m_eShape), LogLevel.ERROR);
			owner.SetObject(null, "");
			return;
		}
		
		if (m_bFollowTerrain)
		{
			//--- Reset entity rotation
			vector transform[4];
			owner.GetWorldTransform(transform);
			vector angles = Math3D.MatrixToAngles(transform);
			angles[1] = 0; //--- Reset pitch
			angles[2] = 0; //--- Reset roll
			Math3D.AnglesToMatrix(angles, transform);
			owner.SetWorldTransform(transform);
			
			//--- Snap all positions to ground
			BaseWorld world = owner.GetWorld();
			vector worldPos;
			foreach (int i, vector pos: positions)
			{
				worldPos = owner.CoordToParent(pos);
				worldPos[1] = Math.Max(world.GetSurfaceY(worldPos[0], worldPos[2]), 0);
				positions[i] = owner.CoordToLocal(worldPos);
			}
		}
		
		Resource res = SCR_Shape.CreateAreaMesh(positions, dimensions[1] * 2, GetMaterial(), m_bStretchMaterial);
		MeshObject meshObject = res.GetResource().ToMeshObject();
		if (meshObject)
		{
			owner.SetObject(meshObject, "");
			
			m_vLastPos = owner.GetOrigin();
			m_vLastDir = owner.GetAngles();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (vector.DistanceSq(m_vLastPos, owner.GetOrigin()) > 0.1 || vector.DistanceSq(m_vLastDir, owner.GetAngles()) > 0.1)
			GenerateAreaMesh();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
		
		if (m_bActiveEveryFrameOnInit)
			ActivateEveryFrame();
	}
	
	#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//~ Makes sure mesh area is generated at the correct position in workbench
	override void _WB_SetTransform(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		GenerateAreaMesh();
	}
	#endif
}

enum EAreaMeshShape
{
	ELLIPSE 		= 10,  ///< Ellipse using Radius and Resolution. 
	RECTANGLE 		= 20, ///< Rectangle shape that has a width and length.
}
