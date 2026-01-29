class SCR_AIFindItemInInventory : AITaskScripted
{
	// Inputs
	protected static const string PORT_PREFAB_RESOURCE_NAME = "PrefabResourceName";
	
	// Outputs
	protected static const string PORT_ITEM_ENTITY = "ItemEntity";
	
	//------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		// Read inputs
		ResourceName prefabResourceName;
		
		GetVariableIn(PORT_PREFAB_RESOURCE_NAME, prefabResourceName);
		
		if (prefabResourceName.IsEmpty())
			return ENodeResult.FAIL;
		
		// Verify own inventory
		IEntity myEntity = owner.GetControlledEntity();
		if (!myEntity)
			return ENodeResult.FAIL;
		InventoryStorageManagerComponent myInvComp = InventoryStorageManagerComponent.Cast(myEntity.FindComponent(InventoryStorageManagerComponent));
		if (!myInvComp)
			return ENodeResult.FAIL;
		
		SCR_PrefabNamePredicate searchPredicate = new SCR_PrefabNamePredicate();
		searchPredicate.prefabName = prefabResourceName;
		
		IEntity item = myInvComp.FindItem(searchPredicate);
		
		if (!item)
			return ENodeResult.FAIL;
		
		SetVariableOut(PORT_ITEM_ENTITY, item);
				
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------------
	protected ref TStringArray s_aVarsIn = { PORT_PREFAB_RESOURCE_NAME };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected ref TStringArray s_aVarsOut = { PORT_ITEM_ENTITY };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override static bool VisibleInPalette() { return true; }
}