class SCR_AIGetWeaponOfType : AITaskScripted
{
	static const string PORT_WEAPON_COMPONENT = "WeaponComponent";
	static const string PORT_WEAPON_TYPE = "WeaponType";
	
	[Attribute("1", UIWidgets.ComboBox, "Wanted weapon type", "", ParamEnumArray.FromEnum(EWeaponType) )]
	protected EWeaponType m_WeaponType;
	
	private SCR_AICombatComponent m_CombatComponent;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity ent = owner.GetControlledEntity();
		if (ent)
			m_CombatComponent = SCR_AICombatComponent.Cast(ent.FindComponent(SCR_AICombatComponent));				
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_CombatComponent)
			return ENodeResult.FAIL;
			
		EWeaponType weaponType;
		
		if (!GetVariableIn(PORT_WEAPON_TYPE, weaponType))
			weaponType = m_WeaponType;
		
		BaseWeaponComponent weaponComp = m_CombatComponent.FindWeaponOfType(weaponType);
		
		if (!weaponComp)
			return ENodeResult.FAIL;
			
		SetVariableOut(PORT_WEAPON_COMPONENT, weaponComp);
		return ENodeResult.SUCCESS;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {PORT_WEAPON_TYPE};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {PORT_WEAPON_COMPONENT};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
};