[BaseContainerProps()]
class SCR_SequenceDef
{
	[Attribute("4", UIWidgets.Slider, "Repetition count 1", "0 10 1")]
	int m_iRepCount;
	
	[Attribute("0", UIWidgets.Slider, "Repetition count randomisation 1", "0 10 1")]
	int m_iRepCountRnd;
	
	[Attribute("10000", UIWidgets.Slider, "[ms]", "0 30000 1")]
	int m_iRepTime;
	
	[Attribute("500", UIWidgets.Slider, "[ms]", "0 30000 1")]
	int m_iRepTimeRnd;
}
