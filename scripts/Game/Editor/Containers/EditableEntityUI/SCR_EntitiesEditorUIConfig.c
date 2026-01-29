/** @ingroup Editor_Editable_Entity_UI
*/
/*!
Config which defines visual representation of editable entities based on various rules.
*/
[BaseContainerProps(configRoot: true)]
class SCR_EditableEntityUIConfig
{
	[Attribute()]
	protected ref array<ref SCR_EntitiesEditorUIRule> m_aRules;
	
	/*!
	Get rules defining icons based on specific rules.
	\return Array of rules
	*/
	array<ref SCR_EntitiesEditorUIRule> GetRules()
	{
		return m_aRules;
	}
	
	/*!
	Get editable entity UI config.
	\param config UI config. Wnen null, it will search for shared config in SCR_MenuLayoutEditorComponent
	\return Editable entity UI config
	*/
	static SCR_EditableEntityUIConfig GetConfig(SCR_EditableEntityUIConfig config = null)
	{
		if (config)
			return config;
		
		SCR_MenuLayoutEditorComponent layoutManager = SCR_MenuLayoutEditorComponent.Cast(SCR_MenuLayoutEditorComponent.GetInstance(SCR_MenuLayoutEditorComponent, true));
		if (layoutManager)
			return layoutManager.GetEditableEntityUI();
		else
			return null;
	}
};