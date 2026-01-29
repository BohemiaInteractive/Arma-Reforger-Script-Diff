class SCR_ReplaceWheelUserAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.Auto, "The index of the tire this action should be activated on", category: "")]
	protected int m_iTireIndex;
	
	#ifdef ENABLE_DESTRUCTION
	protected SCR_TireReplacementManagerComponent m_ReplacementManager;
	
	//------------------------------------------------------------------------------------------------	
	override bool CanBeShownScript(IEntity user)
	{
		if(!m_ReplacementManager)
			return false;
		
		return m_ReplacementManager.CanTireBeReplaced(m_iTireIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if(!m_ReplacementManager)
			return false;
		
		//TODO: notify user somehow when area is not clear
		return m_ReplacementManager.IsWheelAreaClear(m_iTireIndex, user);
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if(!m_ReplacementManager)
			return;
		
		m_ReplacementManager.InitReplace(m_iTireIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{		
		GenericEntity ownerEntity = GenericEntity.Cast(pOwnerEntity);
		m_ReplacementManager = SCR_TireReplacementManagerComponent.Cast(ownerEntity.FindComponent(SCR_TireReplacementManagerComponent));
	}	
	#endif
};