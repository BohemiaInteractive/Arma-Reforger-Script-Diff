class SCR_BaseCaptureMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
	//~ Delay to make sure the OnSpawn music is played correctly after the respawn menu music
	protected void OnBaseCapture(SCR_MilitaryBaseComponent base, Faction faction)
	{
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
		if (!campaignBase || campaignBase.GetBuiltByPlayers())
			return;

		PlayerController pc = GetGame().GetPlayerController();
		
		if (!pc)
			return;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		
		if (!player || player.GetFaction() != faction)
			return;
		
		m_MusicManager.Play(SCR_SoundEvent.SOUND_ONBASECAPTURE);	
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;
		
		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
		
		if (!baseManager)
			return;
		
		baseManager.GetOnBaseFactionChanged().Insert(OnBaseCapture);
	}
	
	override void OnDelete()
	{
		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
		
		if (!baseManager)
			return;
		
		baseManager.GetOnBaseFactionChanged().Remove(OnBaseCapture);
	}
}
