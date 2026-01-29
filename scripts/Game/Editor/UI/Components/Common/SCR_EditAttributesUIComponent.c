//! @ingroup Editor_UI Editor_UI_Components

class SCR_EditAttributesEditorUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute(desc: "Dialog created when attributes are edited.", defvalue: "-1", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ChimeraMenuPreset))]
	private ChimeraMenuPreset m_MenuPreset;

//	private SCR_AttributesManagerEditorComponent m_AttributesManager;
//
//	//------------------------------------------------------------------------------------------------
//	protected void OnEditingStart(array<SCR_BaseEditorAttribute> attributes)
//	{
//		if (m_MenuPreset == -1)
//			return;
//
//		EditorMenuBase menu = EditorMenuBase.Cast(GetMenu());
//		if (!menu)
//			return;
//
//		menu.OpenDialog(m_MenuPreset);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	override void HandlerAttachedScripted(Widget w)
//	{
//		m_AttributesManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
//		if (!m_AttributesManager)
//		{
//			Print("SCR_EditAttributesEditorUIComponent requires SCR_AttributesManagerEditorComponent!", LogLevel.ERROR);
//			return;
//		}
//
//		m_AttributesManager.GetOnAttributesStart().Insert(OnEditingStart);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	override void HandlerDeattached(Widget w)
//	{
//		super.HandlerDeattached(w);
//
//		m_AttributesManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
//		if (m_AttributesManager)
//			m_AttributesManager.GetOnAttributesStart().Remove(OnEditingStart);
//	}
}
