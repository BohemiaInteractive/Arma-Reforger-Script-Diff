class SCR_BIKIStringBuilder
{
	protected string m_sContent;

	//------------------------------------------------------------------------------------------------
	//! Add text line that gets ended by a line return
	//! \param[in] line
	void AddLine(string line)
	{
		m_sContent += line + "\n";
	}

	//------------------------------------------------------------------------------------------------
	//! Add a text line in format "|key= value" (or "|key=" if value is empty)
	//! \param[in] key if empty, adds nothing
	//! \param[in] value
	void AddKeyValueLine(string key, string value = string.Empty)
	{
		key.Trim();
		if (!key)
			return;

		value.Trim();
		if (value)
			m_sContent += string.Format("|%1= %2\n", key, value);
		else
			m_sContent += string.Format("|%1=\n", key, value);
	}

	//------------------------------------------------------------------------------------------------
	//! Add a wiki title (e.g == title lv 2 ==)
	//! \param[in] titleLevel in range 1..6
	//! \param[in] text can be empty (e.g ==  ==)
	void AddTitle(int titleLevel, string text)
	{
		if (titleLevel < 1)
			titleLevel = 1;

		if (titleLevel > 6)
			titleLevel = 6;

		if (m_sContent)
		{
			if (m_sContent.Length() < 2 || m_sContent.Substring(m_sContent.Length() - 2, 2) != "\n\n")
			{
				if (titleLevel < 3)
					m_sContent += "\n\n";
				else
				if (titleLevel == 3)
					m_sContent += "\n";
			}
		}

		switch (titleLevel)
		{
			case 1: m_sContent += "= " + text + " =\n\n"; break;
			case 2: m_sContent += "== " + text + " ==\n\n"; break;
			case 3: m_sContent += "=== " + text + " ===\n\n"; break;
			case 4: m_sContent += "==== " + text + " ====\n"; break;
			case 5: m_sContent += "===== " + text + " =====\n"; break;
			case 6: m_sContent += "====== " + text + " ======\n"; break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Begin a wiki table
	//! \see EndTable()
	//! \param[in] style
	// void BeginTable(string classes = "wikitable", string style = string.Empty)
	void BeginTable(string classes = "wikitable", string style = string.Empty)
	{
		m_sContent += "{|"; // }
		if (classes)
			m_sContent += " class=\"" + classes + "\"";

		if (style)
			m_sContent += " style=\"" + style + "\"";

		m_sContent += "\n";
	}

	//------------------------------------------------------------------------------------------------
	//! This method allows empty headers
	//! \param[in] titles list of titles to display on one line
	void AddTableHeadersLine(notnull array<string> titles)
	{
		foreach (string title : titles)
		{
			if (!title) // IsEmpty()
				m_sContent += "!\n";
			else
				m_sContent += "! " + title + "\n";
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add a table line of header cells
	//! This method forbids empty headers and stops on the first encountered one
	//! \see BeginTable()
	//! \see EndTable()
	void AddTableHeadersLine(string title1, string title2 = string.Empty, string title3 = string.Empty, string title4 = string.Empty, string title5 = string.Empty)
	{
		array<string> titles = { title1, title2, title3, title4, title5 };
		foreach (string title : titles)
		{
			if (!title)
				return;

			m_sContent += "! " + title + "\n";
		}
	}

	//------------------------------------------------------------------------------------------------
	//! This method allows an empty header
	//! \param[in] title
	//! \param[in] classes
	//! \param[in] style
	void AddTableHeader(string title, string classes = string.Empty, string style = string.Empty)
	{
		if (!classes && !style)
		{
			m_sContent += "! " + title + "\n";
			return;
		}

		m_sContent += "!";
		if (classes)
			m_sContent += " class=\"" + classes + "\"";

		if (style)
			m_sContent += " style=\"" + style + "\"";

		m_sContent += " | " + title + "\n";
	}

	//------------------------------------------------------------------------------------------------
	//! Add a table line of cells
	//! This method allows empty data cells
	//! \see BeginTable()
	//! \see EndTable()
	//! \param[in] data
	void AddTableDataCellsLine(notnull array<string> data)
	{
		m_sContent += "|-\n";
		foreach (string datum : data)
		{
			m_sContent += "| " + datum + "\n";
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add a table line of cells
	//! This method forbids empty data cells and stops on the first encountered one
	//! \see BeginTable()
	//! \see EndTable()
	//! \param[in] data1
	//! \param[in] data2
	//! \param[in] data3
	//! \param[in] data4
	//! \param[in] data5
	void AddTableDataCellsLine(string data1, string data2 = string.Empty, string data3 = string.Empty, string data4 = string.Empty, string data5 = string.Empty)
	{
		m_sContent += "|-\n";
		array<string> data = { data1, data2, data3, data4, data5 };
		foreach (string datum : data)
		{
			if (!datum)
				return;

			m_sContent += "| " + datum + "\n";
		}
	}

	//------------------------------------------------------------------------------------------------
	//! This method allows an empty data cell
	//! \param[in] datum
	//! \param[in] classes cell's CSS classes
	//! \param[in] style cell's CSS
	void AddTableDataCell(string datum, string classes = string.Empty, string style = string.Empty)
	{
		if (!classes && !style)
		{
			m_sContent += "| " + datum + "\n";
			return;
		}

		m_sContent += "|";
		if (classes)
			m_sContent += " class=\"" + classes + "\"";

		if (style)
			m_sContent += " style=\"" + style + "\"";

		if (datum)
			m_sContent += " | " + datum + "\n";
		else
			m_sContent += " |\n";
	}

	//------------------------------------------------------------------------------------------------
	//! Add a title header followed by provided data (horizontal, line)
	//! \see BeginTable()
	//! \see EndTable()
	//! \param[in] title
	//! \param[in] data
	void AddHeaderAndDataCellsLine(string title, notnull array<string> data)
	{
		m_sContent += "|-\n" +
			"! " + title + "\n";

		foreach (string datum : data)
		{
			m_sContent += "| " + datum + "\n";
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add the end of a previously opened table
	//! \see BeginTable()
	void EndTable()
	{
		// {
		m_sContent += "|}\n";
	}

	//------------------------------------------------------------------------------------------------
	//! Add {{GameCategory|armaR}} wiki category directly after the current content (empty lines not managed)
	//! \param[in] catLv1
	//! \param[in] catLv2
	//! \param[in] catLv3
	//! \param[in] catLv4
	//! \param[in] catLv5
	void AddCategory(string catLv1 = string.Empty, string catLv2 = string.Empty, string catLv3 = string.Empty, string catLv4 = string.Empty, string catLv5 = string.Empty)
	{
		if (!catLv1)
			m_sContent += "{{GameCategory|armaR}}\n";
		else if (!catLv2)
			m_sContent += "{{GameCategory|armaR|" + catLv1 + "}}\n";
		else if (!catLv3)
			m_sContent += "{{GameCategory|armaR|" + catLv1 + "|" + catLv2 + "}}\n";
		else if (!catLv4)
			m_sContent += "{{GameCategory|armaR|" + catLv1 + "|" + catLv2 + "|" + catLv3 + "}}\n";
		else if (!catLv5)
			m_sContent += "{{GameCategory|armaR|" + catLv1 + "|" + catLv2 + "|" + catLv3 + "|" + catLv4 + "}}\n";
		else
			m_sContent += "{{GameCategory|armaR|" + catLv1 + "|" + catLv2 + "|" + catLv3 + "|" + catLv4 + "|" + catLv5 + "}}\n";
	}

	//------------------------------------------------------------------------------------------------
	//! \return the content so far
	string GetContent()
	{
		return m_sContent;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the escaped content
	string GetEscapedContent()
	{
		return Escape(m_sContent);
	}

	//------------------------------------------------------------------------------------------------
	//! Empty the String Builder's content
	void Clear()
	{
		m_sContent = string.Empty;
	}

	//
	//	Static Methods
	//

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] input
	//! \return
	static string Escape(string input)
	{
		input.Replace("|", "{{!}}");
		input.Replace("=", "{{=}}");
		return input;
	}

	// formatting

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] massInKg
	//! \return
	static string FormatMass(float massInKg)
	{
		if (massInKg <= 0)
			return "-";

		if (massInKg < 1)
			return string.Format("%1&nbsp;g", Math.Round(massInKg * 1000));

		if (massInKg < 1000)
			return string.Format("%1&nbsp;kg", massInKg.ToString(-1, 2));

		return string.Format("%1&nbsp;T", massInKg.ToString(-1, 2));
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] metrePerSecond
	//! \return
	static string FormatSpeedMPS(float metrePerSecond)
	{
		if (metrePerSecond <= 0)
			return "-";

		if (metrePerSecond < 1)
			return string.Format("%1&nbsp;m/s", SCR_FormatHelper.FloatToDecString(metrePerSecond, 3));

		if (metrePerSecond < 10000)
			return string.Format("%1&nbsp;m/s", metrePerSecond.ToString(-1, 1));

		return string.Format("%1&nbsp;km/s", (metrePerSecond * 0.001).ToString(-1, 2));
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] metrePerSecond
	//! \return
	static string FormatSpeedKMH(float metrePerSecond)
	{
		if (metrePerSecond <= 0)
			return "-";

		metrePerSecond *= Physics.MS2KMH;
		if (metrePerSecond < 1)
			return string.Format("%1&nbsp;km/h", SCR_FormatHelper.FloatToDecString(metrePerSecond, 2));

		if (metrePerSecond < 1000)
			return string.Format("%1&nbsp;km/h", metrePerSecond.ToString(-1, 2));

		return string.Format("%1&nbsp;km/h", Math.Round(metrePerSecond));
	}

//	//------------------------------------------------------------------------------------------------
//	//!
//	//! \param[in] metrePerSecond
//	//! \return
//	static string FormatSpeed(float metrePerSecond)
//	{
//		// TODO
//		// TODO + Mach 1 etc
//	}

	// editor links

	//------------------------------------------------------------------------------------------------
	//! \param[in] resourceName
	//! \return
	static string GetResourceManagerLink(ResourceName resourceName)
	{
		if (!resourceName)
			return string.Empty;

		return "{{Link|enfusion:/" + "/ResourceManager/~ArmaReforger:" + resourceName.GetPath() + "}}";
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] scriptFilePath RELATIVE file path, only accepts .c files - returns empty string on any other type
	//! \return
	static string GetScriptEditorLink(string scriptFilePath)
	{
		if (!scriptFilePath.EndsWith(".c"))
			return string.Empty;

		return "{{Link|enfusion:/" + "/ScriptEditor/" + scriptFilePath + "}}";
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] resourceName only accepts .ent (world) and .et (Prefab) - returns empty string if anything else
	//! \return
	static string GetWorldEditorLink(ResourceName resourceName)
	{
		if (!resourceName)
			return string.Empty;

		if (!resourceName.EndsWith(".ent") && !resourceName.EndsWith(".et"))
			return string.Empty;

		return "{{Link|enfusion:/" + "/WorldEditor/" + resourceName.GetPath() + "}}";
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_BIKIStringBuilder(string content = string.Empty)
	{
		m_sContent = content;
	}
}
