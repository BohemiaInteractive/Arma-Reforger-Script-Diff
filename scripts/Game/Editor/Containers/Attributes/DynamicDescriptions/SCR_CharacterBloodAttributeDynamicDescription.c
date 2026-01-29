/**
Holds logic when a dynamic description to show a description if value is set above or below the Unconscious level
*/
[BaseContainerProps(), BaseContainerCustomStringTitleField("Blood level warnings (CUSTOM)")]
class SCR_CharacterBloodAttributeDynamicDescription : SCR_BaseAttributeDynamicDescription
{
	[Attribute("1", desc: "If true will show the description if value is set to less or equal than unconcious level. If false will set the description if set greater than unconcious level")]
	protected bool m_bLessOrEqualThan;
	
	[Attribute("1", desc: "If true it will only show this description if unconsciousness is enabled, else it will only show if disabled")]
	protected bool m_bOnlyShowIfUnconsciousnessEnabled;
	
	protected SCR_AttributesManagerEditorComponent m_AttributeManager;
	
	protected bool m_bEntityUnconsciousnessPermitted = false;
	protected float m_fUnconsciousLevel;
	protected float m_fStartingValue;
	
	//------------------------------------------------------------------------------------------------
	override void InitDynamicDescription(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{		
		super.InitDynamicDescription(attribute);
		
		SCR_SliderBloodAttributeComponent bloodSlider = SCR_SliderBloodAttributeComponent.Cast(attributeUi);
		
		if (!bloodSlider)
		{
			Print("'SCR_CharacterBloodAttributeDynamicDescription' is not attached to an attribute with the 'SCR_SliderBloodAttributeComponent' ui!", LogLevel.ERROR);
			return;
		}
		
		if (!m_AttributeManager)
			m_AttributeManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return;
		
		m_fStartingValue = var.GetFloat();
		m_fUnconsciousLevel = bloodSlider.GetUnconsciousLevel();
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsUnconsciousnessEnabled()
	{
		//~ If attribute found check if enabled from the attribute
		SCR_BaseEditorAttributeVar unconciousVar;
		if (m_AttributeManager.GetAttributeVariable(SCR_CharUnconsciousnessEditorAttribute, unconciousVar))
			return unconciousVar.GetBool();
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
		{
			SCR_GameModeHealthSettings healthSettings = SCR_GameModeHealthSettings.Cast(gameMode.FindComponent(SCR_GameModeHealthSettings));
			if (healthSettings)
			{
				return healthSettings.IsUnconsciousnessPermitted();
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{		
		if (!super.IsValid(attribute, attributeUi) || !m_AttributeManager || m_bOnlyShowIfUnconsciousnessEnabled != IsUnconsciousnessEnabled())
			return false;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return false;
		
		float value = var.GetFloat();
		
		if (m_bLessOrEqualThan)
		{
			if (m_fStartingValue <= m_fUnconsciousLevel || value > m_fUnconsciousLevel)
				return false;
			
			return true;
		}
		else 
		{
			if (m_fStartingValue > m_fUnconsciousLevel || value <= m_fUnconsciousLevel)
				return false;
			
			return true;
		}
	}
};
