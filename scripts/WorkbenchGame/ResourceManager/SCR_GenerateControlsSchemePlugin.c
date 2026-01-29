#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Generate Controls Scheme", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF11B)]
class SCR_GenerateControlsSchemePlugin : WorkbenchPlugin
{
	[Attribute(desc: "A list of contexts to be exported. Actions from all contexts will be merged into one image.")]
	protected ref array<string> m_aContextNames;

	[Attribute(defvalue: "$profile:GenerateControlsScheme/", desc: "Directory at which the scheme will be saved")]
	protected string m_sDirectory;

	//[Attribute("2", UIWidgets.ComboBox, "", enumType: SCR_EGenerateControlsSchemeType)]
	protected SCR_EGenerateControlsSchemeType m_eType = SCR_EGenerateControlsSchemeType.GAMEPAD;

	[Attribute(defvalue: "1", desc: "Font size multiplier. Use a smaller font when action names are too long and overflow from the image.")]
	protected float m_fFontSize;

	[Attribute(defvalue: "1", desc: "True to label actions that are not used in given contexts. Unassigned actions will be shown in faded color.")]
	protected bool m_bShowUnassigned;

	//------------------------------------------------------------------------------------------------
	//! Controller
	bool ProcessBinding_Gamepad(map<string, ref array<string>> allBindings, string fullPath, string contextTitle)
	{
		bool didCopy = FileIO.CopyFile("scripts/WorkbenchGame/GenerateControlsScheme_Gamepad.txt", fullPath);
		if (!didCopy)
		{
			Print(string.Format("Cannot copy header to '%1'!", fullPath), LogLevel.ERROR);
			return false;
		}

		FileHandle file = FileIO.OpenFile(fullPath, FileMode.APPEND);
		if (!file)
		{
			Print(string.Format("Cannot open file at '%1'!", fullPath), LogLevel.ERROR);
			return false;
		}

		const float tL1 = 280;
		const float lL1 = 290;
		const float lL2 = 370;

		const float tR1 = 800;
		const float lR1 = 790;
		const float lR2 = 710;

		map<string, ref SCR_GenerateControlsSchemeGamepadLabel> labels = new map<string, ref SCR_GenerateControlsSchemeGamepadLabel>();
		labels.Insert("gamepad0:view", 						new SCR_GenerateControlsSchemeGamepadLabel("end", 	500, 040, 510, 510, 035, 510, 245));
		labels.Insert("gamepad0:menu", 						new SCR_GenerateControlsSchemeGamepadLabel("start", 580, 040, 572, 572, 035, 572, 245));

		labels.Insert("gamepad0:left_trigger", 				new SCR_GenerateControlsSchemeGamepadLabel("end", 	tL1, 070, lL1, lL2, 065, 450, 171));
		labels.Insert("gamepad0:shoulder_left", 			new SCR_GenerateControlsSchemeGamepadLabel("end", 	tL1, 130, lL1, lL2, 125, 420, 185));
		labels.Insert("gamepad0:left_thumb_horizontal", 	new SCR_GenerateControlsSchemeGamepadLabel("end", 	tL1, 250, lL1, lL2, 275, 420, 250, "⭤"));
		labels.Insert("gamepad0:left_thumb_vertical", 		new SCR_GenerateControlsSchemeGamepadLabel("end", 	tL1, 280, lL1, lL2, 275, 420, 250, "⭥"));
		labels.Insert("gamepad0:thumb_left", 				new SCR_GenerateControlsSchemeGamepadLabel("end", 	tL1, 310, lL1, lL2, 275, 420, 250, "●"));
		labels.Insert("gamepad0:pad_up", 					new SCR_GenerateControlsSchemeGamepadLabel("end", 	tL1, 360, lL1, lL2, 400, 460, 320, "▲"));//355, 480, 295, "▲"));
		labels.Insert("gamepad0:pad_left", 					new SCR_GenerateControlsSchemeGamepadLabel("end", 	tL1, 390, lL1, lL2, 400, 460, 320, "◄"));//385, 460, 325, "◄"));
		labels.Insert("gamepad0:pad_right", 				new SCR_GenerateControlsSchemeGamepadLabel("end", 	tL1, 420, lL1, lL2, 400, 460, 320, "►"));//415, 505, 320, "►"));
		labels.Insert("gamepad0:pad_down", 					new SCR_GenerateControlsSchemeGamepadLabel("end", 	tL1, 450, lL1, lL2, 400, 460, 320, "▼"));//445, 490, 345, "▼"));

		labels.Insert("gamepad0:right_trigger", 			new SCR_GenerateControlsSchemeGamepadLabel("start", tR1, 070, lR1, lR2, 065, 630, 171));
		labels.Insert("gamepad0:shoulder_right", 			new SCR_GenerateControlsSchemeGamepadLabel("start", tR1, 130, lR1, lR2, 125, 660, 185));
		labels.Insert("gamepad0:y", 						new SCR_GenerateControlsSchemeGamepadLabel("start", tR1, 190, lR1, lR2, 185, 660, 220));
		labels.Insert("gamepad0:x", 						new SCR_GenerateControlsSchemeGamepadLabel("start", tR1, 220, lR1, lR2, 215, 630, 250));
		labels.Insert("gamepad0:b", 						new SCR_GenerateControlsSchemeGamepadLabel("start", tR1, 250, lR1, lR2, 245, 690, 250));
		labels.Insert("gamepad0:a", 						new SCR_GenerateControlsSchemeGamepadLabel("start", tR1, 280, lR1, lR2, 275, 660, 280));
		labels.Insert("gamepad0:right_thumb_horizontal", 	new SCR_GenerateControlsSchemeGamepadLabel("start", tR1, 360, lR1, lR2, 385, 610, 320, "⭤"));
		labels.Insert("gamepad0:right_thumb_vertical", 		new SCR_GenerateControlsSchemeGamepadLabel("start", tR1, 390, lR1, lR2, 385, 610, 320, "⭥"));
		labels.Insert("gamepad0:thumb_right", 				new SCR_GenerateControlsSchemeGamepadLabel("start", tR1, 420, lR1, lR2, 385, 610, 320, "●"));

		string texts = string.Format("<text x='20' y='570' text-anchor='start' font-size='25' font-weight='bold'>%1</text>\n", contextTitle);
		string lines;
		string textsDisabled;
		string linesDisabled;
		SCR_GenerateControlsSchemeGamepadLabel label;
		array<string> bindings;
		for (int l, countL = labels.Count(); l < countL; l++)
		{
			label = labels.GetElement(l);
			if (allBindings.Find(labels.GetKey(l), bindings))
			{
				label.AddText(texts, bindings[0]);
				label.AddLines(lines);
			}
			else if (m_bShowUnassigned)
			{
				label.AddText(textsDisabled, "N/A");
				label.AddLines(linesDisabled);
			}
		}

		file.WriteLine(string.Format("<g font-family='Roboto' font-size='%1' fill='#ccc'>", 20 * m_fFontSize));
		file.WriteLine(textsDisabled);
		file.WriteLine("</g>");

		file.WriteLine(string.Format("<g font-family='Roboto' font-size='%1' fill='black'>", 20 * m_fFontSize));
		file.WriteLine(texts);
		file.WriteLine("</g>");

		file.WriteLine("<g stroke='black' stroke-width='2' opacity='0.1' transform='translate(0.5,0.5)'>");
		file.WriteLine(linesDisabled);
		file.WriteLine("</g>");

		file.WriteLine("<g stroke='black' stroke-width='2' opacity='1' transform='translate(0.5,0.5)'>");
		file.WriteLine(lines);
		file.WriteLine("</g>");

		file.WriteLine("</svg>");

		file.Close();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Keyboard
	protected bool ProcessBinding_Keyboard(map<string, ref array<string>> allBindings, string fileName, string contextTitle)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse
	protected bool ProcessBinding_Mouse(map<string, ref array<string>> allBindings, string fileName, string contextTitle)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Find all relevant actions and generate the image
	protected void Generate()
	{
		if (!m_aContextNames)
			return;

		InputBinding input = GetGame().GetInputManager().CreateUserBinding();

		//--- Whitelist specific keys based on selected input m_eType
		array<string> whitelist = {};
		switch (m_eType)
		{
			case SCR_EGenerateControlsSchemeType.KEYBOARD:
				break;

			case SCR_EGenerateControlsSchemeType.MOUSE:
				break;

			case SCR_EGenerateControlsSchemeType.GAMEPAD:
				whitelist.Insert("gamepad0:menu");
				whitelist.Insert("gamepad0:view");
				whitelist.Insert("gamepad0:a");
				whitelist.Insert("gamepad0:b");
				whitelist.Insert("gamepad0:x");
				whitelist.Insert("gamepad0:y");
				whitelist.Insert("gamepad0:pad_up");
				whitelist.Insert("gamepad0:pad_down");
				whitelist.Insert("gamepad0:pad_left");
				whitelist.Insert("gamepad0:pad_right");
				whitelist.Insert("gamepad0:shoulder_left");
				whitelist.Insert("gamepad0:shoulder_right");
				whitelist.Insert("gamepad0:thumb_left");
				whitelist.Insert("gamepad0:thumb_right");
				whitelist.Insert("gamepad0:left_thumb_horizontal");
				whitelist.Insert("gamepad0:left_thumb_vertical");
				whitelist.Insert("gamepad0:right_thumb_horizontal");
				whitelist.Insert("gamepad0:right_thumb_vertical");
				whitelist.Insert("gamepad0:left_trigger");
				whitelist.Insert("gamepad0:right_trigger");
				break;
		}

		//--- Find relevant actions
		map<string, ref array<string>> allBindings = new map <string, ref array<string>>();
		string contextTitle;
		foreach (string contextName : m_aContextNames)
		{
			BaseContainer context = input.FindContext(contextName);
			if (!context)
			{
				Print(string.Format("Context '%1' not found!", contextName), LogLevel.ERROR);
				continue;
			}

			array<string> actionNames = {};
			if (!context.Get("ActionRefs", actionNames))
				continue;

			foreach (string actionName : actionNames)
			{
				array<string> bindings = {};
				if (!input.GetBindings(actionName, bindings, EInputDeviceType.MOUSE))
					continue;

				if (!input.GetBindings(actionName, bindings, EInputDeviceType.KEYBOARD))
					continue;

				if (!input.GetBindings(actionName, bindings, EInputDeviceType.GAMEPAD))
					continue;

				if (!input.GetBindings(actionName, bindings, EInputDeviceType.JOYSTICK))
					continue;

				foreach (string binding : bindings)
				{
					if (whitelist.Find(binding) < 0)
						continue;

					array<string> currentBindings;
					allBindings.Find(binding, currentBindings);
					if (!currentBindings)
						currentBindings = {};

					currentBindings.Insert(actionName);
					allBindings.Insert(binding, currentBindings);
				}
			}

			if (contextTitle != "")
				contextTitle += ", ";

			contextTitle += contextName;
		}

		string fileName;
		foreach (string contextName : m_aContextNames)
		{
			fileName += "_" + contextName;
		}

		string directoryLocal = m_sDirectory;
		if (!directoryLocal.EndsWith("/"))
			directoryLocal += "/";

		string fullPath;
		bool succeeded;
		switch (m_eType)
		{
			case SCR_EGenerateControlsSchemeType.KEYBOARD:
			{
				fullPath = directoryLocal + "keyboard" + fileName + ".svg";
				succeeded = ProcessBinding_Keyboard(allBindings, fullPath, contextTitle);
				break;
			}
			case SCR_EGenerateControlsSchemeType.MOUSE:
			{
				fullPath = directoryLocal + "mouse" + fileName + ".svg";
				succeeded = ProcessBinding_Mouse(allBindings, fullPath, contextTitle);
				break;
			}
			case SCR_EGenerateControlsSchemeType.GAMEPAD:
			{
				fullPath = directoryLocal + "gamepad" + fileName + ".svg";
				succeeded = ProcessBinding_Gamepad(allBindings, fullPath, contextTitle);
				break;
			}
		}

		if (succeeded)
			Print(string.Format("Controls scheme generated in '%1'", fullPath), LogLevel.DEBUG);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default functions
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (Workbench.ScriptDialog("Generate Controls Scheme", "Generate a *.svg file which lists all actions in given contexts.\nThe file can be attached to Confluence page to showcase a control scheme.", this))
			Generate();
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
	protected bool ButtonOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}

enum SCR_EGenerateControlsSchemeType
{
	KEYBOARD,
	MOUSE,
	GAMEPAD
}

class SCR_GenerateControlsSchemeGamepadLabel
{
	protected string m_sAnchor;
	protected float m_fTextX;
	protected float m_fTextY;
	protected float m_fLineX1;
	protected float m_fLineX2;
	protected float m_fLineY;
	protected float m_fTargetX;
	protected float m_fTargetY;
	protected string m_sSymbol;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] anchor
	//! \param[in] textX
	//! \param[in] textY
	//! \param[in] lineX1
	//! \param[in] lineX2
	//! \param[in] lineY
	//! \param[in] targetX
	//! \param[in] targetY
	//! \param[in] symbol
	void SCR_GenerateControlsSchemeGamepadLabel(string anchor, float textX, float textY, float lineX1, float lineX2, float lineY, float targetX, float targetY, string symbol = "")
	{
		m_sAnchor = anchor;
		m_fTextX = textX;
		m_fTextY = textY;
		m_fLineX1 = lineX1;
		m_fLineX2 = lineX2;
		m_fLineY = lineY;
		m_fTargetX = targetX;
		m_fTargetY = targetY;
		m_sSymbol = symbol;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] text
	//! \param[in] binding
	void AddText(out string text, string binding)
	{
		if (m_sSymbol != "")
		{
			if (m_sAnchor == "end")
				binding = binding + " " + m_sSymbol;
			else
				binding = m_sSymbol + " " + binding;
		}

		text += string.Format("<text x='%3' y='%4' text-anchor='%2'>%1</text>\n", binding, m_sAnchor, m_fTextX, m_fTextY);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] text
	void AddLines(out string text)
	{
		//text += string.Format("<text x='%3' y='%4' text-anchor='%2'>%1</text>\n", binding, m_sAnchor, m_fTextX, m_fTextY);
		text += string.Format("<line x1='%1' x2='%2' y1='%3' y2='%3' />\n", m_fLineX1, m_fLineX2, m_fLineY, m_fTargetX, m_fTargetY);
		text += string.Format("<line x1='%2' x2='%4' y1='%3' y2='%5' />\n", m_fLineX1, m_fLineX2, m_fLineY, m_fTargetX, m_fTargetY);
	}
}
#endif // WORKBENCH
