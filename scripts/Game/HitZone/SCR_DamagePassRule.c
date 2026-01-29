//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_DamagePassRuleContainerTitle()]
class SCR_DamagePassRule
{
	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EDamageState), desc: "Allow this rule if current damage state is one of defined.\nLeave empty to allow all damage states")]
	ref array<EDamageState> m_aDamageStates;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EDamageType), desc: "Allow this rule if source damage type is one of defined.\nLeave empty to allow all damage types")]
	ref array<EDamageType> m_aSourceDamageTypes;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EDamageType), desc: "Force damage type to be passed.\nSet to TRUE to pass unmodified")]
	EDamageType m_eOutputDamageType;

	[Attribute(desc: "Scaling of damage\n[x]")]
	float m_fMultiplier;

	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Pass to default hit zone of root damage manager")]
	bool m_bPassToRoot;

	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Pass to default hit zone of parent damage manager")]
	bool m_bPassToParent;	
	
	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Pass to default hit zone of same damage manager")]
	bool m_bPassToDefaultHitZone;
}

//------------------------------------------------------------------------------------------------
class SCR_DamagePassRuleContainerTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = string.Empty;

		int type;
		if (source.Get("m_eOutputDamageType", type))
			title = "Pass as " + SCR_Enum.GetEnumName(EDamageType, type);

		float multiplier;
		if (source.Get("m_fMultiplier", multiplier))
			title += " x " + multiplier.ToString(-1, 3);

		bool root;
		if (source.Get("m_bPassToRoot", root) && root)
			title += ", root";

		bool parent;
		if (source.Get("m_bPassToParent", parent) && parent)
			title += ", parent";		
		
		bool defaultHitZone;
		if (source.Get("m_bPassToDefaultHitZone", defaultHitZone) && defaultHitZone)
			title += ", defaultHitZone";

		return true;
	}
}

