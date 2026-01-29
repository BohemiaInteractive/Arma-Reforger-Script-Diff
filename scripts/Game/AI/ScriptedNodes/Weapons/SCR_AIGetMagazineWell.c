class SCR_AIGetMagazineWell: AITaskScripted
{
	static const string PORT_MAGAZINE_WELL_OUT	= "MagazineWellOut";
	
	private IEntity m_ownerEntity;
	private BaseWeaponManagerComponent m_weaponManager;
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_ownerEntity = owner.GetControlledEntity();
		if (!m_ownerEntity)
			Debug.Error("Must be run on ai agent that has owner entity!");
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_weaponManager)
		{
			if ( !m_ownerEntity )
				return ENodeResult.FAIL;
			m_weaponManager = BaseWeaponManagerComponent.Cast(m_ownerEntity.FindComponent(BaseWeaponManagerComponent));
			if (!m_weaponManager)
				return ENodeResult.FAIL;
		}
		BaseWeaponComponent weapon = m_weaponManager.GetCurrent();
		if (!weapon) 
			return ENodeResult.FAIL;
		
		SetVariableOut(PORT_MAGAZINE_WELL_OUT, GetWeaponMainMagazineWell(weapon));
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_MAGAZINE_WELL_OUT
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription() 
	{ 
		return "Returns MagazineWell of current weapon on Owner's controlled entity";
	};
	
	//------------------------------------------------------------------------------------------------
	static typename GetWeaponMainMagazineWell(BaseWeaponComponent weapon)
	{
		ref array<BaseMuzzleComponent> outMuzzles = new array<BaseMuzzleComponent>;
		weapon.GetMuzzlesList(outMuzzles);
		if (!outMuzzles.IsEmpty())
			return outMuzzles[0].GetMagazineWell().Type();
		else
			return BaseWeaponComponent;
	}
};
