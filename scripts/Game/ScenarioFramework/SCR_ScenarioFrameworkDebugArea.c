[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkDebugArea : ScriptAndConfig
{
	[Attribute(desc: "Name of the Area which will be selected to spawn. Careful as it is case-sensitive!", category: "Debug")];
	string m_sForcedArea;
	
	[Attribute(desc: "Name of the Layer Task that will get spawned under attached Area. Careful as it is case-sensitive! If left empty, Layer Task will be randomly selected", category: "Debug")];
	string m_sForcedLayerTask;
	
	[Attribute(desc: "Name of the Slot Task that will get spawned under attached Area and Layer Task. Careful as it is case-sensitive! If left empty, Slot Task will be randomly selected", category: "Debug")];
	string m_sForcedSlotTask;

	//------------------------------------------------------------------------------------------------
	//! \return Forced area entity component representing scenario framework area.
	SCR_ScenarioFrameworkArea GetForcedArea()
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sForcedArea))
			return null;
		
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sForcedArea);
		if (!entity)
			return null;
		
		return SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Forced layer task entity reference.
	SCR_ScenarioFrameworkLayerTask GetForcedLayerTask()
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sForcedLayerTask))
			return null;
		
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sForcedLayerTask);
		if (!entity)
			return null;
		
		return SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Forced slot task entity reference.
	SCR_ScenarioFrameworkSlotTask GetForcedSlotTask()
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sForcedSlotTask))
			return null;
		
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sForcedSlotTask);
		if (!entity)
			return null;
		
		return SCR_ScenarioFrameworkSlotTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkSlotTask));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Represents forced area, layer task, and slot task information for an entity.
	string GetWholeInfo()
	{
		return string.Format("%1 - %2 - %3", m_sForcedArea, m_sForcedLayerTask, m_sForcedSlotTask);
	}
}