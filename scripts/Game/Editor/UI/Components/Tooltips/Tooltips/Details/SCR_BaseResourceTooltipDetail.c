[BaseContainerProps(), BaseContainerCustomStringTitleField("DO NOT USE")]
class SCR_BaseResourceTooltipDetail : SCR_EntityTooltipDetail
{
	[Attribute()]
	protected LocalizedString m_sResourceFormatting;
	
	[Attribute(EResourceType.SUPPLIES.ToString(), uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EResourceType))]
	protected EResourceType m_eResourceType;
	
	protected TextWidget m_wText;
	
	protected SCR_ResourceConsumer m_ResourceConsumer;
	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandleConsumer;
	
	//------------------------------------------------------------------------------------------------
	override bool NeedUpdate()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Temp solution until replication fixed
	protected void TEMP_OnInteractorReplicated()
	{
		m_ResourceConsumer = GetConsumer();
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceConsumer GetConsumer()
	{
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		super.UpdateDetail(entity);
		
		if (!m_ResourceConsumer)
			return;
		
		if (m_ResourceSubscriptionHandleConsumer)
			m_ResourceSubscriptionHandleConsumer.Poke();
		else
			m_ResourceSubscriptionHandleConsumer = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandleGraceful(m_ResourceConsumer, Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent))));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_wText = TextWidget.Cast(widget);
		if (!m_wText)
			return false;
		
		m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(entity.GetOwner());
		if (!m_ResourceComponent)
			return false;
		
		if (!IsValid())
		{
			m_ResourceComponent = null;
			return false;
		}
		
		m_ResourceConsumer = GetConsumer();
		if (!m_ResourceConsumer)
		{
			m_ResourceComponent = null;
			return false;
		}
		
		m_ResourceComponent.TEMP_GetOnInteractorReplicated().Insert(TEMP_OnInteractorReplicated);
		
		UpdateDetail(entity);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsValid()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_BaseResourceTooltipDetail()
	{
		if (!m_ResourceComponent)
			return;
		
		m_ResourceComponent.TEMP_GetOnInteractorReplicated().Remove(TEMP_OnInteractorReplicated);
		m_ResourceComponent = null;
	}
}
