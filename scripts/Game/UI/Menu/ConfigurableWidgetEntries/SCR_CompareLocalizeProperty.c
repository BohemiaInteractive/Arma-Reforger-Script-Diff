//! Sort by localized property labels
//-------------------------------------------------------------------------------------------
class SCR_CompareLocalizeProperty : SCR_SortCompare<SCR_LocalizedProperty>
{
	//-------------------------------------------------------------------------------------------
	override static int Compare(SCR_LocalizedProperty left, SCR_LocalizedProperty right)
	{
		if (!left || !right)
			return -1;
		
		string name1 = ResolveName(left.m_sLabel);
		string name2 = ResolveName(right.m_sLabel);
		
		if (name1.Compare(name2) == -1)
			return 1;
		else
			return 0;
	}
	
	//-------------------------------------------------------------------------------------------
	static string ResolveName(string name)
	{
		if (name.StartsWith("#"))
			return WidgetManager.Translate(name);
		else
			return name;
	}
};