class SCR_VolumeDataProviderComponentClass: VolumeDataProviderComponentClass
{
};

enum EVolumeCategories
{
	Freshwater,
	Ladders
};

class SCR_VolumeDataProviderComponent : VolumeDataProviderComponent
{
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EVolumeCategories), desc: "Volume Category")]
	protected string Category;
	
	void SCR_VolumeDataProviderComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetCategory(Category);
	}
};
