//------------------------------------------------------------------------------------------------
//! modded version for to be used with the inventory 2.0 
class SCR_EquipClothAction: SCR_InventoryAction
{
	#ifndef DISABLE_INVENTORY
	
	protected bool m_bIsSwappingItems = false;
	protected string m_sItemToSwapName = "";
	
	protected EquipedLoadoutStorageComponent m_LoadoutStorage;
	protected BaseLoadoutClothComponent m_LoadoutCloth;
	protected SCR_CharacterInventoryStorageComponent m_CharacterStorage;

	
	[Attribute( "#AR-Inventory_Equip", desc: "What text should be displayed when the Equip action is available." )]
	protected string m_sEquipActionString;
	[Attribute( "#AR-Inventory_Swap", desc: "What text should be displayed when the Swap action is available." )]
	protected string m_sSwapActionString;
	[Attribute( "#AR-Inventory_Replaces", desc: "What text should be displayed when the Replace action is available." )]
	protected string m_sReplaceActionString;
	
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		manager.EquipCloth( pOwnerEntity );
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		if ( !super.CanBePerformedScript( user ) )
			return false;
		
		if ( !m_LoadoutStorage )
			m_LoadoutStorage = EquipedLoadoutStorageComponent.Cast( user.FindComponent( EquipedLoadoutStorageComponent ) );
		
		if ( !m_LoadoutCloth )
			return false;	
	
		LoadoutAreaType targetArea = m_LoadoutCloth.GetAreaType();
		if (!targetArea)
			return false;
		
		array<typename> blockedSlots = {};
		m_LoadoutCloth.GetBlockedSlots(blockedSlots);
		foreach (typename blockedArea : blockedSlots)
		{
			if (m_LoadoutStorage.GetClothFromArea(blockedArea))
			{
				m_sItemToSwapName = "";
				m_bIsSwappingItems = false;
				return false;
			}
		}
		
		if (!m_CharacterStorage)
			m_CharacterStorage = SCR_CharacterInventoryStorageComponent.Cast(user.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		if (m_CharacterStorage)
		{
			blockedSlots.Clear();
			m_CharacterStorage.GetBlockedSlots(blockedSlots);
			if (blockedSlots.Contains(targetArea.Type()))
			{
				m_sItemToSwapName = "";
				m_bIsSwappingItems = false;
				return false;
			}
		}
		
		// Checks if the desired area is occupied. If it's occupied it will save the UI info related to the replaced item is saved.
		if (m_LoadoutStorage)
		{
			IEntity itemToSwap = m_LoadoutStorage.GetClothFromArea( targetArea.Type() );
			if (itemToSwap)
			{
				InventoryItemComponent itemComp = InventoryItemComponent.Cast( itemToSwap.FindComponent( InventoryItemComponent ) );
			
				// If the component is missing configuration we can fall back on the target area name.
				if ( !itemComp || !itemComp.GetAttributes().GetUIInfo().GetName() )
				{
					m_sItemToSwapName =	targetArea.ToString();
					m_bIsSwappingItems = false;
					
					return true;
				}
				
				m_sItemToSwapName = itemComp.GetAttributes().GetUIInfo().GetName();
				m_bIsSwappingItems = true;
			}
			else
			{
				m_sItemToSwapName = "";
				m_bIsSwappingItems = false;
			}
		}
		else
		{
			m_sItemToSwapName = "";
			m_bIsSwappingItems = false;
		}
		
		return true;
 	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (m_bIsSwappingItems)
		{
			string replaceAction = string.Format(WidgetManager.Translate(m_sReplaceActionString), WidgetManager.Translate(m_sItemToSwapName));
			outName = string.Format( "%1 %2", m_sSwapActionString, replaceAction );
		}
		else
			outName = m_sEquipActionString;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		if (!m_Item)
			return;
		IEntity item = m_Item.GetOwner();
		if (!item)
			return;
		m_LoadoutCloth = BaseLoadoutClothComponent.Cast( item.FindComponent( BaseLoadoutClothComponent ) );
		
		if (!m_LoadoutCloth)
		{
			Print("Missing BaseLoadoutClothComponent", LogLevel.ERROR);
			return;
		}
	}
	
	#endif
};