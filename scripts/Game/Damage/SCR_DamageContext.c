/*!
\addtogroup Damage
\{
*/

//
class SCR_DamageContext: BaseDamageContext
{
	void SCR_DamageContext(EDamageType type, float damage, out vector hitPosDirNorm[3], IEntity entity, HitZone hitzone, notnull Instigator dmgDealer, 
	GameMaterial hitMaterial, int colliderIndex, int boneNodeIndex)
	{
        damageType = type;
        damageValue = damage;
        hitEntity = entity;
        colliderID = colliderIndex;
        struckHitZone = hitzone;
        instigator = dmgDealer;
        material = hitMaterial;
        hitPosition = hitPosDirNorm[0];
        hitDirection = hitPosDirNorm[1];
        hitNormal = hitPosDirNorm[2];
        boneIndex = boneNodeIndex;
	}
    
	//support for custom variables hasn't been implemented yet.
};