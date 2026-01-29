class SCR_AISmartActionSystem : AISmartActionSystem
{
	protected void OnEntityTransformChanged(SCR_EditableEntityComponent editableEntity, vector prevTransfom[4])
	{
		AISmartActionComponent comp = AISmartActionComponent.Cast(editableEntity.GetOwner().FindComponent(AISmartActionComponent));
		if (comp)
			UpdateSmartAction(comp);
	}
	
	override event protected void OnInit()
	{
		super.OnInit();
		
		SCR_EditableEntityCore editableEntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!editableEntityCore)
			return;
		
		editableEntityCore.Event_OnEntityTransformChangedServer.Insert(OnEntityTransformChanged);	
	}

	override event protected void OnCleanup()
	{		
		super.OnCleanup();
		
				SCR_EditableEntityCore editableEntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!editableEntityCore)
			return;
		
		editableEntityCore.Event_OnEntityTransformChangedServer.Remove(OnEntityTransformChanged);	
	}
}