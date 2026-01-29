//! TODO MARIO: Rework this class to work with multiple containers!!.
class SCR_ResourceContainerStoreAction : ScriptedUserAction
{
	[Attribute(defvalue: EResourceType.SUPPLIES.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sets the type of Resource to be used.\nOnly a transaction matching Resource types can be successfully concluded.", enums: ParamEnumArray.FromEnum(EResourceType))]
	protected EResourceType m_eResourceType;
	
	//~protected static SCR_CharacterResourceRequesterComponent s_CharacterResourceRequester;
	protected SCR_ResourceGenerator m_ResourceGenerator;
	protected SCR_ResourceConsumer m_ResourceConsumer;
	protected float m_fMaxStoredResource;
	protected float m_fCurrentResource;
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if (!gamemode || !gamemode.IsMaster())
			return;
		
		m_fCurrentResource		= m_ResourceConsumer.GetAggregatedResourceValue();
		m_fMaxStoredResource	= Math.Min(m_ResourceGenerator.GetAggregatedMaxResourceValue() - m_ResourceGenerator.GetAggregatedResourceValue(), m_fCurrentResource);
		
		m_ResourceConsumer.RequestConsumtion(m_fMaxStoredResource);
		m_ResourceGenerator.RequestGeneration(m_fMaxStoredResource);
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		if (!playerController)
			return;
		
		SCR_ResourcePlayerControllerInventoryComponent resourcePlayerControllerComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(playerController.FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (!resourcePlayerControllerComponent)
			return;
		
		resourcePlayerControllerComponent.OnPlayerInteraction(EResourcePlayerInteractionType.STORAGE, m_ResourceConsumer.GetComponent(), m_ResourceGenerator.GetComponent(), m_eResourceType, m_fMaxStoredResource);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		/*
		if (m_fMaxStoredResource > 0 && m_fMaxStoredResource < m_fCurrentResource)
			outName = string.Format("Store %1/%2 resources", m_fMaxStoredResource, m_fCurrentResource);
		else
			outName = string.Format("Store %1 resources", m_fMaxStoredResource);
		
		return true;
		*/
		
		if (m_fMaxStoredResource > 0 && m_fMaxStoredResource < m_fCurrentResource)
			outName = WidgetManager.Translate(GetUIInfo().GetName(), m_fMaxStoredResource, m_fCurrentResource);
		else
			outName = WidgetManager.Translate(GetUIInfo().GetDescription(), m_fMaxStoredResource);
		
		return true;
		
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool CanBePerformedScript(IEntity user)
	{
		SCR_ResourceComponent component = SCR_ResourceComponent.Cast(GetOwner().FindComponent(SCR_ResourceComponent));
		
		if (!component)
			return false;
		
		m_ResourceGenerator	= component.GetGenerator(EResourceGeneratorID.DEFAULT_STORAGE, m_eResourceType);
		m_ResourceConsumer	= component.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, m_eResourceType);
		
		if (!m_ResourceGenerator || !m_ResourceConsumer)
		{
			SetCannotPerformReason(WidgetManager.Translate("#AR-Supplies_CannotPerform_Generic"));
			
			return false;
		}
		
		m_fCurrentResource		= m_ResourceConsumer.GetAggregatedResourceValue();
		m_fMaxStoredResource	= Math.Min(m_ResourceGenerator.GetAggregatedMaxResourceValue() - m_ResourceGenerator.GetAggregatedResourceValue(), m_fCurrentResource);
		
		if (m_ResourceGenerator.GetContainerCount() == 0)
			SetCannotPerformReason("#AR-Supplies_CannotPerform_Container_NoStorage");
		else if (m_fMaxStoredResource == 0.0)
			SetCannotPerformReason("#AR-Supplies_CannotPerform_Container_StorageFull");
		
		return m_fMaxStoredResource > 0.0 && m_fMaxStoredResource <= m_fCurrentResource;
	}
	
	//------------------------------------------------------------------------------------------------
	override event void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		SCR_ResourceComponent component = SCR_ResourceComponent.Cast(pOwnerEntity.FindComponent(SCR_ResourceComponent));
		
		if (!component)
			return;
		
		m_ResourceGenerator	= component.GetGenerator(EResourceGeneratorID.DEFAULT_STORAGE, m_eResourceType);
		m_ResourceConsumer	= component.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, m_eResourceType);
	}
}