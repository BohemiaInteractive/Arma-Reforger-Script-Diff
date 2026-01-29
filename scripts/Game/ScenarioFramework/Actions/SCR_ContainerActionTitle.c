class SCR_ContainerActionTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = source.GetClassName();
		title.Replace("SCR_ScenarioFrameworkAction", "");
		title.Replace("SCR_ScenarioFramework", "");
		string sOriginal = title;
		SplitStringByUpperCase(sOriginal, title);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] input
	//! \param[out] output
	void SplitStringByUpperCase(string input, out string output)
	{
		output = "";
		bool wasPreviousUpperCase;
		int asciiChar;
		for (int i, count = input.Length(); i < count; i++)
		{
			asciiChar = input.ToAscii(i);
			bool isLowerCase = (asciiChar > 96 && asciiChar < 123);
			if (i > 0 && !wasPreviousUpperCase && !isLowerCase)
			{
				output += " " + asciiChar.AsciiToString();
				wasPreviousUpperCase = true;
			}
			else
			{
				if (isLowerCase)
					wasPreviousUpperCase = false;

				output += asciiChar.AsciiToString();
			}
		}
	}
}