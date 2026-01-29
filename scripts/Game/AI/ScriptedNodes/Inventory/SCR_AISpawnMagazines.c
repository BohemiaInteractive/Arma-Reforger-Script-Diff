class SCR_AISpawnMagazines : AITaskScripted
{
	static const string PORT_MAGAZINE_WELL = "MagazineWell";
	
	ref map<string, string> m_mTypenameToResourceName;
	
	[Attribute("", UIWidgets.EditBox, "Name of magazine well" )]
	protected string m_sMagazineWellType;
	
	[Attribute("4", UIWidgets.Slider, "Amount of magazines", "0 10 1" )]
	protected int m_iMagazineCount;

	private typename m_oMagazineWell;
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_MAGAZINE_WELL
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	override void OnInit(AIAgent owner)
	{
		m_mTypenameToResourceName = new map<string, string>;
		m_mTypenameToResourceName.Insert("MagazineWellAK545","{E5912E45754CD421}Prefabs/Weapons/Magazines/Magazine_545x39_AK_30rnd_Tracer.et");
		m_mTypenameToResourceName.Insert("MagazineWellStanag556","{A9A385FE1F7BF4BD}Prefabs/Weapons/Magazines/Magazine_556x45_STANAG_30rnd_Tracer.et");
		m_mTypenameToResourceName.Insert("MagazineWellPKM","{BEEA49E27174B224}Prefabs/Weapons/Magazines/Box_762x54_PK_100rnd_Tracer.et");
		m_mTypenameToResourceName.Insert("MagazineWellM60","{AD8AB93729348C3E}Prefabs/Weapons/Magazines/Box_762x51_M60_100rnd_Tracer.et");
		m_mTypenameToResourceName.Insert("MagazineWellM14","{6D18CC33708EE712}Prefabs/Weapons/Magazines/Magazine_762x51_M14_20rnd_Base.et");
		m_mTypenameToResourceName.Insert("MagazineWellSVD","{9CCB46C6EE632C1A}Prefabs/Weapons/Magazines/Magazine_762x54_SVD_10rnd_Sniper.et");
		m_mTypenameToResourceName.Insert("MagazineWellVZ58_762","{FAFA0D71E75CEBE2}Prefabs/Weapons/Magazines/Vz58/Magazine_762x39_Vz58_30rnd_Tracer.et");
		m_mTypenameToResourceName.Insert("MagazineWellM9Beretta","{9C05543A503DB80E}Prefabs/Weapons/Magazines/Magazine_9x19_M9_15rnd_Ball.et");
		m_mTypenameToResourceName.Insert("MagazineWellMakarovPM","{8B853CDD11BA916E}Prefabs/Weapons/Magazines/Magazine_9x18_PM_8rnd_Ball.et");
		m_mTypenameToResourceName.Insert("MagazineWellM249","{06D722FC2666EB83}Prefabs/Weapons/Magazines/Box_556x45_M249_200rnd_4Ball_1Tracer.et");
		m_mTypenameToResourceName.Insert("MagazineWellRPG7","{32E12D322E107F1C}Prefabs/Weapons/Ammo/Ammo_Rocket_PG7VM.et");
	}	
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!GetVariableIn(PORT_MAGAZINE_WELL,m_oMagazineWell))
			m_oMagazineWell = m_sMagazineWellType.ToType();
		
		ResourceName resourceName;
		if (m_mTypenameToResourceName.Find(m_oMagazineWell.ToString(),resourceName))
		{
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = owner.GetControlledEntity().GetOrigin();	
			Resource res = Resource.Load(resourceName);
			
			for (int i = 0; i < m_iMagazineCount; i++)
				GetGame().SpawnEntityPrefab(res, null, params);
			return ENodeResult.SUCCESS;			
		}
		return ENodeResult.FAIL;			
	}	
		
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	protected static override string GetOnHoverDescription()
	{
		return "AI task that picks up all magazines of provided MagazineWell type in the vicinity of its inventory.";
	}	
};