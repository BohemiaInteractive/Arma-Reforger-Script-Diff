[ComponentEditorProps(category: "GameScripted/Editor", description: "Manager of editor context actions", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_ContextActionsEditorComponentClass: SCR_BaseActionsEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
/*!
Component to hold all context actions for each editor mode and filter out actions depending on context

Intended use is for this component to be added to each editor mode entity which should have specific actions available

These actions are defined on the editor mode prefab entity as types of SCR_BaseContextAction
*/
class SCR_ContextActionsEditorComponent : SCR_BaseActionsEditorComponent
{	
	protected ref ScriptInvoker m_OnMenuOpen = new ScriptInvoker();
	protected SCR_LayersEditorComponent m_LayerManager;
	protected SCR_StatesEditorComponent m_StateManager;
	
	/*!
	Get event called for the user when the menu is opened.
	\return Script invoker
	*/
	ScriptInvoker GetOnMenuOpen()
	{
		return m_OnMenuOpen;
	}
	
	override protected int ValidateSelection(bool isInstant)
	{
		//--- Evaluate selected/hovered-on entities only when selecting, not when placing or transforming
		if (!m_StateManager || m_StateManager.GetState() == EEditorState.SELECTING)
		{

			if (m_HoverManager)
			{
				m_HoveredEntity = m_HoverManager.GetFirstEntity();
				
				//--- If the entity is inside a composition, make the composition the hovered entity
				if (m_LayerManager)
					m_HoveredEntity = m_LayerManager.GetParentBelowCurrentLayer(m_HoveredEntity);
	
				//--- Update selection
				if (m_SelectedManager)
				{
					if (m_HoveredEntity)
					{
						//--- Open menu over entity outside of the current selection - select it instead
						if (!m_SelectedManager.Contains(m_HoveredEntity))
							m_SelectedManager.Replace(m_HoveredEntity);
					}
					else if (!isInstant)
					{
						//--- Opened menu without any entity under cursor - clear the selection
						//--- (not when the action was activated instantly, i.e., by a shortcut; it would prevent. e.g., deleting selected entity when cursor is on empty space)
						m_SelectedManager.Clear();
					}
				}
			}
			else
			{
				m_HoveredEntity = null;
			}
			
			//--- Clear previously cached entities
			m_SelectedEntities.Clear();
			
			//--- Get selected entities
			if (m_SelectedManager)
				m_SelectedManager.GetEntities(m_SelectedEntities);
			
			//--- No entities selected, use the entity under cursor (needed for example when deleting composition while hovering over one of its child entities)
			if (m_HoveredEntity && m_SelectedEntities.IsEmpty())
				m_SelectedEntities.Insert(m_HoveredEntity);
		}
		else
		{
			m_HoveredEntity = null;
			m_SelectedEntities.Clear();
		}
		
		//--- Cache flags
		EEditorContextActionFlags flags;
		if (GetManager().IsLimited())
		{
			flags |= EEditorContextActionFlags.LIMITED;
		}
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
		{
			flags |= EEditorContextActionFlags.USING_MOUSE_AND_KEYBOARD;
		}
		return flags;
	}
	override void EvaluateActions(notnull array<SCR_BaseEditorAction> actions, vector cursorWorldPosition, out notnull array<ref SCR_EditorActionData> filteredActions, out int flags = 0)
	{
		super.EvaluateActions(actions, cursorWorldPosition, filteredActions, flags);
		m_OnMenuOpen.Invoke(actions, cursorWorldPosition, filteredActions, flags);
	}
	
	override protected bool ActionCanBeShown(SCR_BaseEditorAction action, vector cursorWorldPosition, int flags)
	{
		return action.GetInfo() && action.CanBeShown(m_HoveredEntity, m_SelectedEntities, cursorWorldPosition, flags);
	}
	
	override protected bool ActionCanBePerformed(SCR_BaseEditorAction action, vector cursorWorldPosition, int flags)
	{
		return action.CanBePerformed(m_HoveredEntity, m_SelectedEntities, cursorWorldPosition, flags);
	}
	
	override void EOnEditorActivate()
	{
		super.EOnEditorActivate();
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_EDITOR_ACTIONS_MENU, "Actions Menu", "Editor");
		
		m_LayerManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		m_StateManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
	}
	
	override void EOnEditorDeactivate()
	{
		super.EOnEditorDeactivate();
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ACTIONS_MENU);
	}
};
