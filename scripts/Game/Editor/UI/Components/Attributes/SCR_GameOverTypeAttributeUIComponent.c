class SCR_GameOverTypeAttributeUIComponent: SCR_DropdownEditorAttributeUIComponent
{	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		//Makes sure that var is considered edited as the starting value is -1
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		var.SetInt(0);
		    
		super.Init(w, attribute);
	}
};