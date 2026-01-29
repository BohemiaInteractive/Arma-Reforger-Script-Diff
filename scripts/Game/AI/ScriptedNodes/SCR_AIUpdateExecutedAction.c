// BT node for printing debug messsages
class SCR_AIUpdateExecutedAction : AITaskScripted
{
	private SCR_AIBaseUtilityComponent m_Utility;
	
	protected static override bool VisibleInPalette() {return true;}

	protected override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Utility)
			return ENodeResult.FAIL;

		m_Utility.SetExecutedAction(m_Utility.GetCurrentAction());
		return ENodeResult.SUCCESS;
	}
	
	override void OnEnter(AIAgent owner)
	{
		m_Utility = SCR_AIBaseUtilityComponent.Cast(owner.FindComponent(SCR_AIBaseUtilityComponent));
	}
	
	protected static override string GetOnHoverDescription()
	{
		return "Update which activity is now being executed, so BT will always receive correct params";
	}
};
