[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_LabelType")]
class SCR_EditableEntityCampaignBuildingLabelSetting : SCR_EditableEntityCoreLabelSettingExtended
{
	[Attribute("-1", desc: "Conflict service point equivalent, leave empty if there are no equivalents", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EServicePointType))]
	protected SCR_EServicePointType m_eLinkedConflictServicePoint;
	
	//------------------------------------------------------------------------------------------------
	SCR_EServicePointType GetLinkedConflictServicePoint()
	{
		return m_eLinkedConflictServicePoint;
	}
}

class SCR_EditableEntityCampaignBuildingModeLabelData
{
	protected EEditableEntityLabel m_eLabel;
	protected SCR_UIInfo m_UIInfo;
	protected SCR_EServicePointType m_eLinkedServicePoint;
	
	//------------------------------------------------------------------------------------------------
	EEditableEntityLabel GetEntityLabel()
	{
		return m_eLabel;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_UIInfo GetUIInfo()
	{
		return m_UIInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EServicePointType GetServicePointType()
	{
		return m_eLinkedServicePoint;
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_EditableEntityCampaignBuildingModeLabelData(EEditableEntityLabel label, SCR_UIInfo uiInfo, SCR_EServicePointType linkedServicePoint)
	{
		m_eLabel = label;
		m_UIInfo = uiInfo;
		m_eLinkedServicePoint = linkedServicePoint;
	}
}
