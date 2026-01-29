[EntityEditorProps(category: "GameScripted/Test", description: "Entity that fires raycasts to a target", color: "0 0 255 255")]
class SCR_TestRaycasterClass: GenericEntityClass
{
};

enum ETestRaycaster
{
	LINE = 0,
	SPHERE = 1
};

//------------------------------------------------------------------------------------------------
class SCR_TestRaycaster : GenericEntity
{
	[Attribute("0", UIWidgets.ComboBox, "Type Of Raycast used", "", ParamEnumArray.FromEnum(ETestRaycaster))]
	int m_iTypeOfRaycast;

	private void DBG_Line()
	{
		vector p[2];
		p[0] = m_vRCStart;
		p[1] = m_vRCEnd;

		int shapeFlags = ShapeFlags.NOOUTLINE;
		Shape s = Shape.CreateLines(ARGBF(1, 1, 1, 1), shapeFlags, p, 2);
		m_aDbgShapes.Insert(s);	
	}
	
	private void DBG_Sphere(vector pos, int color)
	{
		vector matx[4];
		Math3D.MatrixIdentity4(matx);
		matx[3] = pos;
		int shapeFlags = ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP;
		Shape s = Shape.CreateSphere(color, shapeFlags, pos, 0.05);
		s.SetMatrix(matx);
		m_aDbgShapes.Insert(s);
	}
	
	private void DoTraceLine()
	{
		autoptr TraceParam param = new TraceParam;
		param.Exclude = this;
		param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		param.LayerMask = TRACE_LAYER_MASK;
		param.Start = m_vRCStart;
		param.End = m_vRCEnd;
		
		TraceResult(param);
	}

	private void DoTraceSphere()
	{
		autoptr TraceSphere param = new TraceSphere;
		param.Exclude = this;
		param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		param.LayerMask = TRACE_LAYER_MASK;
		param.Start = m_vRCStart;
		param.End = m_vRCEnd;
		param.Radius = 0.5;
		
		TraceResult(param);
	}

	private void TraceResult(TraceParam param)
	{
		float hit = GetWorld().TraceMove(param, null);
		
		if (!param.TraceEnt)
			return;

		Print("_____");
		Print("| " + GetName() + " results" );
		Print("|_ Entity: " + param.TraceEnt);
		Print("|_ Collider: " + param.ColliderName);
		//Print("|_ Material type: " + param.MaterialType);
		Print(" ");
		
		vector hitPos = m_vRCStart + vector.Forward * (hit * RAY_LENGTH);
		DBG_Sphere(hitPos, ARGBF(0.5, 1, 0, 0));
	}

	void DoRaycast(int typeOfRayCast)
	{
		switch (typeOfRayCast)
		{
		case ETestRaycaster.LINE:
			DoTraceLine();
		break;
		case ETestRaycaster.SPHERE:
			DoTraceSphere();
		break;
		default:
		break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fTime += timeSlice;
		
		if (m_fTime > MEASURE_INTERVAL)
		{
			m_aDbgShapes.Clear();

			DoRaycast(m_iTypeOfRaycast);

			DBG_Line();
			DBG_Sphere(m_vRCStart, ARGBF(1, 1, 1, 1));
			DBG_Sphere(m_vRCEnd, ARGBF(1, 1, 1, 1));
			
			m_fTime = 0;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_TestRaycaster(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, false);

		vector worldMat[4];
		GetWorldTransform(worldMat);

		m_vRCStart = worldMat[3];
		m_vRCEnd = m_vRCStart + vector.Forward * RAY_LENGTH;
		
		m_aDbgShapes = new array<ref Shape>;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_TestRaycaster()
	{
	}

	private const int 		TRACE_LAYER_MASK = EPhysicsLayerDefs.Projectile;
	private const float 	MEASURE_INTERVAL = 1.0;
	private const float 	RAY_LENGTH = 3.0;
	private float m_fTime;

	private vector m_vRCStart;
	private vector m_vRCEnd;
	
	//! dbg
	private ref array<ref Shape> m_aDbgShapes;
};
