class SCR_AISendMessageDebugUIClass : GenericEntityClass
{
	
};

/*!
This debug menu lets us send various messages between selected AIs.
*/

class SCR_AISendMessageDebugUI : GenericEntity
{
	static SCR_AISendMessageDebugUI s_Instance;
	
	//-------------------------------------------------------------------------------------------
	static void Init()
	{
		if (!s_Instance)
			s_Instance = SCR_AISendMessageDebugUI.Cast(GetGame().SpawnEntity(SCR_AISendMessageDebugUI));
	}
	
	
	//-------------------------------------------------------------------------------------------
	void SCR_AISendMessageDebugUI(IEntitySource src, IEntity parent)
	{
		SetFlags(EntityFlags.ACTIVE, true);
		SetEventMask(EntityEvent.FRAME);
	}
	
	
	//-------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SEND_MESSAGE))
			s_Instance.Draw();
	}
	
	
	//-------------------------------------------------------------------------------------------
	protected void Draw()
	{
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld)
			return;
		
		// Init list with string names of info messages
		/*
		TStringArray InfoMsgNames = {};
		TIntArray infoMsgEnumValues = {};
		typename msgTypeInfoTypename = EMessageType_Info;
		for (int i = 0; i < msgTypeInfoTypename.GetVariableCount(); i++)
		{
			int _enumValue;
			msgTypeInfoTypename.GetVariableValue(null, i, _enumValue);
			
			// Check if this message is supposed to be used in this Debug UI
			SCR_AIMessageBase msg = aiWorld.GetInfoMessageOfType(_enumValue);
			
			if (!msg)
				continue;
						
			if (!msg.DebugUi_IsVisible())
				continue;
			
			infoMsgEnumValues.Insert(_enumValue);
			
			string _s = typename.EnumToString(EMessageType_Info, _enumValue);
			InfoMsgNames.Insert(_s);
		}
		*/		
		
		DbgUI.Begin("Send Info Message");
		
		
		// Text: Selected AI
		IEntity ent = GetSelectedAiEntity();
		DbgUI.Text(string.Format("Selected Entity: %1", ent.ToString()));
		
		if (ent)
		{
			AIControlComponent controlComp = AIControlComponent.Cast(ent.FindComponent(AIControlComponent));
			AIAgent senderAgent = controlComp.GetAIAgent();
			
			const int buttonWidth = 500;
			if (DbgUI.Button("INFO: NO AMMO -> GROUP", buttonWidth))
				SendNoAmmoToGroup(ent, senderAgent);
		}
		else
		{
			DbgUI.Text("You must select an AI Entity with Game Master first!");
		}
		
		DbgUI.End();
	}
	
	
	//-------------------------------------------------------------------------------------------
	protected SCR_AIMessageBase CreateInfoMessage(EMessageType_Info msgType, IEntity senderEnt)
	{	
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		AIControlComponent controlComp = AIControlComponent.Cast(senderEnt.FindComponent(AIControlComponent));
		AIAgent senderAgent = controlComp.GetAIAgent();
		AICommunicationComponent mailbox = senderAgent.GetCommunicationComponent();
		
		SCR_AIMessageBase msg; // Disabled for now, it's not useful
		
		return msg;	
	}
	
	
	//-------------------------------------------------------------------------------------------
	protected void BroadcastMessage(AIAgent senderAgent, SCR_AIMessageBase msg, AIAgent receiver)
	{
		AICommunicationComponent mailbox = senderAgent.GetCommunicationComponent();
		
		mailbox.RequestBroadcast(msg, receiver);
	}
	
	
	//-------------------------------------------------------------------------------------------
	protected IEntity GetSelectedAiEntity()
	{
		// Get first AI entity selected in Game Master
		SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
		set<SCR_EditableEntityComponent> selectedEntities = new set<SCR_EditableEntityComponent>();
		filter.GetEntities(selectedEntities);
		
		if (selectedEntities.Count() != 1)
			return null;

		SCR_EditableCharacterComponent editCharacterComp = SCR_EditableCharacterComponent.Cast(selectedEntities[0]);
				
		if (!editCharacterComp)
			return null;
		
		return editCharacterComp.GetAgent().GetControlledEntity();
	}
	
	
	
	
	//-------------------------------------------------------------------------------------------
	protected void SendNoAmmoToGroup(IEntity senderEnt, AIAgent senderAgent)
	{
		SCR_AIMessage_NoAmmo msg = SCR_AIMessage_NoAmmo.Cast(CreateInfoMessage(EMessageType_Info.NO_AMMO, senderEnt));
		
		msg.m_entityToSupply = senderEnt;
		
		// Get current mag well		
		auto weaponManager = BaseWeaponManagerComponent.Cast(senderEnt.FindComponent(BaseWeaponManagerComponent));
		BaseWeaponComponent weapon = weaponManager.GetCurrent();
		msg.m_MagazineWell = SCR_AIGetMagazineWell.GetWeaponMainMagazineWell(weapon);
		Print(string.Format("SendNoAmmoToGroup: %1, %2", senderEnt.ToString(), msg.m_MagazineWell.ToString()), LogLevel.DEBUG);
		
		AIAgent receiver = senderAgent.GetParentGroup();
		BroadcastMessage(senderAgent, msg, receiver);
	}
};