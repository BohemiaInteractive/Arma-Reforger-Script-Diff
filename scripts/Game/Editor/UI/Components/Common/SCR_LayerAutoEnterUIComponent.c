class SCR_LayerAutoEnterUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute(defvalue: "1", desc: "How long it takes to toggle hovered entity automatically during entity transformation.")]
	protected float m_fAutoEnterDuration;
	
	[Attribute(defvalue: "0.2", uiwidget: UIWidgets.Slider, "", params: "0 1 0.01")]
	protected float m_fAutoEnterDelay;
	
	protected SCR_LayersEditorComponent m_LayersManager;
	protected SCR_PreviewEntityEditorComponent m_PreviewEntityManager;
	protected SCR_TransformingEditorComponent m_TransformingManager;
	protected SCR_HoverEditableEntityFilter m_HoverFilter;
	protected SCR_RadialProgressBarUIComponent m_RadialProgressUI;
	protected SCR_EditableEntityComponent m_AutoEnterTarget;
	protected float m_fAutoEnterCharge = -1;
	protected bool m_bCanCharge = true;
	
	//------------------------------------------------------------------------------------------------
	protected void OnTargetChange(SCR_EditableEntityComponent target)
	{
		//--- Not editing anymore or the target was deleted, cancel charge
		if (!target || !m_PreviewEntityManager.IsEditing())
		{
			Clear();
			return;
		}
		
		if (!m_bCanCharge)
			return;
		
		//--- Enter the layer only when hovering over its icon (i.e., delegate)
		bool isDelegate;
		SCR_EditableEntityComponent hoverEntity = m_HoverFilter.GetEntityUnderCursor(isDelegate);
		if (!isDelegate)
			return;
		
		//--- Filter out incompatible entities
		if (hoverEntity)
		{
			hoverEntity = m_LayersManager.GetParentBelowCurrentLayer(hoverEntity);
			if (!hoverEntity)
				return;

			//--- Cannot enter non-interactive entities or compatible slots (hovering snaps to them instead)
			if (!hoverEntity.HasEntityFlag(EEditableEntityFlag.LAYER) || m_PreviewEntityManager.GetTargetInteraction() != EEditableEntityInteraction.LAYER)
				return;
			
			//--- Cannot enter into itself
			if (m_TransformingManager && m_TransformingManager.IsEditing(hoverEntity))
				return;
		}
		
		//--- Start charging
		if (hoverEntity != m_AutoEnterTarget)
		{
			m_AutoEnterTarget = hoverEntity;
			m_fAutoEnterCharge = 0;
		}		
	}

	//------------------------------------------------------------------------------------------------
	protected void Clear()
	{
		m_fAutoEnterCharge = -1;
		m_AutoEnterTarget = null;
		GetWidget().SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetGracePeriod()
	{
		m_bCanCharge = true;
	}
	
	//---- REFACTOR NOTE START: Using call later as input delay and also using hardcoded value there

	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate(float tDelta)
	{
		//--- Enter hovered entity automatically while editing
		if (m_fAutoEnterCharge >= 0)
		{
			//--- Chaaaaarge!
			m_fAutoEnterCharge += tDelta;
			
			if (m_fAutoEnterCharge < m_fAutoEnterDelay)
				return;
			
			GetWidget().SetVisible(true);
			
			//--- Update progress bar
			if (m_RadialProgressUI)
				m_RadialProgressUI.SetProgress(Math.InverseLerp(m_fAutoEnterDelay, 1, m_fAutoEnterCharge) / m_fAutoEnterDuration);
			
			//--- Position the widget on the enetered layer
			vector worldPos;
			if (m_AutoEnterTarget.GetPos(worldPos))
			{
				vector screenPos = GetGame().GetWorkspace().ProjWorldToScreen(worldPos, m_AutoEnterTarget.GetOwner().GetWorld());
				FrameSlot.SetPos(GetWidget(), screenPos[0], screenPos[1]);
			}
			
			//--- Charge complete
			if (m_fAutoEnterCharge > m_fAutoEnterDuration)
			{
				m_LayersManager.ToggleCurrentLayer(m_AutoEnterTarget);
				Clear();
				
				//--- Disable charging right after entering a layer, so it won't instantly start charging sub-layer if there is another icon under cursor
				m_bCanCharge = false;
				GetGame().GetCallqueue().CallLater(ResetGracePeriod, 100);
			}
		}
	}
	
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	override protected bool IsUnique()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
			
		m_LayersManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent, true));
		m_HoverFilter = SCR_HoverEditableEntityFilter.Cast(SCR_HoverEditableEntityFilter.GetInstance(EEditableEntityState.HOVER, true));
		m_PreviewEntityManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent, true));
		m_TransformingManager = SCR_TransformingEditorComponent.Cast(SCR_TransformingEditorComponent.GetInstance(SCR_TransformingEditorComponent));
		m_RadialProgressUI = SCR_RadialProgressBarUIComponent.Cast(SCR_WidgetTools.FindHandlerInChildren(w, SCR_RadialProgressBarUIComponent));
		
		m_PreviewEntityManager.GetOnTargetChange().Insert(OnTargetChange);
		
		MenuRootBase menu = GetMenu();
		if (menu)
			menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
		
		Clear();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
			
		if (m_PreviewEntityManager)
			m_PreviewEntityManager.GetOnTargetChange().Remove(OnTargetChange);
		
		MenuRootBase menu = GetMenu();
		if (menu)
			menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
	}
}
