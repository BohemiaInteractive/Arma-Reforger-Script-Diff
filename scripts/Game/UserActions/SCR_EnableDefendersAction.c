class SCR_EnableDefendersAction : ScriptedUserAction
{

	protected SCR_DefenderSpawnerComponent m_DefenderSpawner;
	protected SCR_SpawnerAIGroupManagerComponent m_GroupSpawningManager;
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_DefenderSpawner)
			return;

		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_SpawnerRequestComponent playerReqComponent = SCR_SpawnerRequestComponent.Cast(playerController.FindComponent(SCR_SpawnerRequestComponent));
		if (!playerReqComponent)
			return;

		playerReqComponent.EnableSpawning(m_DefenderSpawner, !m_DefenderSpawner.IsSpawningEnabled(), playerController.GetPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (m_GroupSpawningManager.IsAtAILimit())
		{
			SetCannotPerformReason("#AR-Campaign_Action_BuildBlocked-UC");
			return false;
		}
		
		if (!m_DefenderSpawner.UserRankCheck(user))
		{
			string rankName;
			FactionAffiliationComponent factionAffiliationComp = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
			if (!factionAffiliationComp)
				return false;
			
			SCR_Faction faction = SCR_Faction.Cast(factionAffiliationComp.GetAffiliatedFaction());
			if (!faction)
				return false;
			
			rankName = faction.GetRankName(m_DefenderSpawner.GetMinimumRank());
			
			SetCannotPerformReason(rankName);
			return false;
		}			
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_DefenderSpawner)
			return false;		
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return false;

		SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(playerController.GetControlledEntity());
		if (!chimeraCharacter || chimeraCharacter.GetFaction() != m_DefenderSpawner.GetFaction())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (m_DefenderSpawner.IsSpawningEnabled())
			outName = "#AR-DefenderSpawner_DisableSpawning";
		else
			outName = "#AR-DefenderSpawner_EnableSpawning";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_DefenderSpawner = SCR_DefenderSpawnerComponent.Cast(pOwnerEntity.FindComponent(SCR_DefenderSpawnerComponent));
	
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;	
			
		m_GroupSpawningManager = SCR_SpawnerAIGroupManagerComponent.Cast(gameMode.FindComponent(SCR_SpawnerAIGroupManagerComponent));
		if (!m_GroupSpawningManager)
		{
			Print("SCR_DefenderSpawnerComponent requires SCR_SpawnerAIGroupManagerComponent attached to gamemode to work properly!", LogLevel.ERROR);	
			return;
		}	
	}
};
