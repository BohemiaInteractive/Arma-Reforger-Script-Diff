class SCR_LayerEditableEntityUIComponent : SCR_BaseEditableEntityUIComponent
{
	[Attribute("IconEmpty", desc: "Icon used when the entity does not have any child entities.")]
	protected string m_sEmptyIconWidgetName;
	
	[Attribute("IconLayer", desc: "Icon used when the entity is a layer, e.g., has some children.\nMUST BE HIDDEN BY DEFAULT!")]
	protected string m_sLayerIconWidgetName;
	
	protected bool m_bIsCustom;
	
	//------------------------------------------------------------------------------------------------
	protected void OnParentEntityChanged(SCR_EditableEntityComponent entity, SCR_EditableEntityComponent parent, SCR_EditableEntityComponent parentPrev)
	{
		if (parent == m_Entity || parentPrev == m_Entity)
		{
			bool isLayer;
			if (m_Entity)
				isLayer = m_Entity.IsLayer();
			
			Widget emptyIconWidget = GetWidget().FindAnyWidget(m_sEmptyIconWidgetName);
			if (emptyIconWidget)
				emptyIconWidget.SetVisible(!isLayer);
			
			Widget layerIconWidget = GetWidget().FindAnyWidget(m_sLayerIconWidgetName);
			if (layerIconWidget)
				layerIconWidget.SetVisible(isLayer);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_EditableEntityComponent entity, SCR_UIInfo info, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		if (info)
		{
			//--- Set custom texture
			ImageWidget emptyIconWidget = ImageWidget.Cast(GetWidget().FindAnyWidget(m_sEmptyIconWidgetName));
			m_bIsCustom = info.SetIconTo(emptyIconWidget);
		}
		
		//--- Use automatic textures only if the entity does not have custom one
		if (!m_bIsCustom && entity)
		{
			SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
			core.Event_OnParentEntityChanged.Insert(OnParentEntityChanged);
			if (core)
				OnParentEntityChanged(null, entity, null);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (!m_bIsCustom)
		{
			SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
			if (core)
				core.Event_OnParentEntityChanged.Remove(OnParentEntityChanged);
		}
	}
}
