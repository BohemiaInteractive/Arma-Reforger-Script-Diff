//------------------------------------------------------------------------------------------------
class SCR_PlacementCallback : ScriptedInventoryOperationCallback
{
	RplId m_PlaceableId;
	RplId m_TargetId;
	int m_iNodeId;
	bool m_bIsBeingAttachedToEntity;
	ref ScriptedInventoryOperationCallback m_ChainedCallback;
	
	//------------------------------------------------------------------------------------------------
	protected override void OnComplete()
	{
		SCR_ItemPlacementComponent placementComponent = SCR_ItemPlacementComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ItemPlacementComponent));
		if (!placementComponent)
			return;

		placementComponent.AskPlaceItem(m_PlaceableId, m_TargetId, m_iNodeId, m_bIsBeingAttachedToEntity);

		super.OnComplete();

		if (m_ChainedCallback)
			m_ChainedCallback.OnComplete();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnFailed()
	{
		super.OnFailed();

		if (m_ChainedCallback)
			m_ChainedCallback.OnFailed();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnAllComplete()
	{
		super.OnAllComplete();

		if (m_ChainedCallback)
			m_ChainedCallback.OnAllComplete();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnAllFailed()
	{
		super.OnAllFailed();

		if (m_ChainedCallback)
			m_ChainedCallback.OnAllFailed();
	}
}