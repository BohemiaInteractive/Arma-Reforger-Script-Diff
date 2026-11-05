[BaseContainerProps(), BaseContainerCustomEnumWithValue(SCR_EFieldManualRepackingRule, "m_eVisibleForRule", "m_sText", "EMPTY FIELD", "%1 - %2")]
class SCR_FieldManualPiece_MagazineRepackingText : SCR_FieldManualPiece_Text
{
	[Attribute(defvalue: SCR_EFieldManualRepackingRule.OTHER_THAN_CANT.ToString(), desc: "Text will be visible only when this is currently used rule", uiwidget: UIWidgets.ComboBox, enumType: SCR_EFieldManualRepackingRule)]
	protected SCR_EFieldManualRepackingRule m_eVisibleForRule;

	//------------------------------------------------------------------------------------------------
	override void CreateWidget(notnull Widget parent)
	{
		SCR_EMagazineRepackingRules currentRule = SCR_MagazineRepackingSystem.GetGameModeRepackingRule();
		if (currentRule == SCR_EMagazineRepackingRulesOverride.NO_OVERRIDE)
			currentRule = SCR_EMagazineRepackingRules.CAN_WHEN_OTHER_CONTAINS_THIS_AMMO_TYPES;

		switch (m_eVisibleForRule)
		{
			case SCR_EFieldManualRepackingRule.OTHER_THAN_CANT:
				if (currentRule == SCR_EMagazineRepackingRules.CANNOT)
					return;

				break;

			default:
				if (currentRule != m_eVisibleForRule)
					return;
		}

		super.CreateWidget(parent);
	}
}
