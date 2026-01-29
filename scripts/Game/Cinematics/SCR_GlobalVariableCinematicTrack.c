[CinematicTrackAttribute(name:"Global Variable Track", description:"Track used for setting sound global variable")]
class SCR_GlobalVariableTrack : CinematicTrackBase
{
	[Attribute("")]
	string m_sGlobalVariableName;
	
	[Attribute("0.0")]
	float m_fValue;
	
	protected float m_fValueLast;
	
	override void OnInit(World world)
	{
		super.OnInit(world);
	}
			
	override void OnApply(float time)
	{
		if (m_sGlobalVariableName.IsEmpty())
		{
			return;
		}
		
		if (float.AlmostEqual(m_fValue, m_fValueLast, 0.001))
		{
			return;
		}
		
		AudioSystem.SetVariableByName(m_sGlobalVariableName, m_fValue, "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf");
		
		m_fValueLast = m_fValue;
	}
}
