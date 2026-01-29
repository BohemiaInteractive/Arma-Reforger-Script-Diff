[ComponentEditorProps(category: "GameScripted/Editor", description: "Camera data per session. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CameraDataEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
class SCR_CameraDataEditorComponent : SCR_BaseEditorComponent
{
	protected ref array<ref SCR_ManualCameraComponentSave> m_aSavedData;
	
	void SaveComponents(SCR_ManualCamera camera)
	{
		if (camera)
		{
			if (!m_aSavedData)
				m_aSavedData = {};
			
			camera.SaveComponents(m_aSavedData);
		}
	}
	void LoadComponents(SCR_ManualCamera camera)
	{
		if (m_aSavedData)
			camera.LoadComponents(m_aSavedData);
	}
	bool IsSave()
	{
		return m_aSavedData != null;
	}
};