[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_CommentPlacablePrefabDetail : SCR_EntityTooltipDetail
{
	protected TextWidget m_wText;
	protected SCR_EditableEntityComponent m_Entity;
	protected SCR_ContentBrowserEditorComponent m_ContentBrowserComponent;

	protected int m_iExtendedPrefabsCount;

	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		if (m_iExtendedPrefabsCount > 0)
			m_wText.SetText(m_iExtendedPrefabsCount.ToString());
		else
			m_wText.SetText(string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	protected int GetExtendedEntityCount()
	{
		m_ContentBrowserComponent.SetExtendedEntity(m_Entity);
		return m_ContentBrowserComponent.FilterExtendedSlots();
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_wText = TextWidget.Cast(widget);
		m_Entity = entity;
		m_ContentBrowserComponent = SCR_ContentBrowserEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent));

		if (!m_wText || !m_Entity || !m_ContentBrowserComponent)
			return false;

		m_bShowLabel = m_iExtendedPrefabsCount > 0;
		return m_bShowLabel;
	}
}
