class SCR_AIGetRandomPointInCircularSector: AITaskScripted
{
	static const string ORIGIN_ENTITY_PORT		= "OriginEntity";
	static const string SECTOR_AXIS_POINT_PORT	= "SectorAxisPoint";
	static const string RANGE_PORT				= "AngularRange";
	static const string POSITION_OUT_PORT		= "PositionOut";
	
	[Attribute("180", UIWidgets.EditBox, "Limit of random angle +/-" )]
	protected float m_fAngularRange;
	
	[Attribute("1", UIWidgets.CheckBox, "Restrict to circumference" )]
	protected bool m_bReturnCircumference;
	
	[Attribute("0", UIWidgets.CheckBox, "Draw cone when point is generated?" )]
	protected bool m_bDebugMe;
	
#ifdef WORKBENCH
	protected ref array<ref Shape> m_aShapes = {};
#endif	
	
	//-------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() 
	{	
		return true;
	}
	
	//-------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		ORIGIN_ENTITY_PORT,
		SECTOR_AXIS_POINT_PORT,
		RANGE_PORT
	};
	
	//-------------------------------------------------------------------------------------------------
	override array<string> GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//-------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		POSITION_OUT_PORT
	};
	
	//-------------------------------------------------------------------------------------------------
	override array<string> GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//-------------------------------------------------------------------------------------------------
    override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		vector sectorAxis, sectorAxisPoint, sectorOrigin, result;
		float range, angle, wantedMaxLength, resX, resZ;
		IEntity controlledEntity;
		
		if (!GetVariableIn(ORIGIN_ENTITY_PORT,controlledEntity))
		{
			AIGroup gr = AIGroup.Cast(owner);
			if(!gr)
				controlledEntity = owner.GetControlledEntity();
			else
				controlledEntity = gr.GetLeaderEntity();
		};
		
		if (!controlledEntity)
			return NodeError(this, owner, "No entity for origin of vector provided!");
		
		sectorOrigin = controlledEntity.GetOrigin();
		
		if (!GetVariableIn(SECTOR_AXIS_POINT_PORT, sectorAxisPoint))
			return NodeError(this, owner, "Not provided cone axis position!");
		
		sectorAxis = sectorAxisPoint - sectorOrigin; // main diractional vector (axis of the cone)
		
		AIWaypoint wp = AIWaypoint.Cast(controlledEntity);
		if (wp)
		{
			wantedMaxLength = Math.Max(wp.GetCompletionRadius(), 1.0);		
		}
		else
		{
			wantedMaxLength = sectorAxis.Length();
		};
		
		if(!GetVariableIn(RANGE_PORT, range))
			range = m_fAngularRange;
		range *= Math.DEG2RAD;
		
		angle = Math.Atan2(sectorAxis[2],sectorAxis[0]);
		
		if (!m_bReturnCircumference)
		{
			SCR_Math2D.GetRandomPointInSector(sectorOrigin[0],sectorOrigin[2], angle - range, angle + range, wantedMaxLength, resX, resZ);
		}
		else
		{
			angle += Math.RandomFloat(-range, range);
			SCR_Math2D.PolarToCartesian(angle, wantedMaxLength, resX, resZ);
			resX += sectorOrigin[0];
			resZ += sectorOrigin[2];
		}	
		
		result[0] = resX;
		result[1] = sectorAxisPoint[1];
		result[2] = resZ;
		
		SetVariableOut(POSITION_OUT_PORT, result);
		
#ifdef WORKBENCH
			if (m_bDebugMe && DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_DEBUG_SHAPES))
			{
				vector vecDebug = sectorOrigin;
				float x,y, angleDebug;
			
				m_aShapes.Clear();				
				m_aShapes.Insert(Shape.CreateSphere(COLOR_RED_A, ShapeFlags.NOZBUFFER, result, 0.5));			
				angleDebug = Math.Atan2(sectorAxis[2],sectorAxis[0]) - range;				
				SCR_Math2D.PolarToCartesian(angleDebug,wantedMaxLength,x,y);
				vecDebug[0] = vecDebug[0] + x;
				vecDebug[2] = vecDebug[2] + y;
				m_aShapes.Insert(Shape.CreateArrow(sectorOrigin, vecDebug, 0.1, COLOR_RED_A, ShapeFlags.NOZBUFFER));	
				vecDebug = sectorOrigin;	
				angleDebug = Math.Atan2(sectorAxis[2],sectorAxis[0]) + range;
				SCR_Math2D.PolarToCartesian(angleDebug,wantedMaxLength,x,y);
				vecDebug[0] = vecDebug[0] + x;
				vecDebug[2] = vecDebug[2] + y;
				m_aShapes.Insert(Shape.CreateArrow(sectorOrigin, vecDebug, 0.1, COLOR_RED_A, ShapeFlags.NOZBUFFER));
			}
#endif		
			
		return ENodeResult.SUCCESS;
	}
	
	//-------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Provide either entity or owner entity as origin of the circular sector. Provide another point on the axis of the sector.\nReturns either point inside the sector or on circumference depending on attribute.";
	}
};

