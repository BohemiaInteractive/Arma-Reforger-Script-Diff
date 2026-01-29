/** @ingroup Editor_Editable_Entity_UI
*/
/*!
Configuration of entity icons for given states.
Controlled by SCR_EditableEntityUIConfig;
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleFlags(EEditableEntityState, "m_States")]
class SCR_EntitiesEditorUIRule
{
	//! Entity states this component visualizes.
	[Attribute(desc: "Entity states this component visualizes.", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_States;

	//! When interactive, icons can be selected by mouse or gamepad.
	[Attribute(desc: "When interactive, icons can be selected by mouse or gamepad.")]
	protected bool m_bInteractive;
	
	[Attribute(desc: "When enabled, the icon will be shown only if other rules are active, it won't force visibility.")]
	protected bool m_bIsDependent;
	
	//! Layouts assigned to each supported entity type.
	[Attribute(desc: "Layouts assigned to each supported entity type.")]
	protected ref array<ref SCR_EntitiesEditorUIPrefab> m_PrefabLinks;
	
	protected ref map<EEditableEntityType, ResourceName> m_PrefabLinksMap = new map<EEditableEntityType, ResourceName>;
	protected ResourceName m_DefaultLayout;
	
	ResourceName GetLayout(EEditableEntityType type)
	{
		ResourceName layout;
		if (!m_PrefabLinksMap.Find(type, layout))
			layout = m_DefaultLayout;
		
		if (!layout)
			Print(string.Format("No UI layout found for an entity of type '%1'!", typename.EnumToString(EEditableEntityType, type)), LogLevel.WARNING);
		
		return layout;
	}
	EEditableEntityState GetStates()
	{
		return m_States;
	}
	int GetStatesArray(out array<EEditableEntityState> outStates)
	{
		return SCR_Enum.BitToIntArray(m_States, outStates);
	}
	bool IsInteractive()
	{
		return m_bInteractive;
	}
	bool IsDependent()
	{
		return m_bIsDependent;
	}
	
	void SCR_EntitiesEditorUIRule()
	{
		//--- Build prefab map
		foreach (SCR_EntitiesEditorUIPrefab link: m_PrefabLinks)
		{
			m_PrefabLinksMap.Insert(link.GetType(), link.GetLayout());
		}
		m_PrefabLinks = null; //--- Erase the value afterwards, it's in m_PrefabLinksMap now
		
		if (!m_PrefabLinksMap.Find(EEditableEntityType.GENERIC, m_DefaultLayout))
		{
			Print(string.Format("%1 for type %2 is missing GENERIC type!", Type(), Type().EnumToString(EEditableEntityType, EEditableEntityType.GENERIC)), LogLevel.ERROR);
		}
	}
};