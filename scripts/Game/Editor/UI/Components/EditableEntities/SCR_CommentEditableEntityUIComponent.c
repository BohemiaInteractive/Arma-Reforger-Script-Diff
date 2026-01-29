class SCR_CommentEditableEntityUIComponent : SCR_BaseEditableEntityUIComponent
{
	protected static const string WIDGET_TEXT = "Text"; //--- ToDo: Don't hardcode
	protected static const string WIDGET_ICON = "Icon"; //--- ToDo: Don't hardcode
	protected static const string WIDGET_EXTENDED_ICON = "Icon_ExtendablePrefabs";
	protected SCR_ContentBrowserEditorComponent m_ContentBrowserComponent;

	protected Widget m_wExtendedIcon;

	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_EditableEntityComponent entity, SCR_UIInfo info, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		Widget widget = GetWidget();
		if (!widget)
			return;

		GenericEntity owner = entity.GetOwner();
		if (!owner)
			return;

		SCR_EditableCommentComponent comment = SCR_EditableCommentComponent.Cast(entity);
		if (!comment)
			return;

		TextWidget textWidget = TextWidget.Cast(widget.FindAnyWidget(WIDGET_TEXT));
		if (textWidget)
			comment.ApplyTo(textWidget);

		ImageWidget iconWidget = ImageWidget.Cast(widget.FindAnyWidget(WIDGET_ICON));
		if (iconWidget)
			comment.ApplyTo(iconWidget);

		m_ContentBrowserComponent = SCR_ContentBrowserEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent));
		if (!m_ContentBrowserComponent)
			return;

		m_wExtendedIcon = GetWidget().FindAnyWidget(WIDGET_EXTENDED_ICON);
		if (!m_wExtendedIcon)
			return;

		SCR_CompositionSlotManagerComponent slotManager = SCR_CompositionSlotManagerComponent.GetInstance();
		if (slotManager)
			slotManager.GetOnEntityChanged().Insert(OnEntityChanged);

		if (slotManager)
			m_wExtendedIcon.SetVisible(!slotManager.IsOccupied(owner) && m_ContentBrowserComponent.IsExtendedEntity(entity));
		else
			m_wExtendedIcon.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] slotManager
	//! \param[in] slot
	void OnEntityChanged(SCR_CompositionSlotManagerComponent slotManager, IEntity slot)
	{
		if (!m_ContentBrowserComponent)
			return;

		if (slot == GetEntity().GetOwnerScripted() && m_wExtendedIcon)
			m_wExtendedIcon.SetVisible(!slotManager.IsOccupied(slot) && m_ContentBrowserComponent.IsExtendedEntity(m_Entity));
	}
}
