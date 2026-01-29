[CinematicTrackAttribute(name:"Postprocess cinematic track", description:"Allows you to control camera postprocesses")]
class PostprocessCinematicTrack : CinematicTrackBase
{
	
	[Attribute(defvalue: "1", desc: "Saturation intensity")]
	protected float m_fSaturation;
	
	protected static float m_fFinalSaturation;
	protected static bool s_bEnableSaturation;
	protected const string DESATURATION_EMAT = "{E6B6158B2C156AB1}UI/Materials/ScreenEffects_ColorPP_Cinematics.emat";
	private World actualWorld;
	
	//------------------------------------------------------------------------------------------------
	override void OnApply(float time)
	{
		actualWorld.SetCameraPostProcessEffect(actualWorld.GetCurrentCameraId(), 0, PostProcessEffectType.Colors, DESATURATION_EMAT);
		m_fFinalSaturation = m_fSaturation;
		s_bEnableSaturation = true;
	}
	
	override void OnInit(World world)
	{		
		actualWorld = world;
	}
	
}