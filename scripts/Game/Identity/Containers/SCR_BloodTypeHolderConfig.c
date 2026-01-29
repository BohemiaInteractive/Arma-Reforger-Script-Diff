[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleEnum(SCR_EIdentityType, "m_eIdentityType")]
class SCR_BloodTypeHolderConfig
{
	[Attribute(desc: "Array of all possible bloodtypes with randomization weight. Keep combined randomization weight at 100 for ease of use", category: "Data")]
	protected ref array<ref SCR_CharacterIdentityBloodType> m_aCharacterIdentityBloodtypes;
	
	//------------------------------------------------------------------------------------------------
	//! Get array of valid blood types
	//! \param[in,out] bloodTypes Out array of blood types
	//! \return Lenght of array
	int GetBloodTypes(notnull inout array<SCR_CharacterIdentityBloodType> bloodTypes)
	{
		bloodTypes.Clear();
		
		foreach (SCR_CharacterIdentityBloodType bloodType : m_aCharacterIdentityBloodtypes)
		{
			bloodTypes.Insert(bloodType);
		}
		
		return bloodTypes.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Blood type UI Info
	SCR_UIInfo GetBloodTypeUIInfo(SCR_EBloodType bloodType)
	{
		if (bloodType == SCR_EBloodType.AUTO)
			return null;
		
		foreach (SCR_CharacterIdentityBloodType bloodtypeInfo: m_aCharacterIdentityBloodtypes)
		{
			if (bloodtypeInfo.GetBloodType() == bloodType)
				return bloodtypeInfo.GetUiInfo();
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BloodTypeHolderConfig()
	{		
		//~ Validate array entries
		int count = m_aCharacterIdentityBloodtypes.Count();
		for (int i = count - 1; i >= 0; i--)
		{
			//~ Remove invalid entries
			if (!m_aCharacterIdentityBloodtypes[i] || !m_aCharacterIdentityBloodtypes[i].GetUiInfo() || m_aCharacterIdentityBloodtypes[i].GetBloodType() == SCR_EBloodType.AUTO || m_aCharacterIdentityBloodtypes[i].GetWeight() <= 0)
			{
				Print("'SCR_BloodTypeHolderConfig' m_aCharacterIdentityBloodtypes index: '" + i + "' has an invalid entry!", LogLevel.WARNING);
				m_aCharacterIdentityBloodtypes.RemoveOrdered(i);
				
				continue;
			}
		}
	}
}