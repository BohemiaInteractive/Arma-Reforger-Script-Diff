class SCR_DefendWaypointPresetClass : SCR_DefendWaypointClass
{
};

[BaseContainerProps()]
class SCR_DefendWaypointPreset
{	
	[Attribute("", UIWidgets.EditBox, "Preset name, only informative. Switch using index.")];
	protected string m_sName;
	
	[Attribute("true", UIWidgets.CheckBox, "Use turrets?")];
	protected bool m_bUseTurrets;
	
	[Attribute("1", UIWidgets.Slider, "Fraction of SA used for this waypoint 0 - no, 1 - all available. The rest uses sector defense", "0 1 0.1")];
	protected float m_fFractionOfSA;
	
	[Attribute("", UIWidgets.Auto, "List tags to search in the preset")];
	protected ref array<string> m_aTagsForSearch;
	
	//----------------------------------------------------------------------------------------
	string GetPresetName()
	{
		return m_sName;
	}
	
	//----------------------------------------------------------------------------------------
	void SetPresetName(string newPresetName)
	{
		m_sName = newPresetName;
	}
	
	//----------------------------------------------------------------------------------------
	bool GetUseTurrets()
	{
		return m_bUseTurrets;
	}
	
	//----------------------------------------------------------------------------------------
	void SetUseTurrets(bool useTurrets)
	{
		m_bUseTurrets = useTurrets;
	}

	//----------------------------------------------------------------------------------------
	float GetFractionOfSA()
	{
		return m_fFractionOfSA;
	}
	
	//----------------------------------------------------------------------------------------
	void SetFractionOfSA(float fractionOfSA)
	{
		m_fFractionOfSA = fractionOfSA;
	}

	//----------------------------------------------------------------------------------------
	void GetTagsForSearch(notnull array<string> outTags)
	{
		outTags.Clear();
		outTags.Copy(m_aTagsForSearch);
	}
	
	//----------------------------------------------------------------------------------------
	void SetTagsForSearch(array<string> inTags)
	{
		m_aTagsForSearch.Clear();
		m_aTagsForSearch.Copy(inTags);
	}
};