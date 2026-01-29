//! Holds the name data to show which parts of the vehicle or character the player is healing
[BaseContainerProps(configRoot: true)]
class SCR_HitZoneGroupNameHolder
{
	[Attribute(desc: "List of all hitzone group notification data. That is used to display in notification what is being healed/repaired")]
	protected ref array<ref SCR_HitZoneGroupName> m_aHitZoneGroupNames;
	
	//------------------------------------------------------------------------------------------------
	/*!
	\param hitZoneGroup Hitzone enum to find name of
	\return Name of hitzoneGroup
	*/
	LocalizedString GetHitZoneGroupName(EHitZoneGroup hitZoneGroup)
	{
		foreach (SCR_HitZoneGroupName nameData : m_aHitZoneGroupNames)
		{
			if (nameData.GetHitZoneGroup() == hitZoneGroup)
				return nameData.m_sHitZoneGroupName;
		}
		
		Print("Could not find hitZoneGroup name for: '" + typename.EnumToString(EHitZoneGroup, hitZoneGroup) + "'", LogLevel.ERROR);
		return string.Empty;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), BaseContainerCustomStringTitleField("USE INHERITED VERSION ONLY!")]
class SCR_HitZoneGroupName
{
	[Attribute(desc: "Text shown in notification if the provided hitzone group is send", uiwidget: UIWidgets.LocaleEditBox)]
	LocalizedString m_sHitZoneGroupName;
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Hitzone group
	*/
	EHitZoneGroup GetHitZoneGroup()
	{
		return EHitZoneGroup.VIRTUAL;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVehicleHitZoneGroup, "m_eVehicleHitZoneGroup")]
class SCR_VehicleHitZoneGroupName : SCR_HitZoneGroupName
{
	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "If this hitzone is given to the notification it will display this data. Only the first entry in the action is checked and it can include multiple", enums: ParamEnumArray.FromEnum(EVehicleHitZoneGroup))]
	protected EVehicleHitZoneGroup m_eVehicleHitZoneGroup;
	
	//------------------------------------------------------------------------------------------------
	override EHitZoneGroup GetHitZoneGroup()
	{
		return m_eVehicleHitZoneGroup;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ECharacterHitZoneGroup, "m_eCharacterHitZoneGroup")]
class SCR_CharacterHitZoneGroupName : SCR_HitZoneGroupName
{
	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "If this hitzone is given to the notification it will display this data.", enums: ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	protected ECharacterHitZoneGroup m_eCharacterHitZoneGroup;
	
	//------------------------------------------------------------------------------------------------
	override EHitZoneGroup GetHitZoneGroup()
	{
		return m_eCharacterHitZoneGroup;
	}
}
