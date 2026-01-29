[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_PassagerTooltipDetail: SCR_EntityTooltipDetail
{
	protected TextWidget m_Text;
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;
	
	[Attribute("#AR-Editor_TooltipDetail_CompartmentFill")]
	private string m_sCompartmentFillText;
	
	//------------------------------------------------------------------------------------------------
	override bool NeedUpdate()
	{
		return m_CompartmentManager != null;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		if (!m_CompartmentManager || !m_Text)
			return;
		
		array<BaseCompartmentSlot> compartments = {};
		m_CompartmentManager.GetCompartmentsOfType(compartments, ECompartmentType.CARGO);
		
		int total = compartments.Count();
		int occupied;
		for (int i = 0; i < total; i++)
		{			
			if (compartments[i].GetOccupant()) 
				occupied++;
		}
		
		m_Text.SetTextFormat(m_sCompartmentFillText, occupied, total);
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Text = TextWidget.Cast(widget);
		if (!m_Text)
			return false;
		
		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(entity.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		
		if (!m_CompartmentManager)
			return false;
		
		array<BaseCompartmentSlot> compartments = {};
		m_CompartmentManager.GetCompartmentsOfType(compartments, ECompartmentType.CARGO);
		
		if (compartments.IsEmpty())
			return false;
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(entity.GetOwner().FindComponent(DamageManagerComponent));
		if (damageManager && damageManager.GetState() == EDamageState.DESTROYED)
			return false;
		
		return true;
	}
}
