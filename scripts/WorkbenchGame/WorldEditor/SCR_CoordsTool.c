#ifdef WORKBENCH
[WorkbenchToolAttribute(
	PLUGIN_NAME,
	"Navigate to input/clipboard link/coordinates with bookmarks."
	+ "\nTo use an entry, copy its link/coordinates and use \"Go to clipboard link\"."
	+ "\nTo copy the current camera's link, use the World Editor's Ctrl+Shift+L shortcut.",
	awesomeFontCode: 0xF5A0)]
class SCR_CoordsTool : WorldEditorTool
{
	/*
		Coordinates
	*/

	[Attribute(defvalue: "0 0 0", desc: "Position in world space to set the camera position to.", category: "Coordinates")]
	protected vector m_vPosition;

	[Attribute(defvalue: "0 0 0", desc: "Pitch, yaw, roll to set the camera rotation to.", category: "Coordinates")]
	protected vector m_vRotation;

	[Attribute(desc: "In-game map coordinates - if defined, clicking \"Go to coords\" goes to these coordinates (empty the field to get the Position field to work again)", category: "Coordinates")]
	protected string m_sMapCoordinates;

	/*
		Data
	*/

	[Attribute(defvalue: "0", desc: "Use full link and save/load world, otherwise only save/load coordinates", category: "Data")]
	protected bool m_bUseFullLink;

	[Attribute(desc: "Logged entries - most recent entry at the top", category: "Data")]
	protected ref array<ref SCR_CoordsTool_CoordsEntry> m_aEntries;

	protected static const float MIN_CAMERA_Y = 5;
	protected static const string WEB_PREFIX = "https://enfusionengine.com/api/redirect?to=";

	protected static const string PLUGIN_NAME = "Coords Tool";

	//------------------------------------------------------------------------------------------------
	//! Sets world editor camera position and rotation.
	//! \param[in] pos Position in world space.
	//! \param[in] rot Rotation as pitch, yaw, roll in degrees.
	protected void SetCamera(vector pos, vector rot)
	{
		vector yawPitchRoll = { rot[1], rot[0], rot[2] };
		vector lookDirection = yawPitchRoll.AnglesToVector();
		m_API.SetCamera(pos, lookDirection);

		PrintFormat("Camera set to Position %1 Rotation %2", pos, rot, level: LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Parses string for coordinates and returns true in case of success, false otherwise.
	//! \param[in] input link format (https://enfusionengine.com/api/redirect?to=)enfusion://, X,Y,Z;Pitch,Yaw,Roll or map coordinates (e.g 123 456)
	//! \param[out] pos position
	//! \param[out] rot rotation
	//! \return coordinates extraction success status
	protected bool GetCoordsFromInput(string input, out vector pos, out vector rot)
	{
		input.TrimInPlace();
		if (!input)
			return false;

		if (GetCoordsFromString(input, pos, rot))
			return true;

		if (GetCoordsFromMapCoordsString(input, pos, rot))
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetCoordsFromString(string input, out vector pos, out vector rot)
	{
		input.Replace(WEB_PREFIX, "");

		// Try parse as enflink to get coords
		// enf link is world;pos;rot
		if (input.Contains(";") && input.StartsWith("enfusion"))
		{
			int separatorIndex = input.IndexOf(";");
			input = input.Substring(separatorIndex + 1, input.Length() - separatorIndex - 1);
		}

		// split coords into two
		array<string> values = {};
		input.Split(";", values, true);

		// check validity
		int length = values.Count();
		if (length < 1)
			return false;

		// parse values
		string posString = values[0];
		posString.Replace(",", " ");
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(posString))
			return false;

		pos = posString.ToVector();
		if (length > 1)
		{
			string rotString = values[1];
			if (!rotString.IsEmpty())
			{
				rotString.Replace(",", " ");
				rot = rotString.ToVector();
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetCoordsFromMapCoordsString(string input, out vector pos, out vector rot)
	{
		if (!input)
			return false;

		string mapCoordinates = SCR_StringHelper.Filter(input, SCR_StringHelper.DIGITS);
		int length = mapCoordinates.Length();

		if (length % 2 != 0)
			return false;

		int halfLength = length * 0.5;
		int leftCoords = mapCoordinates.Substring(0, halfLength).ToInt();
		int rightCoords = mapCoordinates.Substring(halfLength, halfLength).ToInt();

		float multiplier = Math.Pow(10, 5 - halfLength);	// 1000 for 00 00, 100 for 000 000, 10 for 0000 0000
															// 1 for 00000 00000, 0.1 for 000000 000000

		rot = { -45, 0, 0 };
		pos = { leftCoords * multiplier + 0.5 * multiplier, 0, rightCoords * multiplier };
		pos[1] = m_API.GetTerrainSurfaceY(pos[0], pos[2]);

		if (multiplier < MIN_CAMERA_Y)
			multiplier = MIN_CAMERA_Y;

		if (pos[1] <= 0)
			pos[1] = multiplier;
		else
			pos[1] = pos[1] + multiplier;

		PrintFormat("Parsed game coordinates: %1 %2", leftCoords.ToString(halfLength), rightCoords.ToString(halfLength), level: LogLevel.NORMAL);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Add entry")]
	protected void AddEntry()
	{
		if (!m_aEntries)
			m_aEntries = {};

		string link = SCR_WorldEditorToolHelper.GetCurrentWorldEditorLink();
		if (link.IsEmpty())
		{
			Print("Error obtaining the link", LogLevel.WARNING);
			return;
		}

		// FOR NOW strip world path as it is not useful
		if (!m_bUseFullLink)
		{
			int semicolonIndex = link.IndexOf(";");
			if (semicolonIndex > -1)
				link = link.Substring(semicolonIndex + 1, link.Length() - semicolonIndex - 1);
		}

		foreach (int i, SCR_CoordsTool_CoordsEntry entry : m_aEntries)
		{
			if (entry.m_sLink == link)
			{
				if (i == 0)
					Print("Current position is the last entry", LogLevel.NORMAL);
				else
					Print("Current position is already stored as entry #" + (i + 1), LogLevel.NORMAL);

				return;
			}
		}

		SCR_CoordsTool_CoordsEntry entry = new SCR_CoordsTool_CoordsEntry();
		entry.m_sName = string.Format("Link #%1 (%2 world)", m_aEntries.Count() + 1, SCR_WorldEditorToolHelper.GetWorldName());
		entry.m_sLink = link;
		m_aEntries.InsertAt(entry, 0);
		UpdatePropertyPanel();
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Load clipboard")]
	protected void NavigateToClipboardLink()
	{
		string input = System.ImportFromClipboard();
		vector pos, rot;
		if (!GetCoordsFromInput(input, pos, rot))
		{
			Print("Clipboard data is in invalid format. Expected:"
				+ "\n- enfusion link format \"enfusion://WorldEditor/world/path.ent;x,y,z(;pitch,yaw,roll)\""
				+ "\n- coordinates format: \"x,y,z(;pitch,yaw,roll)\""
				+ "\n- game map coordinates format e.g \"123 456\" (any even length, with or without space)",
				LogLevel.WARNING);

			return;
		}

		if (m_bUseFullLink && input.StartsWith("enfusion://WorldEditor/"))
		{
			string worldPath;
			m_API.GetWorldPath(worldPath);
			if (worldPath.StartsWith("$"))
				worldPath = "~" + worldPath.Substring(1, worldPath.Length() - 1);

			if (!input.Contains(worldPath)) // same world, no need to reload
			{
				input = input.Substring(23, input.Length() - 23);
				int charIndex = input.IndexOf(";");
				if (charIndex > -1)
					input = input.Substring(0, charIndex);
	
				charIndex = input.IndexOf("~");
				if (charIndex > -1)
				{
					charIndex = input.IndexOfFrom(charIndex, ":");
					if (charIndex > -1)
						input = input.Substring(charIndex + 1, input.Length() - charIndex - 1);
				}
	
				WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
				worldEditor.SetOpenedResource(input);
			}
		}

		SetCamera(pos, rot);
	}

	//------------------------------------------------------------------------------------------------
	//! When clicked by the user, sets the camera to desired position.
	[ButtonAttribute("Go to coords")]
	protected void NavigateToCoords()
	{
		vector pos, rot;
		if (GetCoordsFromMapCoordsString(m_sMapCoordinates, pos, rot))
		{
			SetCamera(pos, rot);
			return;
		}

		SetCamera(m_vPosition, m_vRotation);
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class SCR_CoordsTool_CoordsEntry
{
	[Attribute()]
	string m_sName;

	[Attribute()]
	string m_sLink;
}
#endif // WORKBENCH
