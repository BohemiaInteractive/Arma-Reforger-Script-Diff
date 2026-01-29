[EntityEditorProps(category: "GameScripted/Campaign", description: "This component handles service (composition) operability")]
class SCR_CampaignServiceCompositionComponentClass : ScriptComponentClass
{
}

class SCR_CampaignServiceCompositionComponent : ScriptComponent
{
	//! Service operability (initial)
	[Attribute("100", UIWidgets.Slider, "Initial operability of service. If this goes to (or under) Service Unavailable value, composition is not providing it's service", "0 100 1")]; 
	protected int m_iServiceOperability;
	
	//! Service operability limit
	[Attribute("0", UIWidgets.Slider, "Service is unavailable when operability drops to this value (or bellow)", "0 100 1")]; 
	protected int m_iServiceUnavailable;
	
	protected SCR_SiteSlotEntity m_Slot;
	protected static const int SLOT_SEARCH_DISTANCE = 5;
	protected IEntity m_Owner;
	protected SCR_CampaignMilitaryBaseComponent m_Base;
	protected EEditableEntityLabel m_CompositionType;
	protected SCR_ServicePointComponent m_Service;
	
	ref ScriptInvoker Event_EOnServiceDisabled = new ScriptInvoker();
	ref ScriptInvoker Event_EOnServiceRepaired = new ScriptInvoker();
	
	protected static const bool AVAILABLE = true;
	protected static const bool UNAVAILABLE = false;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_Owner = owner;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] operability
	void ChangeOperability(int operability)
	{
		// First check whether the Service is operable now or not.
		bool isOperableBeforeChange = IsServiceOperable();
		
		// Reduce operability
	 	m_iServiceOperability -= operability;
		
		SCR_ServicePointMapDescriptorComponent desc = SCR_ServicePointMapDescriptorComponent.Cast(m_Owner.FindComponent(SCR_ServicePointMapDescriptorComponent));
		
		// Check if the Service was disabled.
		if (isOperableBeforeChange && !IsServiceOperable())
		{
			if (desc)
				desc.SetServiceMarkerActive(UNAVAILABLE);
			
			Event_EOnServiceDisabled.Invoke();
		}
		
		// Check if the Service was recovered from disabled state.
		if (!isOperableBeforeChange && IsServiceOperable())
		{
			if (desc)
				desc.SetServiceMarkerActive(AVAILABLE);
			
			Event_EOnServiceRepaired.Invoke();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] type
	void SetCompositionType(EEditableEntityLabel type)
	{
		m_CompositionType = type;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] service
	void SetService(SCR_ServicePointComponent service)
	{
		m_Service = service;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EEditableEntityLabel GetCompositionType()
	{
		return m_CompositionType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ServicePointComponent GetService()
	{
		return m_Service;
	}
	
	//------------------------------------------------------------------------------------------------
	// Get the nearest slot to this composition
	protected bool GetNearestSlot(IEntity ent)
	{		
		SCR_SiteSlotEntity slotEnt = SCR_SiteSlotEntity.Cast(ent);
		if (!slotEnt)
		    return true;
						
		m_Slot = slotEnt;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsServiceOperable()
	{
		return m_iServiceOperability > m_iServiceUnavailable;
	}
}
