class SCR_CompartmentUserAction : CompartmentUserAction
{	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{		
		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return false;
		
		UIInfo compartmentInfo = compartment.GetUIInfo();
		if (!compartmentInfo)
			return false;
		
		UIInfo actionInfo = GetUIInfo();
		if (!actionInfo)
			return false;
		
		outName = actionInfo.GetName();
		if (!compartmentInfo.GetName().IsEmpty())
			outName += "%CTX_HACK%" + compartmentInfo.GetName();
		
		return true;
	}
	

	//! TODO: Remove and replace with proper solution
	//! This is a temporary solution for handling compartment actions from different positions
	protected int m_iCompartmentDoorInfoIndex = -1;
	
	//------------------------------------------------------------------------------------------------
	void SetCompartmentDoorInfoIndex(int index)
	{
		m_iCompartmentDoorInfoIndex = index;
	}

	//------------------------------------------------------------------------------------------------
	int GetCompartmentDoorInfoIndex()
	{
		return m_iCompartmentDoorInfoIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	/* Performs door index search based on active player context, if relevant does not set
	WARNING: Does not perform null checks for compartment and caller
	*/
	protected int GetRelevantDoorIndex(IEntity caller)
	{
		if (m_iCompartmentDoorInfoIndex != -1)
		{
			return m_iCompartmentDoorInfoIndex;
		}
		PlayerController plC = GetGame().GetPlayerController();
		if (!plC || plC.GetControlledEntity() != caller)
		{
			return -1;
		}
		auto interactionHandler = InteractionHandlerComponent.Cast(plC.FindComponent(InteractionHandlerComponent));
		if (!interactionHandler)
		{
			return -1;
		}
		auto actionCtx = interactionHandler.GetCurrentContext();
		if (actionCtx)
		{
			return GetCompartmentSlot().PickDoorIndexForPoint(actionCtx.GetOrigin());
		}
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
};