[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_MultiOccupantVehcileTooltipDetail: SCR_EntityTooltipDetail
{
	protected SCR_MultiTextTooltipUIComponent m_MultiLineTextWidget
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;
	
	protected PlayerManager m_PlayerManager;
	
	[Attribute("#AR-Editor_TooltipDetail_Compartment_Empty")]
	private string m_sEmptyDriverText;
	
	[Attribute("#AR-Editor_TooltipDetail_Compartment_Player")]
	private string m_sPlayerText;
	
	[Attribute("0", UIWidgets.ComboBox, "Compartment type to Check", "The type of compartment that this tooltip checks to see if they are filled", ParamEnumArray.FromEnum(ECompartmentType) )]
	protected ECompartmentType m_iCompartmentTypeToCheck;
	
	//------------------------------------------------------------------------------------------------
	override bool NeedUpdate()
	{
		return m_CompartmentManager != null;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		if (!m_CompartmentManager || !m_MultiLineTextWidget)
			return;
		
		m_MultiLineTextWidget.ClearAllText();
		
		array<BaseCompartmentSlot> compartments = {};
		m_CompartmentManager.GetCompartmentsOfType(compartments, m_iCompartmentTypeToCheck);
		int total = compartments.Count();
		
		IEntity occupant;
		SCR_EditableEntityComponent editableGunner;
		
		for (int i = 0; i < total; i++)
		{
			occupant = compartments[i].GetOccupant();
			
			if (!occupant)
			{
				m_MultiLineTextWidget.AddText(m_sEmptyDriverText);
			}
			else 
			{
				editableGunner = SCR_EditableEntityComponent.GetEditableEntity(occupant);
			
				int playerID = editableGunner.GetPlayerID();
				
				if (playerID <= 0 || !m_PlayerManager)
					m_MultiLineTextWidget.AddText(editableGunner.GetDisplayName());
				else
					m_MultiLineTextWidget.AddText(m_sPlayerText, SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID));
			}
		}	
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_MultiLineTextWidget = SCR_MultiTextTooltipUIComponent.Cast(widget.FindHandler(SCR_MultiTextTooltipUIComponent));
		if (!m_MultiLineTextWidget)
			return false;

		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(entity.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		
		if (!m_CompartmentManager)
			return false;
		
		array<BaseCompartmentSlot> compartments = {};
		m_CompartmentManager.GetCompartmentsOfType(compartments, m_iCompartmentTypeToCheck);
		
		if (compartments.IsEmpty())
			return false;
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(entity.GetOwner().FindComponent(DamageManagerComponent));
		if (damageManager && damageManager.GetState() == EDamageState.DESTROYED)
			return false;
		
		m_PlayerManager = GetGame().GetPlayerManager();

		return true;
	}
}
