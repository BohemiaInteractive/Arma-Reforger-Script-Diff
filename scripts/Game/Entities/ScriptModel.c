[EntityEditorProps(category: "GameLib/Scripted", description: "Script model", dynamicBox: true)]
class ScriptModelClass: GenericEntityClass
{

};

class ScriptModel: GenericEntity
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Model", "xob")]
	ResourceName Model;
	[Attribute("1", UIWidgets.ComboBox, "Physics", "", { ParamEnum("None", "2"), ParamEnum("Static", "1"), ParamEnum("Dynamic", "0") } )]
	int Type;	

	void ScriptModel(IEntitySource src, IEntity parent)
	{
		if (Model== "")
			return;

		SetFlags(EntityFlags.ACTIVE | EntityFlags.TRACEABLE | EntityFlags.VISIBLE, false);

		Resource resource = Resource.Load(Model);		
		VObject vobj = resource.GetResource().ToVObject();
		SetObject(vobj, "");

		if (Type == 1)
		{
			Physics.CreateStatic(this, 0xffffffff); // todo - defines for layer mask			
		}
		else if (Type == 0)
		{
			if(!Physics.CreateDynamic(this, 1.0, 0xffffffff))
			{
				//create implicit box
				vector mins, maxs;
				GetBounds(mins, maxs);
				vector center = (mins + maxs) * 0.5;
				vector size = maxs - mins;
								
				ref PhysicsGeomDef geoms[] = {PhysicsGeomDef("", PhysicsGeom.CreateBox(size), "material/default", 0xffffffff)};
				Physics.CreateDynamicEx(this, center, 1, geoms);
			}
			if(GetPhysics())
			{
				GetPhysics().SetMass(1.0);
				GetPhysics().SetActive(ActiveState.ACTIVE);
			}
		}
	}

	void ~ScriptModel()
	{
		if(GetPhysics())
			GetPhysics().Destroy();
	}
};
