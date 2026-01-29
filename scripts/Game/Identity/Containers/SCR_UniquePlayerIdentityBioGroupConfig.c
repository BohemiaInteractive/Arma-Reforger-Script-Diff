//~ Unique Identity Group which only allows the Identity to be assigned to players that have the unique Identity ID
//~ This is for modders to use if they want a group of people to always have a specific identity (Such as admins) or specific players to have a specific identity always.
//~ Todo: Currently you have to create a whole new identity group in order to add a single unique player which is not ideal
[BaseContainerProps(configRoot: true), BaseContainerCustomDoubleTitleField("m_sBioGroupID", "m_iWeight")]
class SCR_UniquePlayerIdentityBioGroupConfig : SCR_CharacterIdentityBioGroupConfig
{
	[Attribute(desc: "Unique Player UID which will be used for randomization. This is not the player ID but the the player UID obtained via BackendApi", category: "Requirements")]
	protected ref array<UUID> m_aValidUniquePlayerUIDs;
	
	override bool IsValidForRandomization(IEntity entity, SCR_ExtendedIdentityComponent extendedIdentity)
	{
		SCR_ExtendedCharacterIdentityComponent extendedCharIdentity = SCR_ExtendedCharacterIdentityComponent.Cast(extendedIdentity);
		if (!extendedCharIdentity)
			return false;
		
		if (extendedCharIdentity.GetPlayerID() <= 0)
			return false;
		
		//~ Check if the character is on the unique character list
		if (!m_aValidUniquePlayerUIDs.Contains(SCR_PlayerIdentityUtils.GetPlayerIdentityId(extendedCharIdentity.GetPlayerID())))
			return false;

		return super.IsValidForRandomization(entity, extendedCharIdentity);
		
	}
}
