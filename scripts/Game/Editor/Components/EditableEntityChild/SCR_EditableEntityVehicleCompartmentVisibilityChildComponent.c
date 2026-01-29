[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "")]
class SCR_EditableEntityVehicleCompartmentVisibilityChildComponentClass : SCR_EditableEntityVisibilityChildComponentClass
{
}

class SCR_EditableEntityVehicleCompartmentVisibilityChildComponent : SCR_EditableEntityVisibilityChildComponent
{	
	//------------------------------------------------------------------------------------------------
	//~ When compartment is added to owner make sure to update the state so it also sets the flag to VIRTUAL
	protected override void OnAddedToParent(IEntity child, IEntity parent)
	{
		UpdateFromCurrentState();
	}
}
