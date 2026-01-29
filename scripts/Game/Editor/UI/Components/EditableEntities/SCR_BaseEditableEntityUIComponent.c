//! @ingroup Editor_UI Editor_UI_Components

//! Component to manage visualisation of individual entity widget.
//! - Provides base from which more specialised entity widgets can inherit (e.g., SCR_GroupEditableEntityUIComponent for showing groups).
//! 	+ Naming convention of inherited classes should be *SCR_<Something>EditableEntityUIComponent*.
//! - Attached to root widget of a layout which is linked from SCR_EntitiesEditorUIComponent.
//! 	+ Refresh() is called continuously from there to update widget position.
class SCR_BaseEditableEntityUIComponent : ScriptedWidgetComponent
{
	[Attribute(desc: "When true, the icon will remain visible on edge of the screen when it's not in the view.")]
	private bool m_bShowOffScreen;
	
	private Widget m_Widget;
	protected SCR_EditableEntityComponent m_Entity;
	private WorkspaceWidget m_Workspace;
	private float m_fFade;
	
	//------------------------------------------------------------------------------------------------
	//! Method called when the widget is updsted from the entity.
	//! \param[in] slot Slot to which this widget is attached to
	void OnRefresh(SCR_EditableEntityBaseSlotUIComponent slot);

	//------------------------------------------------------------------------------------------------
	//! Method called when the widget is initialised.
	//! \param[in] entity Editable entity
	//! \param[in] slot Slot to which this widget is attached to
	//!
	void OnInit(SCR_EditableEntityComponent entity, SCR_UIInfo info, SCR_EditableEntityBaseSlotUIComponent slot);

	//------------------------------------------------------------------------------------------------
	//! Method called the icon moves from on-screen to off-screen and vice versa
	//! \param[in] offScreen True when off-screen
	void OnShownOffScreen(bool offScreen);
	
	//------------------------------------------------------------------------------------------------
	//! Initialise entity widget using editable entity.
	//! \param[in] entity Editable entity which the widget represents
	//! \param[in] slot Slot in which the icon was created
	void Init(SCR_EditableEntityComponent entity, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		if (entity)
		{
			m_Entity = entity;
			OnInit(entity, entity.GetInfo(), slot)
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Initialise entity widget using UI info.
	//! \param[in] info UI info representing an entity
	//! \param[in] slot Slot in which the icon was created
	//!
	sealed void Init(SCR_UIInfo info, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		if (info)
			OnInit(null, info, slot)
	}

	//------------------------------------------------------------------------------------------------
	//! Terminate entity widget.
	//! \param[in] slot
	void Exit(SCR_EditableEntityBaseSlotUIComponent slot)
	{
		if (m_Widget)
		{
			m_Widget.RemoveFromHierarchy();
			m_Widget = null;
		}
	}
	//------------------------------------------------------------------------------------------------
	//! \return the entity which the widget represents.
	SCR_EditableEntityComponent GetEntity()
	{
		return m_Entity;
	}

	//------------------------------------------------------------------------------------------------
	//! \return widget root
	Widget GetWidget()
	{
		return m_Widget;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the widget is visible, false otherwise
	bool IsVisible()
	{
		return m_Widget && m_Widget.IsVisible();
	}

	//------------------------------------------------------------------------------------------------
	//! Set visibility of the widget.
	//! \param[in] visible True if visible
	void SetVisible(bool visible)
	{
		m_Widget.SetVisible(visible);
		m_Widget.SetEnabled(visible);
	}

//	//------------------------------------------------------------------------------------------------
//	//! Get faction of editable entity.
//	//! \param[in] owner Entity which will be checked (by default the entity the widget represents)
//	//! \return Faction
//	Faction GetFaction(GenericEntity owner = null)
//	{
//		if (!owner && m_Entity)
//			owner = m_Entity.GetOwner();
//
//		if (!owner)
//			return null;
//
//		FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));
//		if (!factionComponent)
//			return null;
//
//		return factionComponent.GetAffiliatedFaction();
//	}

	//------------------------------------------------------------------------------------------------
	//! Check if the icon is supposed to be shown on screen borders when its position is off-screen.
	//! \return true when marked to show off-screen
	bool IsShownOffScreen()
	{
		return m_bShowOffScreen;// || m_Entity.HasEntityState(EEditableEntityState.CURRENT_LAYER_CHILDREN);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Widget = w;
		m_Workspace = GetGame().GetWorkspace();
		m_fFade = 0;
	}
}
