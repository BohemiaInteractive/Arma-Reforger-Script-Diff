[CinematicTrackAttribute(name:"Fades Track", description:"Track used for fading of visual and sound")]
class FadesCinematicTrack : CinematicTrackBase
{
	
	[Attribute("0.0", params:"0.0 1.0")]
	float m_fBlackScreenOpacity;
	
	[Attribute("1.0", params:"0.0 1.0")]
	float m_fVolumeSFX;
	
	[Attribute("1.0", params:"0.0 1.0")]
	float m_fVolumeVoices;
	
	[Attribute("false")]
	bool m_bDestroyBlackscreenOnTimelineEnd;
	
	[Attribute("0", params:"0 100")]
	int m_iDestroyLaterInSeconds;
	
	private GenericEntity m_Entity;
	private World globalWorld;
	static Widget m_wBlackScreen;
	protected const string FADEOUT_LAYOUT = "{265245C299401BF6}UI/layouts/Menus/ContentBrowser/DownloadManager/ScrollBackground.layout";
	
	void DestroyScreen()
	{
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			
		if (gamemode)
			gamemode.GetOnGameEnd().Remove(DestroyScreen);

		if (m_wBlackScreen)
			m_wBlackScreen.RemoveFromHierarchy();
	}
	
	override void OnFinish()
	{
		if (!m_bDestroyBlackscreenOnTimelineEnd)
			return;
		
		if (m_iDestroyLaterInSeconds == 0)
			DestroyScreen();
		else
			GetGame().GetCallqueue().CallLater(DestroyScreen, m_iDestroyLaterInSeconds * 1000);
	}
	
	override void OnInit(World world)
	{		
		if (!m_wBlackScreen)
		{
			m_wBlackScreen = GetGame().GetWorkspace().CreateWidgets(FADEOUT_LAYOUT);
			
			SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			
			if (gamemode)
				gamemode.GetOnGameEnd().Insert(DestroyScreen);
		}
	}
	
	override void OnApply(float time)
	{
			
		if (!m_wBlackScreen)
			return;
		
		m_wBlackScreen.SetOpacity(m_fBlackScreenOpacity);
		
		AudioSystem.SetMasterVolume(AudioSystem.SFX, m_fVolumeSFX);
		AudioSystem.SetMasterVolume(AudioSystem.Dialog, m_fVolumeVoices);	
	}
}
