[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_SoundType
{						
	[Attribute("1.0", UIWidgets.Slider, "", "0.1 10.0 0.01")]
	float m_fDensityMultiplier;
	
	[Attribute("", UIWidgets.Object, "")]
	ref array<ref SCR_SoundDef> m_aSoundDef;
}
