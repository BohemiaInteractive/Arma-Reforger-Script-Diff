class SCR_EnterEnemyMilitaryBaseMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
	//~ Delay to make sure the OnSpawn music is played correctly after the respawn menu music
	protected void OnBaseEntered(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		Faction playerFaction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			playerFaction = factionManager.GetLocalPlayerFaction();
		
		if (base.GetFaction() == playerFaction)
			return;
		
		m_MusicManager.Play(SCR_SoundEvent.SOUND_ONENTERINGENEMYBASE);	
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		
		if (!m_MusicManager)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		
		if (!baseManager)
			return;
		
		baseManager.GetOnLocalPlayerEnteredBase().Insert(OnBaseEntered);
	}
	
	override void OnDelete()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		
		if (!baseManager)
			return;
		
		baseManager.GetOnLocalPlayerEnteredBase().Remove(OnBaseEntered);
	}
}