/** @ingroup Editor_UI
*/

/*!
Communication with game master

ToDo: Use some general GUD icon drawing?
*/
class SCR_EditorPingInfoDisplay : SCR_InfoDisplay
{
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_WidgetPrefab;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_SlotWidgetPrefab;
	
	protected bool m_bCanUpdate;
	protected WorkspaceWidget m_Workspace;
	protected SCR_PingEditorComponent m_PingManager;
	protected ref map<SCR_EditableEntityComponent, SCR_EditableEntitySceneSlotUIComponent> m_EntitySlots = new map<SCR_EditableEntityComponent, SCR_EditableEntitySceneSlotUIComponent>;
	protected ref map<SCR_EditableEntityComponent, Widget> m_EntityWidgets = new map<SCR_EditableEntityComponent, Widget>;
	
	protected void OnEditorPingAction(float value, EActionTrigger reason)
	{
		SendPing(false);
	}
	protected void OnEditorPingUnlimitedOnlyAction(float value, EActionTrigger reason)
	{
		SendPing(true);
	}
	protected void SendPing(bool unlimitedOnly)
	{
		if (SCR_EditorManagerEntity.IsOpenedInstance()) return;
		
		vector position;
		SCR_EditableEntityComponent target;
		
		if (!target || !target.GetPos(position))
		{
			//--- Position under cursor
			WorkspaceWidget workspace = GetGame().GetWorkspace();
			if (!workspace) return;
			
			//--- Not reliable, cursor is not hidden in center of the screen on client
			//int mouseX, mouseY;
			//WidgetManager.GetMousePos(mouseX, mouseY);
			
			int screenW = workspace.GetWidth();
			int screenH = workspace.GetHeight();
			
			BaseWorld world = GetGame().GetWorld();
			vector outDir;
			vector startPos = workspace.ProjScreenToWorld(workspace.DPIUnscale(screenW / 2), workspace.DPIUnscale(screenH / 2), outDir, world);
			outDir *= 1000; //--- ToDo: Don't hardcode
	
			autoptr TraceParam trace = new TraceParam();
			trace.Start = startPos;
			trace.End = startPos + outDir;
			trace.Flags = TraceFlags.WORLD | TraceFlags.OCEAN | TraceFlags.ENTS;
			trace.LayerMask = TRACE_LAYER_CAMERA;
			float traceDis = world.TraceMove(trace, null);
			
			target = SCR_EditableEntityComponent.GetEditableEntity(trace.TraceEnt);
			if (!target && traceDis == 1) return; //--- No intersection
			
			position = startPos + outDir * traceDis;
		}

		m_PingManager.SendPing(unlimitedOnly, position, target);
	}
	protected void OnPingEntityRegister(int reporterID, SCR_EditableEntityComponent pingEntity)
	{
		//--- When outside of the editor, don't create a ping on player
		if (pingEntity.GetOwner() == SCR_PlayerController.GetLocalControlledEntity()) return;
			
		SCR_EditableEntitySceneSlotUIComponent slot;
		if (!m_EntitySlots.Find(pingEntity, slot))
		{
			Widget slotWidget = m_Workspace.CreateWidgets(m_SlotWidgetPrefab, m_wRoot);
			if (!slotWidget) return;
			
			slot = SCR_EditableEntitySceneSlotUIComponent.Cast(slotWidget.FindHandler(SCR_EditableEntitySceneSlotUIComponent));
			if (!slot) return;
			
			slot.InitSlot(pingEntity);
			m_EntitySlots.Insert(pingEntity, slot);
		}
		
		//--- Create ping widget in a slot
		slot.DeleteWidget(null);
		Widget entityWidget = slot.CreateWidget(pingEntity, m_WidgetPrefab);
		m_EntityWidgets.Insert(pingEntity, entityWidget);
		m_bCanUpdate = true;
	}
	protected void OnPingEntityUnregister(int reporterID, SCR_EditableEntityComponent pingEntity)
	{
		Widget entityWidget;
		if (m_EntityWidgets.Find(pingEntity, entityWidget))
		{
			m_EntityWidgets.Remove(pingEntity);
		}
		
		SCR_EditableEntitySceneSlotUIComponent slot;
		if (m_EntitySlots.Find(pingEntity, slot))
		{
			if (slot)
			{
				slot.DeleteWidget(null);
				slot.DeleteSlot();
			}
			
			if (!slot || slot.IsEmpty())
			{
				m_EntitySlots.Remove(pingEntity);
				m_bCanUpdate = !m_EntitySlots.IsEmpty();
			}
		}
	}
	
	override event void UpdateValues(IEntity owner, float timeSlice)
	{
		if (!m_bCanUpdate || !m_Workspace) return;
		
		int screenW = m_Workspace.GetWidth();
		int screenH = m_Workspace.GetHeight();
		
		screenW = m_Workspace.DPIUnscale(screenW);
		screenH = m_Workspace.DPIUnscale(screenH);
		vector posCenter = Vector(screenW / 2, screenH / 2, 0);
		
		vector transformCam[4];
		GetGame().GetWorld().GetCurrentCamera(transformCam);
		vector posCam = transformCam[3];
		
		foreach (SCR_EditableEntitySceneSlotUIComponent entitySlot: m_EntitySlots)
		{
			entitySlot.UpdateSlot(screenW, screenH, posCenter, posCam);
		}
	}
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		
		if (!SCR_EditorManagerEntity.GetInstance())
		{
			//--- HUD initialized before the editor - wait
			SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (editorManagerCore) editorManagerCore.Event_OnEditorManagerInitOwner.Insert(OnStartDraw);
			return;
		}
		else
		{
			SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (editorManagerCore) editorManagerCore.Event_OnEditorManagerInitOwner.Remove(OnStartDraw);
		}
		
		m_PingManager = SCR_PingEditorComponent.Cast(SCR_PingEditorComponent.GetInstance(SCR_PingEditorComponent, true));
		if (!m_PingManager) return;
		
		m_PingManager.GetOnPingEntityRegister().Insert(OnPingEntityRegister);
		m_PingManager.GetOnPingEntityUnregister().Insert(OnPingEntityUnregister);

		m_Workspace = m_wRoot.GetWorkspace();
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.AddActionListener("EditorPing", EActionTrigger.DOWN, OnEditorPingAction);
			inputManager.AddActionListener("EditorPingUnlimitedOnly", EActionTrigger.DOWN, OnEditorPingUnlimitedOnlyAction);
		}
	}
	override event void OnStopDraw(IEntity owner)
	{
		super.OnStopDraw(owner);
		
		//--- Clear existing ping widgets
		for (int i = 0, count = m_EntitySlots.Count(); i < count; i++)
		{
			OnPingEntityUnregister(0, m_EntitySlots.GetKey(0));
		}
		
		if (m_PingManager)
		{
			m_PingManager.GetOnPingEntityRegister().Remove(OnPingEntityRegister);
			m_PingManager.GetOnPingEntityUnregister().Remove(OnPingEntityUnregister);
		}

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.RemoveActionListener("EditorPing", EActionTrigger.DOWN, OnEditorPingAction);
			inputManager.RemoveActionListener("EditorPingUnlimitedOnly", EActionTrigger.DOWN, OnEditorPingUnlimitedOnlyAction);
		}
	}
};