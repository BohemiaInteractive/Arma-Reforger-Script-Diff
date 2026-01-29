class SCR_AIDecoBallisticPath : DecoratorScripted
{
	// Inputs
	protected static const string PORT_DISTANCE_COMPENSATION = "DistanceCompensation";
	protected static const string PORT_TARGET_POSITION = "TargetPos";
	
	protected ref TraceParam m_TraceParam;
	
#ifdef WORKBENCH
	protected ref array<ref Shape> m_aDebugShapes = {};
#endif
	
	//--------------------------------------------------------------------------------
	//override void OnInit(AIAgent owner) { }
	
	//--------------------------------------------------------------------------------
	override bool TestFunction(AIAgent owner)
	{
		SCR_ChimeraCharacter myCharacter = SCR_ChimeraCharacter.Cast(owner.GetControlledEntity());
		if (!myCharacter)
			return false;
		
		vector targetPos;
		GetVariableIn(PORT_TARGET_POSITION, targetPos);
		
		vector distanceCompensation;
		GetVariableIn(PORT_DISTANCE_COMPENSATION, distanceCompensation);
		
		vector traceFrom = myCharacter.EyePosition(); // todo For now it only works for character, and not for turrets!
		vector targetPosCompensated = targetPos + distanceCompensation;
		vector traceTo = 0.5*(traceFrom + targetPosCompensated);
		
		
		if (!m_TraceParam)
			m_TraceParam = new TraceParam();
		
		m_TraceParam.Start = traceFrom;
		m_TraceParam.End = traceTo;
		m_TraceParam.Exclude = myCharacter;
		m_TraceParam.LayerMask = EPhysicsLayerDefs.Projectile;
		m_TraceParam.Flags = TraceFlags.ENTS | TraceFlags.OCEAN | TraceFlags.WORLD | TraceFlags.ANY_CONTACT;
		
		float traceResult = GetGame().GetWorld().TraceMove(m_TraceParam, null);
		bool traceHit = traceResult != 1.0;
		
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_DEBUG_SHAPES))
		{
			m_aDebugShapes.Clear();
			
			int lineColor;
			if (!traceHit)
				lineColor = Color.GREEN;
			else
				lineColor = Color.RED;
			
			vector lineVerts[2];
			lineVerts[0] = m_TraceParam.Start;
			lineVerts[1] = m_TraceParam.End;
			Shape lineShape = Shape.CreateLines(lineColor, ShapeFlags.DEFAULT, lineVerts, 2);
			m_aDebugShapes.Insert(lineShape);
			
			if (traceHit)
			{
				vector hitPos = m_TraceParam.Start + traceResult * (m_TraceParam.End - m_TraceParam.Start);
				Shape sphereShape = Shape.CreateSphere(Color.RED, ShapeFlags.DEFAULT, hitPos, 0.2);
				m_aDebugShapes.Insert(sphereShape);
			}
		}
#endif
		
		return !traceHit;
	}
	
	//--------------------------------------------------------------------------------
	static override string GetOnHoverDescription() { return "Estimates if projectile ballistic path can reach destination. It is performance heavy, use with some timer."; }
	static override bool VisibleInPalette() { return true; }
	
	protected static ref TStringArray s_aVarsIn = { PORT_DISTANCE_COMPENSATION, PORT_TARGET_POSITION };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
}