class SCR_CampaignBuildingCompositionComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_CampaignBuildingCompositionComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_CampaignBuildingCompositionComponent buildableCompostion = SCR_CampaignBuildingCompositionComponent.Cast(component);

		// If we do not find a layout anymore it was already fully built
		float current = 1.0;
		int max = 1;
		const SCR_CampaignBuildingLayoutComponent layout = buildableCompostion.GetCompositionLayout();
		if (layout)
		{
			current = layout.GetCurrentBuildValue();
			max = layout.GetToBuildValue();
		}

		// It will spawn by with no progress due to BuildableEntitySerializer.DeserializeSpawnData(), so we can abort if that is the desired outcome.
		if (float.AlmostEqual(current, 0.0))
			return ESerializeResult.DEFAULT;

		const float progress = current / max;

		context.WriteValue("version", 1);
		context.Write(progress);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_CampaignBuildingCompositionComponent buildableCompostion = SCR_CampaignBuildingCompositionComponent.Cast(component);

		int version;
		context.Read(version);

		float progress;
		context.Read(progress);

		SCR_CampaignBuildingLayoutComponent layout = buildableCompostion.GetCompositionLayout();
		if (layout)
		{
			layout.SetBuildingValue(layout.GetToBuildValue() * progress);
		}
		else if (float.AlmostEqual(progress, 1.0))
		{
			// If no layout exists, its not buildable in phases and we want to construct it instantly via link component
			SCR_EditorLinkComponent linkComponent = SCR_EditorLinkComponent.Cast(owner.FindComponent(SCR_EditorLinkComponent));
			if (linkComponent)
				linkComponent.SpawnComposition();
		}

		return true;
	}
}
