class SCR_CallsignBaseComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_CallsignBaseComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_CallsignBaseComponent callsign = SCR_CallsignBaseComponent.Cast(component);

		auto group = SCR_AIGroup.Cast(owner);

		int companyIndex, platoonIndex, squadIndex, characterNumber;
		ERoleCallsign characterRole;

		if ((group && group.GetAgentsCount() == 0) || !callsign.GetCallsignIndexes(companyIndex, platoonIndex, squadIndex, characterNumber, characterRole))
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(companyIndex, -1);
		context.WriteDefault(platoonIndex, -1);
		context.WriteDefault(squadIndex, -1);

		/*
		if (!group)
		{
			context.WriteDefault(characterNumber, -1);
			context.WriteDefault(characterRole, ERoleCallsign.NONE);
		}
		*/

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_CallsignBaseComponent callsign = SCR_CallsignBaseComponent.Cast(component);

		int version;
		context.ReadValue("version", version);

		int companyIndex, platoonIndex, squadIndex;
		context.ReadDefault(companyIndex, -1);
		context.ReadDefault(platoonIndex, -1);
		context.ReadDefault(squadIndex, -1);

		auto group = SCR_AIGroup.Cast(owner);
		if (group)
		{
			auto callsignManager = SCR_CallsignManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_CallsignManagerComponent));
			callsignManager.RemoveAvailableGroupCallsign(group.GetFaction(), companyIndex, platoonIndex, squadIndex);

			auto groupCallsign = SCR_CallsignGroupComponent.Cast(callsign);
			if (!groupCallsign)
				return false;

			groupCallsign.DoAssignCallsign(companyIndex, platoonIndex, squadIndex);
		}
		
		// TODO(@langepau): If desired to remember individual callsigns we need to make sure default group inits etc do not run after and reset it again :)
		// Also we might want to store member callsigns on the group data still as callsigns make no sense on individual chars without groups?
		/*
		else
		{
			int characterNumber;
			context.ReadDefault(characterNumber, -1);

			ERoleCallsign characterRole;
			context.ReadDefault(characterRole, ERoleCallsign.NONE);

			auto factionAffiliation = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));
			if (!factionAffiliation)
				return false;

			auto charCallsign = SCR_CallsignCharacterComponent.Cast(callsign);
			if (!charCallsign)
				return false;

			charCallsign.AssignCharacterCallsign(factionAffiliation.GetAffiliatedFaction(), companyIndex, platoonIndex, squadIndex, characterNumber, characterRole, charCallsign.GetIsCharacterAloneInGroup());
		}
		*/

		return true;
	}
}
