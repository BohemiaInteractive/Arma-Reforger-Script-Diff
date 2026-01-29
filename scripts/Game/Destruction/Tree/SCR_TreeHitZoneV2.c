//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// TODO: Data redundancy is very high, variables are private and replication handling is invalid
class SCR_TreeHitZoneV2 : SCR_HitZone
{
	[Attribute("0", UIWidgets.EditBox, "Enter the tree part number.")]
	private int m_iTreePartIndex;
	
#ifdef ENABLE_DESTRUCTION
	private SCR_TreePartV2 m_OwnerPart;
	private SCR_DestructibleTreeV2 m_OwnerTree;
	private IEntity m_OwnerEntity;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent) 
	{
		//Determine whether this hit zone is on a tree part or a destructible tree
		m_OwnerPart = SCR_TreePartV2.Cast(pOwnerEntity);
		if (!m_OwnerPart)
		{
			m_OwnerEntity = pOwnerEntity;
			while (m_OwnerEntity)
			{
				m_OwnerTree = SCR_DestructibleTreeV2.Cast(m_OwnerEntity);
				if (m_OwnerTree)
					return;

				m_OwnerEntity = m_OwnerEntity.GetParent();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDamage(float damage,
							EDamageType type,
							IEntity pHitEntity,
							inout vector outMat[3],
							IEntity damageSource,
							notnull Instigator instigator,
							int colliderID,
							float speed)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		//Calculate direction vector based on normal of the hit
		vector directionVector = -outMat[2];
		directionVector.Normalize();
		
		AddDamage(damage);
		//If the damage / impulse was big enough to move the tree part
		if (GetDamageState() == EDamageState.DESTROYED)
		{
			//Calculate impulse vector from direction vector and damage
			vector positionVector = outMat[0];
			
			if (type == EDamageType.EXPLOSIVE)
			{
				positionVector = "0 0 0";
				if (m_OwnerEntity)
					directionVector = (m_OwnerEntity.GetOrigin() - outMat[0]) * damage;
			}
			
			directionVector.Normalize();
			vector impulseVector = directionVector * (damage / 10);
			if (m_OwnerPart)
			{
				m_OwnerPart.SetToBreak(positionVector: positionVector, impulseVector: impulseVector, damageType: type);
				return;
			}

			if (m_OwnerTree)
			{
				//m_OwnerTree.SetToDestroy(positionVector, impulseVector, m_iTreePartIndex);
			}
		}
	}
#endif
}
//---- REFACTOR NOTE END ----
