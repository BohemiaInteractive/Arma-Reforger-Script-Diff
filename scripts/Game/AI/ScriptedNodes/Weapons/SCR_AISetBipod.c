class SCR_AISetBipod : AITaskScripted
{
	[Attribute("true", UIWidgets.CheckBox, "When true, bipod will be unfolded, otherwise it will be folded")]
	protected bool m_bUnfoldBipod;
	
	//------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(owner.GetControlledEntity());
		if (!character)
			return ENodeResult.FAIL;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return ENodeResult.FAIL;
		
		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if (!weaponManager)
			return ENodeResult.FAIL;
		
		BaseWeaponComponent weapon = weaponManager.GetCurrentWeapon();
		if (!weapon || !weapon.HasBipod())
			return ENodeResult.FAIL;
		
		weapon.SetBipod(m_bUnfoldBipod);
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	//------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription() { return "Unfolds or folds a bipon on current weapon";}
}