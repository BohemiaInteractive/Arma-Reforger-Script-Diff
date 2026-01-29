/*!
Used to store persistent data for Editor
*/
class SCR_EditorPersistentData: ModuleGameSettings
{	
	[Attribute(desc: "A list of saved states of the content browser such as labels and pagination.")]
	protected ref array<ref SCR_EditorContentBrowserSaveStateData> m_aSavedContentBrowserStates;
};