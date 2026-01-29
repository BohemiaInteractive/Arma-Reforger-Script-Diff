[ComponentEditorProps(category: "GameScripted/Area Mesh", description: "")]
class SCR_CampaignBuildingAreaMeshComponentClass : SCR_TriggerAreaMeshComponentClass
{
}

class SCR_CampaignBuildingAreaMeshComponent : SCR_TriggerAreaMeshComponent
{
	[Attribute("10")]
	protected float m_fBufferZone;
	
	[Attribute("false")]
	protected bool m_bShouldEnableFrameUpdateDuringEditor;
	
	//------------------------------------------------------------------------------------------------
	protected float GetBufferZone()
	{
		return m_fBufferZone;
	}
	
	//------------------------------------------------------------------------------------------------
	override float GetRadius()
	{
		BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(GetOwner());
		if (!trigger)
			return 0;
		
		return trigger.GetSphereRadius() + GetBufferZone();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] val
	void EnableFrameUpdateDuringEditor(bool val)
	{
		m_bShouldEnableFrameUpdateDuringEditor = val;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool ShouldEnableFrameUpdateDuringEditor()
	{
		return m_bShouldEnableFrameUpdateDuringEditor;
	}
}
