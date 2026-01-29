/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Mission
\{
*/

class MissionHeader: ScriptAndConfig
{
	//! Returns the resource name of the mission
	proto external ResourceName GetHeaderResourceName();
	//! Returns the path to the world file
	proto external ResourceName GetWorldResourceName();
	//! Returns the resource name of the used system configuration
	proto external ResourceName GetWorldSystemsConfig();
	//! Returns the path to the world file
	proto external string GetWorldPath();
	//! Reads mission header object from given file
	static proto ref MissionHeader ReadMissionHeader(string path);
}

/*!
\}
*/
