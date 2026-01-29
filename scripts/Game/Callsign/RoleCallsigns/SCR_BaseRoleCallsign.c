[BaseContainerProps(), BaseContainerCustomStringTitleField("Do not use!")]
class SCR_BaseRoleCallsign
{
	[Attribute("1")]
	protected bool m_bEnabled;
	
	[Attribute()]
	protected LocalizedString m_sRoleName;
	
	[Attribute(SCR_ERoleAvailableForCharacterType.AI_ONLY.ToString(), uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_ERoleAvailableForCharacterType), desc: "If the role is availible for Player and AI or only AI or Player. Take into account that Player roles are set and will only change if the player leaves the group not when they die.")]
	protected SCR_ERoleAvailableForCharacterType m_eRoleAvailableForCharacterType;
	
	[Attribute("1", "Unique roles can only be present once in a group (Including any attached Master and Slave groups)")]
	protected bool m_bIsUnique;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Checks if AI can be assigned a role (or a specific role if roleCallsignIndex is given a value)
	\param character IEntity of character to assign callsign (Can be player or AI)
	\param playerID If character is a player then player ID can be used to check if valid
	\param group SCR_AIGroup group of which character is a part of
	\param occupiedRoles roles already occupied by AI for the group the character is part of
	\param occupiedRoles roles already occupied by Player for the group the character is part of
	\param[out] roleCallsignIndex the return index of the found character role. Can be given a value to look for a specific role to assign
	\param[out] isUnique If true than only one character can have the role in the group (as well as any attached master and slave groups)
	\return bool true if callsign found, else it returns false
	*/
	bool IsValidRole(IEntity character, int playerID, SCR_AIGroup group, inout int roleCallsignIndex, out bool isUnique)
	{	
		if (!m_bEnabled)
			return false;
		
		isUnique = m_bIsUnique;
		
		//~ If looking for specific role and this is not that role
		if (roleCallsignIndex >= 0 && roleCallsignIndex != GetRoleIndex())
			return false;
		
		//~ Check if role can be assigned to player or AI
		if (m_eRoleAvailableForCharacterType != SCR_ERoleAvailableForCharacterType.ANY_CHARACTER)
		{
			//~ Player only role and given character is not AI
			if (m_eRoleAvailableForCharacterType == SCR_ERoleAvailableForCharacterType.PLAYER_ONLY && playerID <= 0)
				return false;
			//~ AI Only role and given character is not AI
			else if (m_eRoleAvailableForCharacterType == SCR_ERoleAvailableForCharacterType.AI_ONLY && playerID > 0)
				return false;
		}
		
		//~ Checks if role is availible in group. If not in group than it is always availible
		if (!group)
			return true;

		//~ Get master group if group is slave
		SCR_AIGroup masterGroup;
		if (group.IsSlave())
			masterGroup = group.GetMaster();
		
		if (masterGroup)
			group = masterGroup;

		//~ If unique role check if it is already in use
		if (m_bIsUnique)
		{
			SCR_CallsignGroupComponent groupCallsignComponent = SCR_CallsignGroupComponent.Cast(group.FindComponent(SCR_CallsignGroupComponent));
			
			//~ Check if role is already in use by AI or player
			if (groupCallsignComponent && groupCallsignComponent.IsUniqueRoleInUse(GetRoleIndex()))			
				return false;
		}
		
		//~ All base checks pass
		return true; 
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get Role name
	\return string role localized name
	*/
	string GetRoleName()
	{
		return m_sRoleName;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get Role index
	\return int role index
	*/
	int GetRoleIndex()
	{
		return -1;
	}
};

//------------------------------------------------------------------------------------------------
enum SCR_ERoleAvailableForCharacterType
{
	ANY_CHARACTER,
	AI_ONLY,
	PLAYER_ONLY,
};


