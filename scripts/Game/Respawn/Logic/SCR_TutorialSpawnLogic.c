[BaseContainerProps(category: "Respawn")]
class SCR_TutorialSpawnLogic : SCR_SpawnLogic
{
	protected ref Identity m_PlayerIdentity;
	
	override void OnPlayerSpawned_S(int playerId, IEntity entity)
	{
		SCR_CharacterIdentityComponent identityComp = SCR_CharacterIdentityComponent.Cast(entity.FindComponent(SCR_CharacterIdentityComponent));
		if (identityComp)
		{
			if (!m_PlayerIdentity)
			{
				Identity initialIdentity = identityComp.GetIdentity();
				if (!initialIdentity)
					return;
				
				m_PlayerIdentity = new Identity;
				m_PlayerIdentity.SetName(initialIdentity.GetName());
				m_PlayerIdentity.SetAlias(initialIdentity.GetAlias());
				m_PlayerIdentity.SetSurname(initialIdentity.GetSurname());
				m_PlayerIdentity.SetSoundIdentity(initialIdentity.GetSoundIdentity());
				m_PlayerIdentity.SetVisualIdentity(initialIdentity.GetVisualIdentity());
				
			}
			else
			{
				identityComp.SetIdentity(m_PlayerIdentity);
			}
		}
	}	
}
