[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_RespawnTicketsEditorAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) return null;
		
		#ifndef WORKBENCH
		return null;
		#endif
		return SCR_BaseEditorAttributeVar.CreateFloat(10);
		
	}	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		//TODO: implement logics
		if (!var) return;
		
		Print("Respawn Tickets: " + var.GetFloat());
	}
};