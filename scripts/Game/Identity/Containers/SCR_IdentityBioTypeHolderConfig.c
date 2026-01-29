[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleEnum(SCR_EIdentityType, "m_eIdentityType")]
class SCR_IdentityBioTypeHolderConfig
{
	[Attribute("0", UIWidgets.ComboBox, "Identity Type", "", ParamEnumArray.FromEnum(SCR_EIdentityType) )]
	SCR_EIdentityType m_eIdentityType;
	
	[Attribute()]
	ref array<ref SCR_IdentityBioGroupConfig> m_aIdentityBioGroups;
	
	//------------------------------------------------------------------------------------------------
	void SCR_IdentityBioTypeHolderConfig()
	{
		//~ Highly unlikely that this happens but will send an error if it happens
		if (m_aIdentityBioGroups.Count() > SCR_IdentityManagerComponent.MAX_IDENTITY_GROUPS)
		{
			Print(string.Format("'SCR_IdentityBioTypeHolderConfig': Bio groups count is greater than %1 which will break replication", SCR_IdentityManagerComponent.MAX_IDENTITY_GROUPS), LogLevel.ERROR);
		}
	}
}