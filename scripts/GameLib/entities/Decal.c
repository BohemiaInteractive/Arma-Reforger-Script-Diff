[EntityEditorProps(category: "GameLib/Generic", description: "Decal entity", dynamicBox: true, insertable: false)]
class WorldDecalClass: GenericEntityClass
{

}

//------------------------------------------------------------------------------
class WorldDecal: GenericEntity
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Far plane", "0.01 10")]
	private float FarPlane;
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Stretch", "0.01 100")]
	private float Stretch;
	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, desc: "Lifetime in seconds, 0 is valid and means the decal is permanent", "0 360 0.1")]
	private float Angle;
	[Attribute(defvalue: "", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Decal material", "emat")]
	private ResourceName Mat;
	[Attribute(defvalue: "300", uiwidget: UIWidgets.EditBox, desc: "Lifetime in seconds, 0 is valid and means the decal is permanent", "")]
	private float Lifetime;
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Terrain only")]
	private bool TerrainOnly;
	
	Decal m_decal;
	const float NearPlane = 0;

	#ifdef WORKBENCH
		ref Shape m_ArrowVis;
		ref Shape m_ProjBox;
		ref Shape m_IntersectionShape;
	
		bool m_bCheckIntersection;
		vector m_vIntersection;
	#endif

	//------------------------------------------------------------------------------
	void WorldDecal(IEntitySource src, IEntity parent)
	{
		ClearFlags(EntityFlags.ACTIVE, false);
		SetFlags(EntityFlags.STATIC, false);
		SetEventMask(EntityEvent.INIT);
		
		#ifdef WORKBENCH
			m_bCheckIntersection = false;
		#endif	
	}
	
	//------------------------------------------------------------------------------
	void ~WorldDecal()
	{
		Destroy();
	}
	
	//------------------------------------------------------------------------------
	vector GetForwardVec()
	{
		return GetTransformAxis(0).Normalized();
	}
	
	//------------------------------------------------------------------------------
	float GetSize()
	{
		float scale = GetScale();
		return Math.Clamp(scale, 0.01, 10.0);	
	}
		
#ifdef WORKBENCH
	//------------------------------------------------------------------------------
	Shape CreateArrow(vector from, vector to, float size, int color, ShapeFlags flags)
	{
		vector dir = to - from;
		float len = dir.Length();
		if (len < 0.01)
			len = 0.01;
		dir /= len;
		vector dir1 = dir * size;
		vector center = to - dir1; 
		size = size * 0.5;

		vector dir2 = dir.Perpend() * size;

		vector pts[6];
		pts[0] = from;
		pts[1] = center;
		pts[2] = to - dir1 - dir2;
		pts[3] = to;
		pts[4] = to - dir1 + dir2;
		pts[5] = center;

		return Shape.CreateLines(color, flags, pts, 6);
	}
	
	//------------------------------------------------------------------------------
	void GetPitchMatrix(float pitchRad, out vector rotMat[3])
	{
		float sp = Math.Sin(pitchRad);
		float cp = Math.Cos(pitchRad);
	
		rotMat[0][0] = 1;
		rotMat[1][0] = 0;
		rotMat[2][0] = 0;

		rotMat[0][1] = 0;
		rotMat[1][1] = cp;
		rotMat[2][1] = sp;

		rotMat[0][2] = 0;
		rotMat[1][2] = -sp;
		rotMat[2][2] = cp;
	}
	
	//------------------------------------------------------------------------------
	void RecreateVisualizers()
	{
		float dist = (FarPlane - NearPlane)*0.5;
		vector forward = GetForwardVec();
		m_ArrowVis = CreateArrow(GetOrigin() - forward*dist, GetOrigin() + forward*dist, 0.1, 0xFFFF0000, ShapeFlags.NOZBUFFER|ShapeFlags.DEPTH_DITHER);

		vector entMat[4];
		GetTransform(entMat);
		
		//Normalize() doesn't work on vector array, so now in this way
		entMat[0] = entMat[0].Normalized();
		entMat[1] = entMat[1].Normalized();		
		entMat[2] = entMat[2].Normalized();

		//rotation
		vector rotMat[3];
		GetPitchMatrix((Angle)*Math.DEG2RAD, rotMat);
		
		vector mat[3];		
		Math3D.MatrixMultiply3(entMat, rotMat, mat);
							
		float size = GetSize();
		
		vector point[6];
		float s = size*0.5;
		float w = s*Stretch;
		float f = FarPlane*0.5;
		point[0] = GetOrigin() + Vector(f, w, s).Multiply3(mat);
		point[1] = GetOrigin() + Vector(f, w,-s).Multiply3(mat);
		point[2] = GetOrigin() + Vector(f,-w,-s).Multiply3(mat);
		
		point[3] = point[2];
		point[4] = GetOrigin() + Vector(f,-w, s).Multiply3(mat);
		point[5] = point[0];
			
		
		int color = 0x80FF4000;
		if (m_bCheckIntersection)
			color = 0x20FF4000;
		
		m_ProjBox = Shape.CreateTris(color, ShapeFlags.NOZBUFFER|ShapeFlags.DEPTH_DITHER|ShapeFlags.NOOUTLINE|ShapeFlags.DOUBLESIDE|ShapeFlags.TRANSP, point, 2);
		
		if (m_bCheckIntersection)
		{
			float sphereSize = Math.Clamp(size*0.025, 0.01, 0.5);
			m_IntersectionShape = Shape.CreateSphere(
				0xFFFF8000, 
				ShapeFlags.NOZBUFFER|ShapeFlags.DEPTH_DITHER|ShapeFlags.NOOUTLINE, 
				m_vIntersection, 
				sphereSize);
		}
		else
		{
			m_IntersectionShape = null;
		}
	}
	
	//------------------------------------------------------------------------------
	override void _WB_GetBoundBox(inout vector min, inout vector max, IEntitySource src)
	{
		//HACK as we are in local coords, we have to scale it inversely as the entity matrix
		//will scale it back
		float size = GetSize();
		size = Math.Max(GetSize(), GetSize()*Stretch);
		/*
		it's not visible when rotating with Angle
		float s = Math.Sin(Angle*Math.DEG2RAD);
		float c = Math.Cos(Angle*Math.DEG2RAD);
		
		float x =  s + c;
		float y = -c + s; 
		float a = Math.Sqrt(x*x + y*y);
		*/

		float a = 0.707*Math.Max(1, Stretch);

		min = Vector((NearPlane - FarPlane*0.5)/GetSize(), -a, -a);
		max = Vector(FarPlane*0.5/GetSize(),  a,  a);
	}

	
	//------------------------------------------------------------------------------
	void ClearShapes()
	{
		if (m_ArrowVis)
			delete m_ArrowVis;
		
		if (m_ProjBox)
			delete m_ProjBox;
		
		if (m_IntersectionShape)
			delete m_IntersectionShape;	
	}
	
	//------------------------------------------------------------------------------
	override void _WB_SetExtraVisualiser(EntityVisualizerType type, IEntitySource src)
	{
		/*
		EVT_NONE,					//no visualizer
		EVT_NORMAL,					//normal visualizer
		EVT_SELECTED,				//selected entity visualiser (but no as main)
		EVT_SELECTED_AS_MAIN,		//selected as main entity visualiser
		EVT_UNDER_CURSOR,			//under cursor entity visualiser
		*/
		if (type == EntityVisualizerType.EVT_NONE)
		{

			ClearShapes();
		}
		else
		{
			RecreateVisualizers();
		}
	}
	
	//------------------------------------------------------------------------------
	override void _WB_SetTransform(inout vector mat[4], IEntitySource src)
	{
		Destroy();
		Create();
		
		RecreateVisualizers();
	}

#endif	
		
	//------------------------------------------------------------------------------
	override void EOnInit(IEntity owner) //!EntityEvent.INIT
	{
		Create();
	}

	//------------------------------------------------------------------------------
	void Destroy()
	{
		if (m_decal)
		{
			GetWorld().RemoveDecal(m_decal);
			m_decal = null;
		}
		
		#ifdef WORKBENCH
			ClearShapes();
		#endif
	}
	
	//------------------------------------------------------------------------------
	void Create()
	{
		if (Mat == string.Empty)
			return;
		
		vector origin = GetOrigin();
		vector forward = GetForwardVec();
		
		float farPlane = FarPlane*0.5;
		auto param = new TraceParam;
  		param.Start = origin - forward*farPlane;
  		param.End = origin + forward*farPlane;
  		param.Flags = TraceFlags.WORLD;
			if (!TerrainOnly)
				param.Flags |= TraceFlags.ENTS;
  		param.Exclude = this;
  
		float frac = GetWorld().TraceMove(param, NULL);
	
		#ifdef WORKBENCH
			m_bCheckIntersection = false;
		#endif
		
		if (param.TraceEnt)
		{
			World world = GetWorld();
			Decal decal = world.CreateDecal(param.TraceEnt, param.Start, forward, NearPlane, FarPlane, Angle* Math.DEG2RAD, GetSize(), Stretch, Mat, Lifetime, 0xFFFFFFFF, 0);
			
			#ifdef WORKBENCH
				m_bCheckIntersection = true;
				m_vIntersection = param.Start + (param.End - param.Start)*frac;
			
				//save always when we are in editor mode
				if ((_WB_GetEditorAPI() && _WB_GetEditorAPI().IsEditMode()) || Lifetime == 0)
			#else
				if (Lifetime == 0)
			#endif
					m_decal = decal;
				else
					m_decal = null
		}
	}
}