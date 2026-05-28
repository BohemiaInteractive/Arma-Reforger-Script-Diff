//! Helper class to be able to display this in attributes
[BaseContainerProps(configRoot: true)]
class SCR_BushContainer
{
	[Attribute(defvalue: ETreeSoundTypes.Bush.ToString(), desc: "Type of foliage, used for calculating the bush's slowdown", uiwidget: UIWidgets.ComboBox, enumType: ETreeSoundTypes)]
	protected ETreeSoundTypes m_eTreeType;

	[Attribute(defvalue: "0", desc: "Weight given to the height of the specific foliage type when calculating the slowdown", params: "0 inf 0.01")]
	protected float m_fHeightModifier;
	
	//------------------------------------------------------------------------------------------------
	ETreeSoundTypes GetTreeType()
	{
		 return m_eTreeType; 
	}
	    
	//------------------------------------------------------------------------------------------------
	float GetSlowdownHeightModifier()
	{ 
		return m_fHeightModifier; 
	}
}