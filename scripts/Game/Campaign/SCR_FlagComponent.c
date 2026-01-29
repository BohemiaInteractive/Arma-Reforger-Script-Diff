[EntityEditorProps(category: "GameScripted/Campaign", description: "Simple component for changing material of flags and signs.", color: "0 0 255 255")]
class SCR_FlagComponentClass : SCR_MilitaryBaseLogicComponentClass
{
	[Attribute("1", UIWidgets.CheckBox, "Is Owner entity Flag Pole?", "")]
	protected bool m_bIsFlagPole;

	[Attribute("{9BE1341F99D33483}Assets/Props/Fabric/Flags/Data/Flag_1_2_EVERON.emat", UIWidgets.ResourcePickerThumbnail, "Default material to be used.", params: "emat")]
	protected ResourceName m_sDefaultMaterial;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Optional MLOD, if required.", params: "emat")]
	protected ResourceName m_sDefaultMLOD;

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsFlagPole()
	{
		return m_bIsFlagPole;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetDefaultMaterial()
	{
		return m_sDefaultMaterial;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetDefaultMLOD()
	{
		return m_sDefaultMLOD;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_FlagComponent : SCR_MilitaryBaseLogicComponent
{
	//------------------------------------------------------------------------------------------------
	//! Changes Material used on Flag or Sign
	//! Parameters: flagResource = material to be used, resourceMlod = MLOD , Flags doesn't need this, only signs

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] flagResource
	//! \param[in] resourceMLOD
	void ChangeMaterial(ResourceName flagResource, ResourceName resourceMLOD = string.Empty)
	{
		IEntity flag = GetFlagEntity();
		if (!flag)
			return;

		SignalsManagerComponent signalsManager = SignalsManagerComponent.Cast(flag.FindComponent(SignalsManagerComponent));
		
		if (flagResource.IsEmpty())
		{
			flag.ClearFlags(EntityFlags.VISIBLE);
			
			if (signalsManager)
				signalsManager.SetSignalValue(signalsManager.AddOrFindSignal("IsNotVisible"), 1);
	
			return;
		}

		flag.SetFlags(EntityFlags.VISIBLE);
		
		if (signalsManager)
			signalsManager.SetSignalValue(signalsManager.AddOrFindSignal("IsNotVisible"), 0);
		
		//Custom set material. Finds original material "slot" with MLOD and assigns appropriate new one, if possible.
		VObject mesh = flag.GetVObject();
		if (!mesh)
			return;

		string remap;
		string materials[256];
		int numMats = mesh.GetMaterials(materials);

		for (int i = 0; i < numMats; i++)
		{
			if (materials[i].Contains("MLOD"))
				remap += string.Format("$remap '%1' '%2';", materials[i], resourceMLOD);
			else
				remap += string.Format("$remap '%1' '%2';", materials[i], flagResource);
		}

		flag.SetObject(mesh, remap);
	}

	//------------------------------------------------------------------------------------------------
	//! \return entity with flag from slotComponent or return owner if component is directly on flag entity
	IEntity GetFlagEntity()
	{
		SCR_FlagComponentClass prefabData = SCR_FlagComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData && !prefabData.IsFlagPole())
			return GetOwner();

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent));
		if (!slotManager)
			return null;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		foreach (EntitySlotInfo slot : slots)
		{
			IEntity flag = slot.GetAttachedEntity();
			if (!flag)
				continue;

			return flag;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		SCR_FlagComponentClass prefabData = SCR_FlagComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
			ChangeMaterial(prefabData.GetDefaultMaterial(), prefabData.GetDefaultMLOD());
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
}
