class SCR_CampaignMobileAssemblyComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_CampaignMobileAssemblyComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_CampaignMobileAssemblyComponent mobileAssembly = SCR_CampaignMobileAssemblyComponent.Cast(component);
		if (!mobileAssembly.IsDeployed())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteValue("deployed", true);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto mobileAssembly = SCR_CampaignMobileAssemblyComponent.Cast(component);

		int version;
		context.Read(version);

		bool deployed;
		context.Read(deployed);
		if (deployed)
		{
			mobileAssembly.SetParentFactionID(GetGame().GetFactionManager().GetFactionIndex(SCR_FactionManager.SGetFaction(owner)));
			mobileAssembly.UpdateRadioCoverage();
			mobileAssembly.Deploy(SCR_EMobileAssemblyStatus.DEPLOYED, silent: true);
		}

		return true;
	}
}
