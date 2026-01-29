[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_PlayerTeleportedFeedbackComponentClass : ScriptComponentClass
{
}

class SCR_PlayerTeleportedFeedbackComponent : ScriptComponent
{
	protected ref ScriptInvoker m_OnPlayerTeleported = new ScriptInvoker(); //param int GM that teleported the player
	protected PlayerManager m_PlayerManager;
	
	//------------------------------------------------------------------------------------------------
	//! Get on player Teleported script invoker.
	ScriptInvoker GetOnPlayerTeleported()
	{
		return m_OnPlayerTeleported;
	}
	
	//------------------------------------------------------------------------------------------------
	//! On player teleported send a notification and event that the local player character is teleported
	void PlayerTeleported(IEntity character, bool isLongFade, SCR_EPlayerTeleportedReason teleportReason)
	{
		//~ Do not show teleport feedback if no character
		if (!character)
			return;
		
		//~ Do not show teleport feedback if dead
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(character);
		if (damageManager && damageManager.GetState() == EDamageState.DESTROYED)
			return;
		
		bool editorIsOpen = SCR_EditorManagerEntity.IsOpenedInstance();
		
		m_OnPlayerTeleported.Invoke(editorIsOpen, isLongFade, teleportReason);
		
		//~ Player teleported by GM
		if (teleportReason == SCR_EPlayerTeleportedReason.EDITOR)
		{			
			//~ Only show notification if editor is closed
			if (!editorIsOpen)
				SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_GM_TELEPORTED_PLAYER);	
		}
		//~ Player teleported as blocking spawner
		else if (teleportReason == SCR_EPlayerTeleportedReason.BLOCKING_SPAWNER)
		{
			SCR_NotificationsComponent.SendLocal(ENotification.TELEPORTED_PLAYER_BLOCKING_SPAWNER);
		}
	}
}

enum SCR_EPlayerTeleportedReason
{
	DEFAULT,
	EDITOR,
	BLOCKING_SPAWNER,
	FAST_TRAVEL
}
