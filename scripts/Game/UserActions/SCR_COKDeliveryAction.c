class SCR_COKDeliveryAction: ScriptedUserAction
{
	[Attribute(desc: "Required Entity Prefab for interaction to be possible", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_sRequiredEntityPrefab;
	
	[Attribute(desc: "Name of signal to start and stop radio sounds")]
	protected string m_sSignalName;
	
	protected ref SCR_PrefabNamePredicate m_SearchPredicate;
	protected SignalsManagerComponent m_SignalsManagerComponent;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sRequiredEntityPrefab))
			return;
		
		m_SearchPredicate = new SCR_PrefabNamePredicate;
		m_SearchPredicate.prefabName = m_sRequiredEntityPrefab;
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(pOwnerEntity.FindComponent(SignalsManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (m_SignalsManagerComponent)
			m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal(m_sSignalName), 0);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		if (m_SignalsManagerComponent)
			m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal(m_sSignalName), 1);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return IsEntityInUserStorage(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return IsEntityInUserStorage(user);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsEntityInUserStorage(IEntity user)
	{
		if (!m_SearchPredicate)
			return false;
		
		InventoryStorageManagerComponent storageManager = InventoryStorageManagerComponent.Cast(user.FindComponent(InventoryStorageManagerComponent));
		if (!storageManager)
			return false;
		
		return storageManager.FindItem(m_SearchPredicate);
	}
}