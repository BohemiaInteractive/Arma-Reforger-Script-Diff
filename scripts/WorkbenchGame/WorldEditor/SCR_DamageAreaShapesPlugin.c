#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Optimize Damage Area Shapes", description: "Attempts to adjust shapes of the selected damage area entities.", wbModules: { "WorldEditor" }, awesomeFontCode: 0xf714)]
class SCR_DamageAreaShapesPlugin : WorldEditorPlugin
{
	protected const string PROPERTY_COORDS = "coords";
	protected const string PROPERTY_SHAPE = "Area shape";
	protected const string PROPERTY_AREA = "DamageAreas";
	protected const string PROPERTY_COMPONENTS = "components";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		WorldEditorAPI api = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!api)
			return;

		if (api.GetSelectedEntity() == null)
		{
			SCR_WorkbenchHelper.PrintDialog("Select entities for optimization!", level: LogLevel.ERROR);
			return;
		}

		IEntity ent;
		IEntitySource entSrc;
		SCR_DamageAreaComponent dmgAreaComp;
		array<ref DamageArea> damageAreas = {};

		api.BeginEntityAction();

		bool anythingChanged;
		for (int i, count = api.GetSelectedEntitiesCount(); i < count; i++)
		{
			entSrc = api.GetSelectedEntity(i);
			if (!entSrc)
				continue;

			ent = api.SourceToEntity(entSrc);
			if (!ent)
				continue;

			dmgAreaComp = SCR_DamageAreaComponent.Cast(ent.FindComponent(SCR_DamageAreaComponent));
			if (!dmgAreaComp)
				continue;

			int componentIndex = SCR_BaseContainerTools.FindComponentIndex(entSrc, SCR_DamageAreaComponent);
			ContainerIdPathEntry componentPath = new ContainerIdPathEntry(PROPERTY_COMPONENTS, componentIndex);
			ContainerIdPathEntry areaPath = new ContainerIdPathEntry(PROPERTY_AREA);
			ContainerIdPathEntry shapePath = new ContainerIdPathEntry(PROPERTY_SHAPE);
			array<ref ContainerIdPathEntry> finalPath;
	
			SCR_BaseDamageAreaShape shape;
			dmgAreaComp.GetDamageAreas(damageAreas);
			foreach (int areaId, DamageArea dmgArea : damageAreas)
			{
				if (!dmgArea)
					continue;

				shape = SCR_BaseDamageAreaShape.Cast(dmgArea.GetShape());
				if (!shape)
					continue;

				vector offset;
				areaPath.Index = areaId;
				finalPath = {componentPath, areaPath, shapePath};
				if (!shape.OptimizeShape(entSrc, api, finalPath, offset))
					continue; // it did nothing so we can skip

				anythingChanged = true;
				// Move the entity to such position that its moddified area is in the same place
				vector newEntPos;
				entSrc.Get(PROPERTY_COORDS, newEntPos);
				newEntPos += offset;
				api.SetVariableValue(entSrc, null, PROPERTY_COORDS, newEntPos.ToString(false));
			}
		}

		api.EndEntityAction();
		if (anythingChanged)
			SCR_WorkbenchHelper.PrintDialog("Recentering has been finished. Dont forget to save the changes!", level: LogLevel.WARNING);
	}
}
#endif