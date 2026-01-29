[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_PingEditorComponentClass: SCR_BaseEditorComponentClass
{
	[Attribute(category: "Effects: Send Ping")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsSendPingFromPlayer;

	[Attribute(category: "Effects: Send Ping")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsSendPingFromEditor;
	
	[Attribute(category: "Effects: Send Ping")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsSendPingUnlimitedOnlyFromPlayer;
	
	[Attribute(category: "Effects: Send Ping")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsSendPingUnlimitedOnlyFromEditor;
	
	[Attribute(category: "Effects: Receive Ping")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsReceivePingFromPlayer;
	
	[Attribute(category: "Effects: Receive Ping")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsReceivePingFromEditor;
	
	[Attribute(category: "Effects: Receive Ping")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsReceivePingUnlimitedOnlyFromPlayer;
	
	[Attribute(category: "Effects: Receive Ping")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsReceivePingUnlimitedOnlyFromEditor;
	
	void ActivateEffects(SCR_PingEditorComponent component, bool isReceiver, int reporterID, bool reporterInEditor, bool unlimitedOnly, vector position, set<SCR_EditableEntityComponent> targets)
	{
		if (isReceiver)
		{
			if (unlimitedOnly)
			{
				if (reporterInEditor)
					SCR_BaseEditorEffect.Activate(m_EffectsReceivePingUnlimitedOnlyFromEditor, component, position, targets);
				else
					SCR_BaseEditorEffect.Activate(m_EffectsReceivePingUnlimitedOnlyFromPlayer, component, position, targets);
			}
			else
			{
				if (reporterInEditor)
					SCR_BaseEditorEffect.Activate(m_EffectsReceivePingFromEditor, component, position, targets);
				else
					SCR_BaseEditorEffect.Activate(m_EffectsReceivePingFromPlayer, component, position, targets);
			}
		}
		else
		{
			if (unlimitedOnly)
			{
				if (reporterInEditor)
					SCR_BaseEditorEffect.Activate(m_EffectsSendPingUnlimitedOnlyFromEditor, component, position, targets);
				else
					SCR_BaseEditorEffect.Activate(m_EffectsSendPingUnlimitedOnlyFromPlayer, component, position, targets);
			}
			else
			{
				if (reporterInEditor)
					SCR_BaseEditorEffect.Activate(m_EffectsSendPingFromEditor, component, position, targets);
				else
					SCR_BaseEditorEffect.Activate(m_EffectsSendPingFromPlayer, component, position, targets);
			}
		}
	}
};

/** @ingroup Editor_Components
Player <> Game Master communication.
*/
class SCR_PingEditorComponent : SCR_BaseEditorComponent
{
	[Attribute(defvalue: "10", desc: "How long (in seconds) a ping entity exists before being deleted. Fading is done speratly from this value in SCR_PingEffectsEditorUIComponent.")]
	protected float m_fPingEntityLifetime;
	
	[Attribute(defvalue: "0.75", desc: "How long (in seconds) is the ping on cooldown to prevent spamming.", params: "0, inf, 0.05")]
	protected float m_fCooldownTime;
	
	//~ Time left for cooldown
	protected float m_fCurrentCooldownTime;
	
	//~ In mili seconds what is the update freq for the cooldown update. If changed updated the step param for m_fCooldownTime (m_fCooldownUpdateFreq / 1000)
	protected float m_fCooldownUpdateFreq = 50; 
	
	protected bool m_bIsOnCooldown; 
	
	protected SCR_EditorManagerCore m_Core;
	protected ref map<int, SCR_EditableEntityComponent> m_PlayerPings = new map<int, SCR_EditableEntityComponent>;
	protected SCR_EditableEntityComponent m_LastPingEntity;
	
	protected ref ScriptInvoker Event_OnPingSend = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnPingReceive = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnPingEntityRegister = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnPingEntityUnregister = new ScriptInvoker;
	
	/*!
	Send ping (If not on cooldown).
	\param True if the ping is unlimited only
	\param position Pinged position
	\param target Pinged target
	\return Script invoker
	*/
	void SendPing(bool unlimitedOnly = false, vector position = vector.Zero, SCR_EditableEntityComponent target = null)
	{
		//~ Check if on cooldown. If true send notification informing player
		if (IsPingOnCooldown())
		{
			SCR_NotificationsComponent.SendLocal(ENotification.ACTION_ON_COOLDOWN, m_fCurrentCooldownTime * 100);
			return;
		}
			
		SCR_EditorManagerEntity manager = GetManager();
		if (!manager) 
			return;
		
		//~ If Unlimited only ping
		if (unlimitedOnly)
		{
			//~ Limited editor so cannot send GM only ping
			if (manager.IsLimited())
			{
				SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_GM_ONLY_PING_LIMITED_RIGHTS);
				return;
			}
			//~ Never send editor only ping if editor is not opened
			else if (!manager.IsOpened())
			{
				return;
			}
		}
			
		//~ If no GM do not send out ping
		SCR_PlayerDelegateEditorComponent playerDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
		if (playerDelegateManager && !playerDelegateManager.HasPlayerWithUnlimitedEditor())
		{
			SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PING_NO_GM_TO_PING);
			return;
		}
		
		int reporterID = manager.GetPlayerID();
		bool reporterInEditor = manager.IsOpened() && !manager.IsLimited();
		
		if (target) target.GetPos(position);
		CallEvents(manager, false, reporterID, reporterInEditor, unlimitedOnly, position, target);

		//--- Send the ping to server
		Rpc(SendPingServer, unlimitedOnly, position, Replication.FindId(target));
		
		//~ Ping cooldown to prevent spamming
		ActivateCooldown();
	}
	
	//~ Start Cooldown
	protected void ActivateCooldown()
	{
		m_fCurrentCooldownTime = m_fCooldownTime;
		m_bIsOnCooldown = true;
		
		//~ Add to callqueue to remove cooldown
		GetGame().GetCallqueue().CallLater(UpdateCooldown, m_fCooldownUpdateFreq, true);
	}
	
	//~ Update current cooldown
	protected void UpdateCooldown()
	{
		m_fCurrentCooldownTime -= m_fCooldownUpdateFreq / 1000; 
		
		if (m_fCurrentCooldownTime <= 0)
			OnCooldownDone();
	}
	
	//~ Executed after delay if Cooldown is done
	protected void OnCooldownDone()
	{
		m_bIsOnCooldown = false;
		GetGame().GetCallqueue().Remove(UpdateCooldown);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void SendPingServer(bool unlimitedOnly, vector position, RplId targetID)
	{
		if (!m_Core) return;
		
		SCR_EditorManagerEntity manager = GetManager();
		if (!manager) return;

		int reporterID = manager.GetPlayerID();
		bool reporterInEditor = manager.IsOpened() && !manager.IsLimited();

		//--- Exit when player's entity doesn't exist
		SCR_EditableEntityComponent reporterEntity = SCR_EditableEntityComponent.GetEditableEntity(GetGame().GetPlayerManager().GetPlayerControlledEntity(reporterID));
		//if (!reporterEntity) return;
		
		//--- Send the ping to editor managers of all players
		m_Core.Event_OnEditorManagerPing.Invoke(reporterID, reporterInEditor, reporterEntity, unlimitedOnly, position, targetID);
	}
	protected void ReceivePing(int reporterID, bool reporterInEditor, SCR_EditableEntityComponent reporterEntity, bool unlimitedOnly, vector position, RplId targetID)
	{
		//--- Don't ping yourself
		SCR_EditorManagerEntity manager = GetManager();
		if (!manager || manager.GetPlayerID() == reporterID)
			return;
		
		if (unlimitedOnly && manager.IsLimited())
			return;

		//--- Extra conditions when the reporter is not in unlimited editor (i.e., is not GM who can ping everyone)
		if (!reporterInEditor)
		{
			//--- Ignore when receiver is not in unlimited editor (i.e., no player to player ping allowed, or pinging limited photo mode)
			if (!manager.IsOpened() || manager.IsLimited())
				return;
			
			//--- Ignore when reporter's entity is not accessible from receiver's editor
			SCR_AccessKeysEditorComponent accessKeysManager = SCR_AccessKeysEditorComponent.Cast(manager.FindComponent(SCR_AccessKeysEditorComponent));
			if (accessKeysManager && reporterEntity && !reporterEntity.HasAccessInHierarchy(accessKeysManager.GetAccessKey()))
				return;
		}
		
		//--- Send the ping to the editor user
		Rpc(ReceivePingOwner, reporterID, reporterInEditor, unlimitedOnly, position, targetID);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void ReceivePingOwner(int reporterID, bool reporterInEditor, bool unlimitedOnly, vector position, RplId targetID)
	{
		SCR_EditorManagerEntity manager = GetManager();
		if (!manager) return;
		
		if (unlimitedOnly && manager.IsLimited())
			return;
				
		SCR_EditableEntityComponent target = SCR_EditableEntityComponent.Cast(Replication.FindItem(targetID));
		CallEvents(manager, false, reporterID, reporterInEditor, unlimitedOnly, position, target);
	}

	/*!
	Get the list of current ping entities.
	\param[out] outPlayerPings List of player IDs and ping entities
	\return Number of pings
	*/
	int GetPlayerPings(out notnull map<int, SCR_EditableEntityComponent> outPlayerPings)
	{
		return outPlayerPings.Copy(m_PlayerPings);
	}
	
	/*!
	Get event called when a ping is sent.
	Called only on the machine of whoever sent the ping.
	\return Script invoker
	*/
	ScriptInvoker GetOnPingSend()
	{
		return Event_OnPingSend;
	}
	/*!
	Get event called when a ping is received.
	Called on machine of every player who has editor opened, apart from the one who sent the ping.
	\return Script invoker
	*/
	ScriptInvoker GetOnPingReceive()
	{
		return Event_OnPingReceive;
	}
	/*!
	Get event called when a ping entity is created.
	Called on editor owner.
	\return Script invoker
	*/
	ScriptInvoker GetOnPingEntityRegister()
	{
		return Event_OnPingEntityRegister;
	}
	/*!
	Get event called when a ping entity is removed.
	Called on editor owner.
	\return Script invoker
	*/
	ScriptInvoker GetOnPingEntityUnregister()
	{
		return Event_OnPingEntityUnregister;
	}
	
	/*!
	Get if ping is on cooldown (Locally)
	\return True if the ping is on cooldown and cannot be excecuted
	*/
	bool IsPingOnCooldown()
	{
		return m_bIsOnCooldown;
	}
	
	protected void CallEvents(SCR_EditorManagerEntity manager, bool isReceiver, int reporterID, bool reporterInEditor, bool unlimitedOnly, vector position, SCR_EditableEntityComponent target)
	{
		set<SCR_EditableEntityComponent> targets = new set<SCR_EditableEntityComponent>;
		if (target) targets.Insert(target);
		
		m_LastPingEntity = target;
		
		//--- Invoke handlers
		if (isReceiver)
			Event_OnPingReceive.Invoke(reporterID, reporterInEditor, unlimitedOnly, position, target);
		else
			Event_OnPingSend.Invoke(reporterID, reporterInEditor, unlimitedOnly, position, target);

		//--- Call effects defined in prefab
		SCR_PingEditorComponentClass prefabData = SCR_PingEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData) prefabData.ActivateEffects(this, isReceiver, reporterID, reporterInEditor, unlimitedOnly, position, targets);
		
		if (m_LastPingEntity)
		{
			SCR_EditableEntityComponent prevPingEntity;
			if (m_PlayerPings.Find(reporterID, prevPingEntity))
				Expire(reporterID, prevPingEntity);

			m_PlayerPings.Set(reporterID, m_LastPingEntity);
			Event_OnPingEntityRegister.Invoke(reporterID, m_LastPingEntity);
			
			GetGame().GetCallqueue().CallLater(Expire, m_fPingEntityLifetime * 1000, false, reporterID, m_LastPingEntity);
		}
		
		SCR_NotificationData newNotificationData;
		int targetPlayerID = 0;
		int targetID
		
		//Check if has target
		if (target)
		{
			targetID = Replication.FindId(target);
			targetPlayerID = SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(target.GetOwner());
		}
	
		//Send out notification if not unlimitedOnly
		if (!unlimitedOnly)
		{
			//If pinger has editor rights
			if (reporterInEditor)
			{
				//No target
				if (!target || target.HasEntityFlag(EEditableEntityFlag.LOCAL))
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PING_GM, position, reporterID);
				//Player target
				else if (targetPlayerID > 0)
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PING_GM_TARGET_PLAYER, reporterID, targetPlayerID);
				//EditableEntity target
				else 
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PING_GM_TARGET_ENTITY, reporterID, targetID);
			}
			else {
				//No target
				if (!target || target.HasEntityFlag(EEditableEntityFlag.LOCAL))
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PING_PLAYER, position, reporterID);
				//Player target
				else if (targetPlayerID > 0)
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PING_PLAYER_TARGET_PLAYER, reporterID, targetPlayerID);
				//EditableEntity target
				else 
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PING_PLAYER_TARGET_ENTITY, reporterID, targetID);
			}
		}
		else 
		{
			//No target
			if (!target || target.HasEntityFlag(EEditableEntityFlag.LOCAL))
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.EDITOR_GM_ONLY_PING, position, reporterID);
			//Player target
			else if (targetPlayerID > 0)
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.EDITOR_GM_ONLY_PING_TARGET_PLAYER, reporterID, targetPlayerID);
			//EditableEntity target
			else 
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.EDITOR_GM_ONLY_PING_TARGET_ENTITY, reporterID, targetID);
		}
	}
	
	protected void Expire(int reporterID, SCR_EditableEntityComponent pingEntity)
	{
		SCR_EditableEntityComponent currentPingEntity;
		if (pingEntity && m_PlayerPings.Find(reporterID, currentPingEntity) && pingEntity == currentPingEntity)
		{
			m_PlayerPings.Remove(reporterID);
			Event_OnPingEntityUnregister.Invoke(reporterID, pingEntity);
			
			if (pingEntity.GetOwner() && pingEntity.GetOwner().IsInherited(SCR_EditorPingEntity))
				pingEntity.Delete();
		}
	}
	override void EOnEffect(SCR_BaseEditorEffect effect)
	{
		SCR_EntityEditorEffect entityEffect = SCR_EntityEditorEffect.Cast(effect);
		if (entityEffect) m_LastPingEntity = SCR_EditableEntityComponent.GetEditableEntity(entityEffect.GetEntity());
	}

	//--- Server init
	void SCR_PingEditorComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!Replication.IsServer()) return;
		
		m_Core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (m_Core)
			m_Core.Event_OnEditorManagerPing.Insert(ReceivePing);
	}
	void ~SCR_PingEditorComponent()
	{
		if (m_bIsOnCooldown)
			OnCooldownDone();
		
		if (m_Core) m_Core.Event_OnEditorManagerPing.Remove(ReceivePing);
	}
};