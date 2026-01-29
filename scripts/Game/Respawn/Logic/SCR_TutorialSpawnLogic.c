[BaseContainerProps(category: "Respawn")]
class SCR_TutorialSpawnLogic : SCR_SpawnLogic
{
	protected Identity m_PlayerIdentity;
	
	override void OnPlayerSpawned_S(int playerId, IEntity entity)
	{
		SCR_CharacterIdentityComponent identityComp = SCR_CharacterIdentityComponent.Cast(entity.FindComponent(SCR_CharacterIdentityComponent));
		if (identityComp)
		{
			if (!m_PlayerIdentity)
				m_PlayerIdentity = identityComp.GetIdentity();
			else
				identityComp.SetIdentity(m_PlayerIdentity);
		}
	}	
}
