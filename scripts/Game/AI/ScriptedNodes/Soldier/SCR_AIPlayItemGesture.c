class SCR_AIPlayItemGesture : AITaskScripted
{
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EItemGesture) )]
	protected EItemGesture m_eGesture;
	
	static override string GetOnHoverDescription() { return "Calls CharacterControllerComponent.TryPlayItemGesture"; }
	override static bool VisibleInPalette() { return true; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity ownerEnt = owner.GetControlledEntity();
		if (!ownerEnt)
			return ENodeResult.FAIL;
		
		CharacterControllerComponent characterControl = CharacterControllerComponent.Cast(ownerEnt.FindComponent(CharacterControllerComponent));
		if (!characterControl)
			return ENodeResult.FAIL;
		
		characterControl.TryPlayItemGesture(m_eGesture);
		return ENodeResult.SUCCESS;
	}
}