/*!
Used to hide Entities that are not editable entities but should still listen to the Editor functionality
*/
[ComponentEditorProps(category: "GameScripted/Editor", description: "")]
class SCR_ShowHideInEditorComponentClass : ScriptComponentClass
{
};

class SCR_ShowHideInEditorComponent : ScriptComponent
{
	[Attribute("1", desc: "Set when the entity is visible. If false will hide itself in editor and show outside of editor. This component is ignored if attached to an EditableEntity or if parent is editable entity", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EShowHideInEditor))]
	protected EShowHideInEditor m_eShowHideInEditor;
	
	[Attribute("0", desc: "If true will also Show/Hide children when entity is hidden")]
	protected bool m_bShowHideChildren;
	
	[Attribute("1", desc: "If true will also Show/Hide when editor UI is hidden")]
	protected bool m_bHideWhenEditorUiHidden;
	
	//~  Always true if m_bHideWhenEditorUiHidden is false
	protected bool m_bEditorMenuVisible = true;
	
	//~ If true make sure to remove the on editor create scriptinvoker Subscription
	protected bool m_bSubscribedToOnEditorCreate;
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorOpen()
	{
		ShowHideEntity(true, SCR_EditorModeEntity.GetInstance());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorClosed()
	{
		ShowHideEntity(false, SCR_EditorModeEntity.GetInstance());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorModeChanged(SCR_EditorModeEntity currentMode, SCR_EditorModeEntity prevMode)
	{
		ShowHideEntity(SCR_EditorManagerEntity.IsOpenedInstance(), currentMode);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorMenuVisibilityChanged(bool isVisible)
	{
		m_bEditorMenuVisible = isVisible;
		
		if (!m_bEditorMenuVisible)
		{
			GetOwner().ClearFlags(EntityFlags.VISIBLE, m_bShowHideChildren);
		}
		else 
		{
			SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();	
			if (!editorManagerEntity)
			{
				GetOwner().ClearFlags(EntityFlags.VISIBLE, m_bShowHideChildren);
				return;
			}
			
			ShowHideEntity(editorManagerEntity.IsOpened(), SCR_EditorModeEntity.GetInstance());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowHideEntity(bool editorIsOpen, SCR_EditorModeEntity mode)
	{
		//~ UI not visible so no need to set visible again
		if (!m_bEditorMenuVisible || !editorIsOpen)
		{
			GetOwner().ClearFlags(EntityFlags.VISIBLE, m_bShowHideChildren);
			return;
		}
		
		bool isLimited = !mode || mode.IsLimited();
		
		if (isLimited)
		{
			if (SCR_Enum.HasFlag(m_eShowHideInEditor, EShowHideInEditor.SHOW_IN_LIMITED_EDITOR))
				GetOwner().SetFlags(EntityFlags.VISIBLE, m_bShowHideChildren);
			else 
				GetOwner().ClearFlags(EntityFlags.VISIBLE, m_bShowHideChildren);
		}
		else 
		{
			if (SCR_Enum.HasFlag(m_eShowHideInEditor, EShowHideInEditor.SHOW_IN_UNLIMITED_EDITOR))
				GetOwner().SetFlags(EntityFlags.VISIBLE, m_bShowHideChildren);
			else 
				GetOwner().ClearFlags(EntityFlags.VISIBLE, m_bShowHideChildren);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		//~ Use editor logic if attached to editable entity
		if (owner.FindComponent(SCR_EditableEntityComponent))
			return;
		
		//~ Use editor logic if attached to parent that is an editable entity
		IEntity parent = owner.GetParent();
		if (parent && parent.FindComponent(SCR_EditableEntityComponent))
			return;
		
		//~  Set invisible on init
		GetOwner().ClearFlags(EntityFlags.VISIBLE, m_bShowHideChildren);
		
		if (m_eShowHideInEditor <= 0)
			return;
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			Init(editorManager);
			return;
		}
		
		//~ No editor manager found wait until it is created
		SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (editorManagerCore)
			editorManagerCore.Event_OnEditorManagerInitOwner.Insert(Init);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Init(SCR_EditorManagerEntity editorManagerEntity)
	{	
		if (m_bSubscribedToOnEditorCreate)
		{
			SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (editorManagerCore)
			{
				m_bSubscribedToOnEditorCreate = false;
				editorManagerCore.Event_OnEditorManagerInitOwner.Remove(Init);
			}
		}
		
		editorManagerEntity.GetOnOpened().Insert(OnEditorOpen);
		editorManagerEntity.GetOnClosed().Insert(OnEditorClosed);
		editorManagerEntity.GetOnModeChange().Insert(OnEditorModeChanged);
		
		if (m_bHideWhenEditorUiHidden)
		{
			SCR_MenuEditorComponent menuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
			if (menuManager)
			{
				menuManager.GetOnVisibilityChange().Insert(OnEditorMenuVisibilityChanged);
				
				m_bEditorMenuVisible = menuManager.IsVisible();
				if (!m_bEditorMenuVisible)
					GetOwner().ClearFlags(EntityFlags.VISIBLE, m_bShowHideChildren);
			}
		}
		
		if (editorManagerEntity.IsOpened())
			OnEditorOpen();
		else 
			OnEditorClosed();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{		
		//~  Hide in workbench
		if (SCR_Global.IsEditMode())
			return;
		
		//~ Use editor logic if attached to editable entity
		if (owner.FindComponent(SCR_EditableEntityComponent))
			return;
		
		//~ Use editor logic if attached to parent that is an editable entity
		IEntity parent = owner.GetParent();
		if (parent && parent.FindComponent(SCR_EditableEntityComponent))
			return;
			
		if (m_eShowHideInEditor <= 0)
			return;
		
		//~ Remove on editor creation event
		if (m_bSubscribedToOnEditorCreate)
		{
			SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (editorManagerCore)
				editorManagerCore.Event_OnEditorManagerInitOwner.Remove(Init);
		}
		
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();	
		if (!editorManagerEntity)
			return;
		
		editorManagerEntity.GetOnOpened().Remove(OnEditorOpen);
		editorManagerEntity.GetOnClosed().Remove(OnEditorClosed);
		editorManagerEntity.GetOnModeChange().Remove(OnEditorModeChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		//~  Hide in workbench
		if (SCR_Global.IsEditMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}
};

//------------------------------------------------------------------------------------------------
enum EShowHideInEditor
{
	SHOW_IN_UNLIMITED_EDITOR = 1,
	SHOW_IN_LIMITED_EDITOR = 2,
};