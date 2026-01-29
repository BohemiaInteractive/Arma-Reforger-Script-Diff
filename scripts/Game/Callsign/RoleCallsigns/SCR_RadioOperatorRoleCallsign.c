[BaseContainerProps(), BaseContainerCustomStringTitleField("Radio Operator")]
class SCR_RadioOperatorRoleCallsign: SCR_BaseRoleCallsign
{	
	//------------------------------------------------------------------------------------------------
	override bool IsValidRole(IEntity character, int playerID, SCR_AIGroup group, inout int roleCallsignIndex, out bool isUnique)
	{
		if (!character)
			return false;
		
		if (!super.IsValidRole(character, playerID, group, roleCallsignIndex, isUnique))
			return false;
			
		if (HasRadio(character))
		{
			roleCallsignIndex = GetRoleIndex();
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool HasRadio(notnull IEntity character)
	{
		EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(character.FindComponent(EquipedLoadoutStorageComponent));
		if (!loadoutStorage)
			return false;
		
		IEntity backpack = loadoutStorage.GetClothFromArea(LoadoutBackpackArea);
		return (backpack && backpack.FindComponent(SCR_RadioComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetRoleIndex()
	{
		return ERoleCallsign.RADIO_OPERATOR;
	}
};