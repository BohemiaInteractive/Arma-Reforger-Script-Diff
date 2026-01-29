//------------------------------------------------------------------------------------------------
//! modded version for to be used with the inventory 2.0 
class SCR_InventoryAction : ScriptedUserAction
{
	#ifndef DISABLE_INVENTORY
	InventoryItemComponent 			m_Item;
	protected const string			SREASON_TOO_BIG 	= "#AR-Inventory_InfoTooBig";
	protected const string			SREASON_TOO_HEAVY 	= "#AR-Inventory_InfoTooHeavy";
	
	//------------------------------------------------------------------------------------------------
	protected string GetReason( SCR_InventoryStorageManagerComponent pInventoryManager )
	{
		if( !pInventoryManager )
			return "";
		EInventoryRetCode ERetCode = pInventoryManager.GetReturnCode();
		string sReason = "";
		
		//ERetCode &= EInventoryRetCode.RETCODE_ITEM_TOO_BIG;
		
		if ( ( ERetCode & EInventoryRetCode.RETCODE_ITEM_TOO_BIG ) == EInventoryRetCode.RETCODE_ITEM_TOO_BIG )
			sReason += " " + SREASON_TOO_BIG;
				
		if ( ( ERetCode & EInventoryRetCode.RETCODE_ITEM_TOO_HEAVY ) == EInventoryRetCode.RETCODE_ITEM_TOO_HEAVY )
			sReason += " " + SREASON_TOO_HEAVY;
		return sReason;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (m_Item == null || m_Item.GetParentSlot() != null || m_Item.IsLocked())
			return false;
		
		IEntity itemEntity = m_Item.GetOwner();
		BaseMagazineComponent baseMagComponent = BaseMagazineComponent.Cast(itemEntity.FindComponent(BaseMagazineComponent));
		if (baseMagComponent)
		{
			if (baseMagComponent.IsUsed())
				return false;
			
			// This is a temporary fix for issue #18454,
			// we should lock items during reload instead
			IEntity itemParent = itemEntity.GetParent();
			if (ChimeraCharacter.Cast(itemParent))
				return false;
		}
		
		return true;
	}	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		if (!user)
			return false;
		Managed genericInventoryManager = user.FindComponent( SCR_InventoryStorageManagerComponent );
		if (!genericInventoryManager)
			return false;
		RplComponent genericRpl = RplComponent.Cast(user.FindComponent( RplComponent ));
		if (!genericRpl)
			return false;
		
		return genericRpl.IsOwner();
 	}
 	//---------------------------------------------------------
 	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		if (!CanBePerformedScript(pUserEntity))
		 	return;
		SCR_InventoryStorageManagerComponent genericInventoryManager =  SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent( SCR_InventoryStorageManagerComponent ));
		if ( !genericInventoryManager )
			return;
		PerformActionInternal( genericInventoryManager, pOwnerEntity, pUserEntity);
		
 	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		return false;
	}

	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
	
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_Item = InventoryItemComponent.Cast( pOwnerEntity .FindComponent( InventoryItemComponent ));
	}
		//------------------------------------------------------------------------------------------------
	#else
	override bool GetActionNameScript(out string outName)
	{
		outName = "#AR-Inventory_Disabled";
		return true;
	}
	#endif	
	
};