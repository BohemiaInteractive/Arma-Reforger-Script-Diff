//------------------------------------------------------------------------------------------------
//! SCR_Physics Class
//!
//! Contains various physics functions
//------------------------------------------------------------------------------------------------
class SCR_PhysicsHelper
{
	static ref array<ref Shape> m_aDebugShapes = {};
	
	protected vector m_vCurrentStartPosition;
	protected vector m_vCurrentEndPosition;
	protected vector m_vOriginalEnd;
	protected vector m_vOriginalStart;
	protected vector m_vDirection;
	protected float m_fLength;
	
	protected float m_fLengthTraced;
	
	protected ref ScriptInvoker<vector, IEntity> m_OnTraceFinished;
	
	protected ref TraceParam m_TraceParam;

#ifdef ENABLE_DIAG
	protected int m_iNextDebugColor = COLOR_RED;
#endif
	
	protected const float DEFAULT_TRACE_SEGMENT_LENGTH = 100; //default length of the split-traces
	
	//------------------------------------------------------------------------------------------------
	static void InitPhysicsHelper()
	{
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_SCRIPTS_MENU, "Scripts", "Physics");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SHOW_PHYSICS_HELPER_TRACE, "", "Show helper trace", "Scripts");
	}
	
	//------------------------------------------------------------------------------------------------
	//! Iterates through a physics object's colliders and remaps the input physics interaction layer to the replace physics interaction layer
	//! Returns how many colliders were remapped
	static int RemapInteractionLayer(notnull Physics physicsObject, EPhysicsLayerDefs inputLayer, EPhysicsLayerDefs replaceLayer)
	{
		int numRemapped = 0; 
		for (int i = physicsObject.GetNumGeoms() - 1; i >= 0; i--)
		{
			int layerMask = physicsObject.GetGeomInteractionLayer(i);
			if (!(layerMask & inputLayer))
				continue;
			
			layerMask &= ~inputLayer; // Remove input layer
			layerMask |= replaceLayer; // Add replace layer
			
			physicsObject.SetGeomInteractionLayer(i, layerMask);
			numRemapped++;
		}
		
		return numRemapped;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Traces the smaller segment of longer distance, do not use on its own.
	private void TraceSegment(TraceFlags flags, EPhysicsLayerDefs layerMask, array<IEntity> excludeArray, float segmentSize)
	{
		m_fLengthTraced = m_fLengthTraced + segmentSize;
		
		//if this segment of trace would go over complete traced length we use the original
		if (m_fLengthTraced > m_fLength)
			m_TraceParam.End = m_vOriginalEnd;
		else
			m_TraceParam.End = m_vCurrentEndPosition;
		
		m_TraceParam.Start = m_vCurrentStartPosition;
		m_TraceParam.Flags = flags;
		m_TraceParam.LayerMask = layerMask;
		m_TraceParam.ExcludeArray = excludeArray;
		m_TraceParam.TraceEnt = null;
		
		float traceResult = GetGame().GetWorld().TraceMove(m_TraceParam, null);
		
		//m_TraceParam.TraceEnt is filled if the trace hit, otherwise we check if we are at the end of trace
		if (m_TraceParam.TraceEnt || traceResult == 1 && m_vOriginalEnd == m_TraceParam.End)
		{
#ifdef ENABLE_DIAG	
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SHOW_PHYSICS_HELPER_TRACE))
			{
				float distance = traceResult * (m_TraceParam.End - m_TraceParam.Start).Length();			
				m_aDebugShapes.Insert(Shape.CreateArrow(m_TraceParam.Start, distance * m_vDirection + m_TraceParam.Start , 0.3, COLOR_YELLOW, ShapeFlags.DEFAULT));
			}
#endif
			
			vector traceHitPosition;
			//calculate the position of the hit
			traceHitPosition = m_TraceParam.Start + m_vDirection * (m_TraceParam.End - m_TraceParam.Start).Length() * traceResult;	
			m_OnTraceFinished.Invoke(traceHitPosition, m_TraceParam.TraceEnt);
			GetGame().GetCallqueue().Remove(TraceSegment);
			return;
		} 
		
#ifdef ENABLE_DIAG	
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SHOW_PHYSICS_HELPER_TRACE))
		{
			float distance = traceResult * (m_TraceParam.End - m_TraceParam.Start).Length();			
			m_aDebugShapes.Insert(Shape.CreateArrow(m_TraceParam.Start, distance * m_vDirection + m_TraceParam.Start , 0.3, m_iNextDebugColor, ShapeFlags.DEFAULT));
			if (m_iNextDebugColor == COLOR_RED)
				m_iNextDebugColor = COLOR_GREEN;
			else
				m_iNextDebugColor = COLOR_RED;
		}
#endif
		
		//update vectors for the next segment to be traced
		m_vCurrentStartPosition = m_TraceParam.End;
		m_vCurrentStartPosition = m_TraceParam.End;
		m_vCurrentEndPosition = m_vOriginalStart + (m_fLengthTraced + segmentSize) * m_vDirection;
		return;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Does trace for long distance as a series of shorter traces across multiple frames, for performance improvements.
	//! Subscribe to m_OnTraceFinished<vector, IEntity> that gets invoked after the trace is done.
	//! Create new instance of SCR_PhysicsHelper when you want to do a trace.
	void TraceSegmented(vector start, vector end, TraceFlags flags = TraceFlags.DEFAULT, EPhysicsLayerDefs layerMask = EPhysicsLayerDefs.Default, array<IEntity> excludeArray = null, float segmentSize = DEFAULT_TRACE_SEGMENT_LENGTH)
	{
		//calculate and save direction and length there, so we do not have to recalculate it every frame while tracing
		m_vDirection = end - start;
		m_fLength= m_vDirection.NormalizeSize();
		
		m_TraceParam = new TraceParam();
		
		m_fLengthTraced = 0;
		
		m_vOriginalEnd = end;
		m_vOriginalStart = start;
		
		if (m_fLength < segmentSize)
			m_vCurrentEndPosition = end;
		else
			m_vCurrentEndPosition = start + m_vDirection * segmentSize;
		
		m_vCurrentStartPosition = start;
		
		//Call this method every frame to trace segment of the whole length each frame, when the trace is done the method removes itself from the queue
		GetGame().GetCallqueue().CallLater(TraceSegment, 0, true, flags, layerMask, excludeArray, segmentSize);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnTraceFinished()
	{
		if (!m_OnTraceFinished)
			m_OnTraceFinished = new ScriptInvoker();
		
		return m_OnTraceFinished;
	}
	
	//------------------------------------------------------------------------------------------------
	static void ChangeSimulationState(IEntity ent, SimulationState simState, bool recursively = false)
	{
		Physics physics = ent.GetPhysics();
		if (physics)
			physics.ChangeSimulationState(simState);
		
		if (!recursively)
			return;
		
		for (IEntity child = ent.GetChildren(); child; child = child.GetSibling())
		{
			ChangeSimulationState(child, simState, recursively);
		}
	}
	
};