class SCR_AIPickupInventoryItemsBehavior : SCR_AIBehaviorBase
{
	protected static const string PICKUP_POSITION_PORT = "PickupPosition";
	static const string MAGAZINE_WELL_TYPE_PORT = "MagazineWellType";
	
	ref SCR_BTParam<vector> m_vPickupPosition = new SCR_BTParam<vector>(PICKUP_POSITION_PORT);
	ref SCR_BTParam<typename> m_MagazineWellType = new SCR_BTParam<typename>(MAGAZINE_WELL_TYPE_PORT);
	
	ref SCR_AIMoveIndividuallyBehavior m_MoveBehavior;
	
	//-------------------------------------------------------------------------------------------------------------
	void InitProperties(vector pickupPosition, typename magazineWellType)
	{
		m_vPickupPosition.Init(this, pickupPosition);
		m_MagazineWellType.Init(this, magazineWellType);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.SELF_AID;
	}
	
	//-------------------------------------------------------------------------------------------------------------
	void SCR_AIPickupInventoryItemsBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity,
		vector pickupPosition, typename magazineWellType, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		SetPriority(SCR_AIActionBase.PRIORITY_BEHAVIOR_PICKUP_INVENTORY_ITEMS);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_sBehaviorTree = "{8522FD17F6E08C47}AI/BehaviorTrees/Chimera/Soldier/PickupInventoryItems.bt";
		
		SetIsUniqueInActionQueue(true);
		InitProperties(pickupPosition, magazineWellType);
				
		if (!utility)
			return;
		
		float movePriority = GetPriority() + 0.1;
		m_MoveBehavior = new SCR_AIMoveIndividuallyBehavior(utility, groupActivity, pickupPosition, priority: movePriority, priorityLevel: priorityLevel, radius: 0.3);
		utility.AddAction(m_MoveBehavior);
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		if (m_MoveBehavior)
			m_MoveBehavior.Complete();
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		if (m_MoveBehavior)
			m_MoveBehavior.Fail();
	}
}

class SCR_AIGetPickupInventoryItemsParameters: SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIPickupInventoryItemsBehavior(null, null, vector.Zero, BaseMagazineWell)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	static override bool VisibleInPalette() { return true; }
};