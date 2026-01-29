#ifdef WORKBENCH
[WorkbenchToolAttribute(
	"Coords Tool",
	"Tool that allows you to navigate to coordinates input or clipboard\n" +
	"or copy current one to clipboard, with entry history.\n" +
	"To use an entry, copy its link and use \"Go to clipboard link\"",
	awesomeFontCode : 0xF5A0)]
class SCR_CoordsTool : WorldEditorTool
{
	[Attribute(defvalue: "0 0 0", desc: "VectorCoordinates: Position in world space to set the camera position to.", category: "Coordinates")]
	protected vector m_vPosition;

	[Attribute(defvalue: "0 0 0", desc: "VectorCoordinates: Angles as pitch, yaw, roll to set the camera rotation to.", category: "Coordinates")]
	protected vector m_vRotation;

	[Attribute(defvalue: "0", desc: "Prefix the link with https prefix (" + WEB_PREFIX + ")", category: "Options")]
	protected bool m_bUseWebPrefix;

	[Attribute(desc: "Logged entries - most recent entry at the top", category: "Data")]
	protected ref array<ref SCR_CoordsTool_CoordsEntry> m_aEntries;

	protected static const string WEB_PREFIX = "https:/" + "/enfusionengine.com/api/redirect?to=";

	//------------------------------------------------------------------------------------------------
	// to be moved one day to SCR_WorldEditorToolHelper?
	protected static string GetCurrentWorldEditorLink(bool useWebPrefix)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
			return string.Empty;

		BaseWorld world = worldEditorAPI.GetWorld();
		if (!world)
			return string.Empty;

		vector transform[4];
		world.GetCurrentCamera(transform);

		return GetWorldEditorLink(transform, useWebPrefix);
	}

	//------------------------------------------------------------------------------------------------
	//! Get a World Editor link in Enfusion protocol format
	//! \param[in] transformation The transformation to create the link at.
	//! \return Enfusion protocol link, with or without the enfusionengine.com prefix
	static string GetWorldEditorLink(vector transformation[4], bool useWebPrefix = false)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
			return string.Empty;

		string fullLink;
		worldEditorAPI.GetWorldPath(fullLink);

		if (fullLink.IsEmpty())
			return string.Empty;

		// Fetch position
		vector position = transformation[3];

		// Fetch angles
		vector angles = Math3D.MatrixToAngles(transformation);
		if (angles == vector.Zero) // prevents 0 0 -0
			angles = vector.Zero;

		string worldPath = fullLink;
		if (worldPath[0] != "$")
		{
			// if we don't have an exact path (should not happen),
			// fallback to the original solution:

			// we want to substring only /worlds/(...)
			// to prevent exposing local folders, etc.
			int begin = fullLink.IndexOf("worlds\\");
			if (begin == -1)
				begin = fullLink.IndexOf("worlds/");

			if (begin == -1)
				return string.Empty;

			worldPath = fullLink.Substring(begin, fullLink.Length() - begin);
		}

		// Have consistent link
		worldPath.Replace("\\", "/");

		// Create link
		string link = string.Format(
			"enfusion://WorldEditor/%1;%2,%3,%4;%5,%6,%7",
			worldPath,
			position[0],
			position[1],
			position[2],
			angles[1],
			angles[0],
			angles[2]);

		if (useWebPrefix)
			link = WEB_PREFIX + link;

		return link;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets world editor camera position and rotation.
	//! \param[in] pos Position in world space.
	//! \param[in] rot Rotation as pitch, yaw, roll in degrees.
	protected void SetCamera(vector pos, vector rot)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
			return;

		vector yawPitchRoll = { rot[1], rot[0], rot[2] };
		vector lookDirection = yawPitchRoll.AnglesToVector();
		worldEditorAPI.SetCamera(pos, lookDirection);

		Print("Camera set to Position = " + pos + ", Rotation = " + rot, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Parses string for coordinates and returns true in case of success, false otherwise.
	//! \param[in] coordsLink link format enfusion://, https://enfusionengine.com/api/redirect?to=, or X,Y,Z;Pitch,Yaw,Roll
	//! \param[out] pos position
	//! \param[out] rot rotation
	//! \return coordinates extraction success status
	protected bool GetCoordsFromInput(string coordsLink, out vector pos, out vector rot)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(coordsLink))
			return false;

		if (coordsLink.StartsWith(WEB_PREFIX))
			coordsLink = coordsLink.Substring(WEB_PREFIX.Length(), coordsLink.Length() - WEB_PREFIX.Length());

		// Try parse as enflink to get coords
		// enf link is world;pos;rot
		if (coordsLink.Contains(";") && coordsLink.StartsWith("enfusion"))
		{
			int separatorIndex = coordsLink.IndexOf(";");
			coordsLink = coordsLink.Substring(separatorIndex + 1, coordsLink.Length() - separatorIndex - 1);
		}

		// split coords into two
		array<string> values = {};
		coordsLink.Split(";", values, true);

		// check validity
		int length = values.Count();
		if (length < 1)
			return false;

		// parse values
		string posString = values[0];
		posString.Replace(",", " ");
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(posString))
			return false;

		vector tempPos = posString.ToVector();
		vector tempRot;

		if (length > 1)
		{
			string rotString = values[1];
			if (!rotString.IsEmpty())
			{
				rotString.Replace(",", " ");
				tempRot = rotString.ToVector();
			}
		}

		pos = tempPos;
		rot = tempRot;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! When clicked by the user, sets the camera to desired position.
	[ButtonAttribute("Add entry")]
	protected void AddEntry()
	{
		if (!m_aEntries)
			m_aEntries = {};

		string link = GetCurrentWorldEditorLink(m_bUseWebPrefix);
		if (link.IsEmpty())
		{
			Print("Error obtaining the link", LogLevel.WARNING);
			return;
		}

		int count = m_aEntries.Count();
		if (count > 0)
		{
			SCR_CoordsTool_CoordsEntry firstEntry = m_aEntries[0];
			if (firstEntry.m_sLink == link)
			{
				Print("Current position is last log entry", LogLevel.NORMAL);
				return;
			}
		}

		string worldName;
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (worldEditorAPI)
		{
			worldEditorAPI.GetWorldPath(worldName);
			worldName = FilePath.StripExtension(FilePath.StripPath(worldName));
			if (!worldName.IsEmpty())
				worldName = " (" + worldName + ")";
		}

		SCR_CoordsTool_CoordsEntry entry = new SCR_CoordsTool_CoordsEntry();
		entry.m_sName = "Link #" + (count + 1) + worldName;
		entry.m_sLink = link;
		m_aEntries.InsertAt(entry, 0);
		UpdatePropertyPanel();
	}

	//------------------------------------------------------------------------------------------------
	//! Copies current transform and generates enfusion link, which is copied to clipboard.
	[ButtonAttribute("Copy link to clipboard")]
	protected void CopyLinkToClipboard()
	{
		string link = GetCurrentWorldEditorLink(m_bUseWebPrefix);
		if (link.IsEmpty())
		{
			Print("Link could not be copied, an error occurred.", LogLevel.WARNING);
			return;
		}

		System.ExportToClipboard(link);
		Print("Link copied to clipboard: " + link, LogLevel.NORMAL);
	}

	[ButtonAttribute("Go to clipboard link")]
	protected void NavigateToClipboardLink()
	{
		vector pos;
		vector rot;
		if (!GetCoordsFromInput(System.ImportFromClipboard(), pos, rot))
		{
			Print("String is in invalid format. Expected enfusion protocol link or in following format: \"x,y,z;pitch,yaw,roll\"!", LogLevel.NORMAL);
			return;
		}

		SetCamera(pos, rot);
	}

	//------------------------------------------------------------------------------------------------
	//! When clicked by the user, sets the camera to desired position.
	[ButtonAttribute("Go to coords")]
	protected void NavigateToCoords()
	{
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
