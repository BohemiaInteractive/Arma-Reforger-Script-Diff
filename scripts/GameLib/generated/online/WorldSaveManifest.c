/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class WorldSaveManifest: Managed
{
	ref array<string> m_aFileNames; //list of files to upload
	ref array<ref JsonApiStruct> m_aFiles; //list of files to upload - WIP
	ref array<string> m_aDependencyIds;
	ref array<string> m_aScreenshots;
	bool m_bUnlisted = false;
	string m_sName;
	string m_sSummary;
	string m_sDescription;
	string m_sPreview; //name of the file with preview image - its full path has to be among other files in m_aFiles
	string m_sScenarioId;

}

/*!
\}
*/
