[CinematicTrackAttribute(name:"Skip Track", description:"Track used for skipping cinematic by pressing button")]
class SkipCinematicTrack : CinematicTrackBase
{

	[Attribute("false")]
	bool m_bCanSkip;
	
	[Attribute("true")]
	bool m_bCutsceneContext;

	protected FrameWidget m_wSkipHint;
	protected bool m_bHintCreated;
	protected SCR_FadeUIComponent m_SkipHintComponent;
	
	protected ResourceName m_sSkipHintLayout = "{3761599052EEA9B2}UI/layouts/HUD/SkipCinematicScene/SkipCinematiSceneWidget.layout";
	
	private ChimeraWorld globalWorld;
	private CinematicEntity cineToSkip;	

	void Skip()
	{
		if (!m_bCanSkip)
			return;
		
		cineToSkip = CinematicEntity.Cast(globalWorld.FindEntityByName(GetSceneName()));
		if (cineToSkip)
		{
			cineToSkip.Stop();
			FadeOut(m_wSkipHint);
		}
		
		AudioSystem.SetMasterVolume(AudioSystem.SFX, 1);
		AudioSystem.SetMasterVolume(AudioSystem.Dialog, 1);	
	}
	
	override void OnFinish()
	{
		FadeOut(m_wSkipHint);
	}
	
	override void OnInit(World world)
	{
		globalWorld = world;
		
		GetGame().GetInputManager().AddActionListener("SkipScene", EActionTrigger.DOWN, Skip);
	}
	
	override void OnApply(float time)
	{		
		if (m_bCutsceneContext)
			GetGame().GetInputManager().ActivateContext("CutsceneContext");
		if (m_bCanSkip && !m_bHintCreated)
		{
			CreateSkipHint(); 
			FadeIn(m_wSkipHint);
		}
		
		if (!m_bCanSkip)
			FadeOut(m_wSkipHint);
	}
	
	void CreateSkipHint()
	{
		GetGame().GetWorkspace().CreateWidgets(m_sSkipHintLayout);
		m_wSkipHint = FrameWidget.Cast(GetGame().GetWorkspace().FindAnyWidget("SkipCinematicScene"));
		m_bHintCreated = true;
	}
	
	void FadeIn(Widget w)
	{	
		m_SkipHintComponent = SCR_FadeUIComponent.Cast(w.FindHandler(SCR_FadeUIComponent));
		m_SkipHintComponent.FadeIn(true);
	}
	
	void FadeOut(Widget w)
	{
		if (!m_wSkipHint)
			return;
		
		m_SkipHintComponent = SCR_FadeUIComponent.Cast(w.FindHandler(SCR_FadeUIComponent));
		if (!m_SkipHintComponent)
			return;
		
		m_SkipHintComponent.FadeOut(true);
	}
}