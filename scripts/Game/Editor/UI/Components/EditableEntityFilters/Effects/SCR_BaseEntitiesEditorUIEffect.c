[BaseContainerProps(), SCR_BaseEntitiesEditorUIEffectTitle()]
class SCR_BaseEntitiesEditorUIEffect
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "State on which this effect is applied to.", enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	private EEditableEntityState m_State;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EEditableEntityState GetState()
	{
		return m_State;
	}	

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] w
	void ApplyOn(Widget w)
	{
		Print("ApplyOn() called on base class SCR_BaseEntitiesEditorUIEffect. Please use one of specialized variants.", LogLevel.WARNING);
	}
}

class SCR_BaseEntitiesEditorUIEffectTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		int enumValue;
		if (!source.Get("m_State", enumValue))
			return false;
		
		title = source.GetClassName();
		title.Replace("SCR_", "");
		title.Replace("EntitiesEditorUIEffect", "");
		
		title += ": " + typename.EnumToString(EEditableEntityState, enumValue);
		return true;
	}
}
