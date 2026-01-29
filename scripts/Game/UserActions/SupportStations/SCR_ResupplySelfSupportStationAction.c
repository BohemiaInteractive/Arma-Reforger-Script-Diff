class SCR_ResupplySelfSupportStationAction : SCR_BaseResupplySupportStationAction
{
	[Attribute("1", desc: "Check if magazines or projectiles in owner storage. If not it will not allow to resupply. Used in tandum with arsenals")]
	protected bool m_bCheckIfItemInStorage;
	
	[Attribute("0", desc: "If true allows to get storage manager from parent if not on self. Storagemanager is used to check if magazine is in the storage and is used in tandum with arsenal. Ignored if m_bCheckIfMagazineInStorage is false.")]
	protected bool m_bAllowGetStorageFromParent;
	
	protected InventoryStorageManagerComponent m_InventoryManagerProvider;
	
	//------------------------------------------------------------------------------------------------
	protected override void SetTargetInventory(IEntity user, IEntity owner)
	{
		m_InventoryManagerTarget = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
	}
		
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{	
		if (!super.CanBeShownScript(user))
			return false; 
		
		bool canPerform = super.CanBePerformedScript(user);
		if (!canPerform && GetShowButDisabled())
			return true;
		
		return canPerform;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{	
		return !GetShowButDisabled();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override InventoryStorageManagerComponent GetProviderInventory()
	{
		return m_InventoryManagerProvider;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void ResetReferencesOnServer()
	{
		m_InventoryManagerTarget = null;
		super.ResetReferencesOnServer();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		//~ Get storage manager to check if ammo is in storage (Should be used only if arseneal)
		if (m_bCheckIfItemInStorage)
		{
			m_InventoryManagerProvider = InventoryStorageManagerComponent.Cast(pOwnerEntity.FindComponent(InventoryStorageManagerComponent));
			if (!m_InventoryManagerProvider && m_bAllowGetStorageFromParent && pOwnerEntity.GetParent())
				m_InventoryManagerProvider = InventoryStorageManagerComponent.Cast(pOwnerEntity.GetParent().FindComponent(InventoryStorageManagerComponent));
			
			if (!m_InventoryManagerProvider)
				Print(string.Format("'SCR_ResupplySelfSupportStationAction': %1 should check if Item is in storage but no storage manager found so this step will be ignored!", pOwnerEntity.GetName()), LogLevel.ERROR);
		}
	}
}
