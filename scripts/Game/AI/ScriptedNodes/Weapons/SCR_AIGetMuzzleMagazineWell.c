class SCR_AIGetMuzzleMagazineWell: AITaskScripted
{
	protected const static string WEAPON_COMPONENT_PORT = "WeaponComponent";	
	protected const static string MUZZLE_ID_PORT = "MuzzleId";
	
	static const string MAGAZINE_WELL_PORT = "MagazineWell";
	static const string MAGAZINE_WELL_TYPE_PORT = "MagazineWellType";
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		BaseWeaponComponent weaponComp;
		int muzzleId;
		
		if (!GetVariableIn(WEAPON_COMPONENT_PORT, weaponComp))
			return ENodeResult.FAIL;
		if (!GetVariableIn(MUZZLE_ID_PORT, muzzleId))
			return ENodeResult.FAIL;
		
		if (!weaponComp)
			return ENodeResult.FAIL;
		
		array<BaseMuzzleComponent> muzzles = {};
		weaponComp.GetMuzzlesList(muzzles);
		
		if (muzzleId < 0 || muzzleId >= muzzles.Count())
			return ENodeResult.FAIL;
		
		BaseMuzzleComponent muzzleComp = muzzles[muzzleId];
		
		if (!muzzleComp)
			return ENodeResult.FAIL;
		
		BaseMagazineWell magWell = muzzleComp.GetMagazineWell();
		if (!magWell)
			return ENodeResult.FAIL;
		
		typename magWellType = magWell.Type();
		SetVariableOut(MAGAZINE_WELL_PORT, magWell);
		SetVariableOut(MAGAZINE_WELL_TYPE_PORT, magWellType);
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = { MAGAZINE_WELL_PORT, MAGAZINE_WELL_TYPE_PORT };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static ref TStringArray s_aVarsIn = { WEAPON_COMPONENT_PORT, MUZZLE_ID_PORT };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	

	//------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription() 
	{ 
		return "Returns magazine well and its type of a specific weapon muzzle";	
	};
};
