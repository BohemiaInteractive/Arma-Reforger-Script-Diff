[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_PlayerModesEditorAttribute: SCR_BaseMultiSelectPresetsEditorAttribute
{	
	protected SCR_EditorManagerEntity GetEditorManager(Managed item)
	{				
		SCR_EditablePlayerDelegateComponent delegate = SCR_EditablePlayerDelegateComponent.Cast(item);
		if (!delegate) 
			return null;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;
		
		return core.GetEditorManager(delegate.GetPlayerID());
	}
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		EEditorMode modes;
	
		//--- Player modes, affects specific editor manager
		SCR_EditorManagerEntity editorManager = GetEditorManager(item);
		if (!editorManager) 
			return null;
		
		if (SCR_Global.IsAdmin(editorManager.GetPlayerID()))
			return null;
		
		modes = editorManager.GetModes();
		
		super.ReadVariable(item, manager);
		SetFlags(modes);
		return SCR_BaseEditorAttributeVar.CreateVector(GetFlagVector());
	}
	
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{		
		if (isInit)
		{
			manager.SetAttributeAsSubAttribute(SCR_PlayerBudgetPropEnableEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_PlayerBudgetVehicleEnableEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_PlayerBudgetAIEnableEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_PlayerBudgetSystemEnableEditorAttribute);
		}
		
		if (!var)
		{
			manager.SetAttributeEnabled(SCR_PlayerBudgetPropEnableEditorAttribute, false);
			manager.SetAttributeEnabled(SCR_PlayerBudgetVehicleEnableEditorAttribute, false);
			manager.SetAttributeEnabled(SCR_PlayerBudgetAIEnableEditorAttribute, false);
			manager.SetAttributeEnabled(SCR_PlayerBudgetSystemEnableEditorAttribute, false);
		}
		else 
		{
			int flags = var.GetVector()[0];
			bool enableBudget = flags & EEditorMode.EDIT;
			
			manager.SetAttributeEnabled(SCR_PlayerBudgetPropEnableEditorAttribute, enableBudget);
			manager.SetAttributeEnabled(SCR_PlayerBudgetVehicleEnableEditorAttribute, enableBudget);
			manager.SetAttributeEnabled(SCR_PlayerBudgetAIEnableEditorAttribute, enableBudget);
			manager.SetAttributeEnabled(SCR_PlayerBudgetSystemEnableEditorAttribute, enableBudget);
		}
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{		
		if (!var)
			return;
		
		super.WriteVariable(item, var, manager, playerID);
		
		//--- Player modes
		SCR_EditorManagerEntity editorManager = GetEditorManager(item);
		if (!editorManager)
			return;
		
		if (SCR_Global.IsAdmin(editorManager.GetPlayerID()))
			return;
		
		int flags = GetFlags(0);
		editorManager.SetEditorModes(EEditorModeAccess.BASE, flags, false);
	}
	
	protected override void CreatePresets()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core) return;
		
		m_aValues.Clear();	
		
		SCR_EditorModePrefab mode;
		SCR_SortedArray<SCR_EditorModePrefab> modePrefabs = new SCR_SortedArray<SCR_EditorModePrefab>;
		for (int i, count = core.GetBaseModePrefabs(modePrefabs, EEditorModeFlag.EDITABLE, true); i < count; i++)
		{
			mode = modePrefabs[i];
			SCR_EditorAttributeFloatStringValueHolder value = new SCR_EditorAttributeFloatStringValueHolder();
			value.SetWithUIInfo(mode.GetInfo(), mode.GetMode());
			m_aValues.Insert(value);
		}
	}
	
};