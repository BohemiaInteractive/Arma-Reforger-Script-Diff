class SCR_AIDropMagazines : AITaskScripted
{
	static const string PORT_MAGAZINE_WELL = "MagazineWell";
	
	[Attribute("0.5", UIWidgets.Slider, "Fraction of magazines to drop", "0 1 1" )]
	protected float m_FractionOfMagazines;
	[Attribute("", UIWidgets.EditBox, "Name of magazine well" )]
	protected string m_sMagazineWellType;
	
	private SCR_InventoryStorageManagerComponent m_Inventory;
	private typename m_oMagazineWell;
	
	//--------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_MAGAZINE_WELL
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//--------------------------------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity ent = owner.GetControlledEntity();
		if (ent)
		{
			m_Inventory = SCR_InventoryStorageManagerComponent.Cast(ent.FindComponent(SCR_InventoryStorageManagerComponent));
		}
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Inventory)
			return ENodeResult.FAIL;
		
		if(!GetVariableIn(PORT_MAGAZINE_WELL,m_oMagazineWell))
			m_oMagazineWell = m_sMagazineWellType.ToType();
		
		array<typename> components = {};
		array<IEntity> foundItems = {};
        components.Insert(MagazineComponent);
       	
		for (int i = m_Inventory.FindItemsWithComponents(foundItems,components, EStoragePurpose.PURPOSE_DEPOSIT)-1; i> -1; i--)
		{
			MagazineComponent magComp = MagazineComponent.Cast(foundItems[i].FindComponent(MagazineComponent));
			if (magComp && magComp.GetMagazineWell().Type() != m_oMagazineWell)
			{
				foundItems.Remove(i);
			}
		}
		// now I have only items that are magazines and have same well type
		
		for (int i = 0, length = Math.Floor((foundItems.Count()-1) * m_FractionOfMagazines); i <= length; i++)
		{
			InventoryItemComponent pInvComp = InventoryItemComponent.Cast(foundItems[i].FindComponent(InventoryItemComponent));
			InventoryStorageSlot parentSlot = pInvComp.GetParentSlot();
			bool removed = m_Inventory.TryRemoveItemFromStorage(foundItems[i],parentSlot.GetStorage());
		}
		return ENodeResult.SUCCESS;
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	override protected string GetNodeMiddleText()
	{
		return "Drop " + (m_FractionOfMagazines*100).ToString() + "% of magazines to ground.";	
	}	
	
	//--------------------------------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "AI task that drops all magazines of specified MagazineWell type.";
	}
};