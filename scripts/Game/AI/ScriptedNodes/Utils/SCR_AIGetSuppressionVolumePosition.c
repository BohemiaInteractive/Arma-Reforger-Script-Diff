
class SCR_AIGetSuppressionVolumeCenterPosition : AITaskScripted
{
	protected static const string CENTER_POS_PORT = "CenterPosition";
	protected static const string DISTANCE_PORT = "Distance_m";
		
	protected static const string SUPPRESSION_VOLUME = "SuppressionVolume";
	
	protected ref TStringArray s_aVarsOut = {CENTER_POS_PORT,DISTANCE_PORT};
	protected ref TStringArray s_aVarsIn = {SUPPRESSION_VOLUME};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
			
	static override bool VisibleInPalette() { return true; }
	
	static override string GetOnHoverDescription() { return "Returns center position of given suppression volume"; };
	
	//---------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AISuppressionVolumeBase volume;
		GetVariableIn(SUPPRESSION_VOLUME, volume);
		
		if (!volume)
			return NodeError(this, owner, "No suppression volume provided!");
			
		vector centerPos = volume.GetCenterPosition();
		SetVariableOut(CENTER_POS_PORT, centerPos);
		
		float distance;
		IEntity m_CharacterEntity = owner.GetControlledEntity();
		if (m_CharacterEntity)
			distance = vector.Distance(m_CharacterEntity.GetOrigin(), centerPos);
		
		SetVariableOut(DISTANCE_PORT, distance);
		
		return ENodeResult.SUCCESS;
	};
}

class SCR_AIGetSuppressionVolumeLine : AITaskScripted
{
	protected static const string SUPPRESSION_VOLUME_PORT = "SuppressionVolume";
	protected static const string START_IN_POS_PORT = "StartPositionIn";
	protected static const string END_IN_POS_PORT = "EndPositionIn";
	protected static const string PROGRESS_IN_PORT = "ProgressIn";
	
	protected static const string START_OUT_POS_PORT = "StartPositionOut";
	protected static const string END_OUT_POS_PORT = "EndPositionOut";
	protected static const string SUPPRESS_TIME_PORT = "SuppressionTimeSOut";
	protected static const string AIM_TIME_PORT = "AimTimeSOut";
	
	protected ref TStringArray s_aVarsOut = {START_OUT_POS_PORT,END_OUT_POS_PORT,SUPPRESS_TIME_PORT,AIM_TIME_PORT};
	protected ref TStringArray s_aVarsIn = {SUPPRESSION_VOLUME_PORT,START_IN_POS_PORT,END_IN_POS_PORT,PROGRESS_IN_PORT};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
			
	static override bool VisibleInPalette() { return true; }
	
	static override string GetOnHoverDescription() { return "Returns start and end positions of suppression line based on given volume. If previous line is provided, new line will try to start where previous line ended"; };
	
#ifdef WORKBENCH
	//Diagnostic visualization
	ref array<ref Shape> m_aDbgShapes = {};
#endif	
	
	//---------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AISuppressionVolumeBase volume;
		GetVariableIn(SUPPRESSION_VOLUME_PORT, volume);
		
		if (!volume)
			return NodeError(this, owner, "No suppression volume provided!");
		
		// Get character entity
		IEntity m_CharacterEntity = owner.GetControlledEntity();
		if (!m_CharacterEntity)
			return ENodeResult.FAIL;
		
		vector startPos, prevStartPos, prevEndPos, prevDir;
		
		// Use starting position if provided
		if (GetVariableIn(START_IN_POS_PORT, prevStartPos) && prevStartPos != vector.Zero)
		{
			// Whole previous line provided, use interpolated pos based on progress
			if (GetVariableIn(END_IN_POS_PORT, prevEndPos) && prevEndPos != vector.Zero)
			{
				prevDir = vector.Direction(prevStartPos, prevEndPos).Normalized();
				
				float progress;
				GetVariableIn(PROGRESS_IN_PORT, progress); // If not connected, we assume 0 anyway
							
				startPos = prevStartPos + (prevEndPos - prevStartPos) * progress;
			}
			else
				startPos = prevStartPos;
		}
		
		// No start pos or small chance (more aggressive suppression if we skip line from time to time)
		if (startPos == vector.Zero || Math.RandomFloat01() > 0.85)
		{
			// Get start pos based on simulated first run
			vector tmpStPos;
			startPos = volume.GetRandomPosition(m_CharacterEntity, tmpStPos);
		}
		
		vector endPos = volume.GetRandomPosition(m_CharacterEntity, startPos, prevDir);
		
		float distancePerDeg = Math.Tan(Math.DEG2RAD) * vector.Distance(m_CharacterEntity.GetOrigin(), volume.GetCenterPosition());	
		float degsToTurn = vector.Distance(startPos, endPos) / distancePerDeg;
		float suppressionTime = degsToTurn * (0.3 + Math.RandomFloatInclusive(0, 1.1));
		float aimTime = Math.Max(0.02, Math.RandomGaussFloat(0.236, 0.2));
		
#ifdef WORKBENCH
		m_aDbgShapes.Clear();
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_SUPPRESS_DEBUG))
		{
			m_aDbgShapes.Insert(Shape.CreateSphere(Color.RED, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, startPos, 0.1));
			m_aDbgShapes.Insert(Shape.CreateSphere(Color.YELLOW, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, endPos, 0.1));
			m_aDbgShapes.Insert(Shape.CreateArrow(startPos, endPos, 1, Color.BLUE, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE));
		}
#endif
		
		SetVariableOut(START_OUT_POS_PORT, startPos);
		SetVariableOut(END_OUT_POS_PORT, endPos);
		SetVariableOut(SUPPRESS_TIME_PORT, suppressionTime);
		SetVariableOut(AIM_TIME_PORT, aimTime);
		
		return ENodeResult.SUCCESS;
	}
}