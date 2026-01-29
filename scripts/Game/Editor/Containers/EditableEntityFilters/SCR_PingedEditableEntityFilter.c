[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_PingedEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	protected ref map<int, SCR_EditableEntityComponent> m_PlayerPings = new map<int, SCR_EditableEntityComponent>;
	
	protected void OnPingEntityRegister(int reporterID, SCR_EditableEntityComponent pingEntity)
	{
		Add(pingEntity, true);
	}
	protected void OnPingEntityUnregister(int reporterID, SCR_EditableEntityComponent pingEntity)
	{
		Remove(pingEntity, true);
	}

	override void EOnEditorActivate()
	{
		SCR_PingEditorComponent pingManager = SCR_PingEditorComponent.Cast(SCR_PingEditorComponent.GetInstance(SCR_PingEditorComponent, true));
		if (!pingManager) return;
		
		pingManager.GetOnPingEntityRegister().Insert(OnPingEntityRegister);
		pingManager.GetOnPingEntityUnregister().Insert(OnPingEntityUnregister);
		
		InitEntities(); //--- Initialize the list before starting to fill it
		
		map<int, SCR_EditableEntityComponent> playerPings = new map<int, SCR_EditableEntityComponent>;
		pingManager.GetPlayerPings(playerPings);
		foreach (int reporterID, SCR_EditableEntityComponent pingEntity: playerPings)
		{
			OnPingEntityRegister(reporterID, pingEntity);
		}
	}
	override void EOnEditorDeactivate()
	{
		SCR_PingEditorComponent pingManager = SCR_PingEditorComponent.Cast(SCR_PingEditorComponent.GetInstance(SCR_PingEditorComponent));
		if (!pingManager) return;
		
		pingManager.GetOnPingEntityRegister().Remove(OnPingEntityRegister);
		pingManager.GetOnPingEntityUnregister().Remove(OnPingEntityUnregister);
	}
};