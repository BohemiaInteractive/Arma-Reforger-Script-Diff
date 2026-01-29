class SCR_ResourceStorageOpenAction : SCR_OpenStorageAction
{
	[Attribute(defvalue: EResourceType.SUPPLIES.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sets the type of Resource to be used.\nOnly a transaction matching Resource types can be successfully concluded.", enums: ParamEnumArray.FromEnum(EResourceType))]
	protected EResourceType m_eResourceType;
	protected IEntity m_TopMostOwner;
	
	//------------------------------------------------------------------------------------------------
	override event bool CanBePerformedScript(IEntity user)
	{
		m_TopMostOwner = GetOwner();
		IEntity topOwner = m_TopMostOwner.GetParent();
		
		while (topOwner)
		{
			SCR_ResourceComponent component = SCR_ResourceComponent.Cast(topOwner.FindComponent(SCR_ResourceComponent));
			
			if (component && component.GetEncapsulator(m_eResourceType))
				m_TopMostOwner = topOwner;
			
			topOwner = topOwner.GetParent();
		}
		
		return super.CanBePerformedScript(user);
	}
	
	#ifndef DISABLE_INVENTORY
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_TopMostOwner = pOwnerEntity;
		IEntity topOwner = m_TopMostOwner.GetParent();
		
		while (topOwner)
		{
			SCR_ResourceComponent component = SCR_ResourceComponent.Cast(topOwner.FindComponent(SCR_ResourceComponent));
			
			if (component && component.GetEncapsulator(m_eResourceType))
				m_TopMostOwner = topOwner;
			
			topOwner = topOwner.GetParent();
		}
		
		SCR_InventoryStorageManagerComponent manager2 =  SCR_InventoryStorageManagerComponent.Cast(m_TopMostOwner.FindComponent( SCR_InventoryStorageManagerComponent ));
		
		if ( !manager2 )
			return;
		
		super.PerformActionInternal(manager2, m_TopMostOwner, pUserEntity);
	}
	#endif
	
	//------------------------------------------------------------------------------------------------
	override event void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{	
		m_TopMostOwner = pOwnerEntity;
		IEntity topOwner = pOwnerEntity.GetParent();
		
		while (topOwner)
		{
			SCR_ResourceComponent component = SCR_ResourceComponent.Cast(topOwner.FindComponent(SCR_ResourceComponent));
			
			if (component && component.GetEncapsulator(m_eResourceType))
				m_TopMostOwner = topOwner;
			
			topOwner = topOwner.GetParent();
		}
	}
}