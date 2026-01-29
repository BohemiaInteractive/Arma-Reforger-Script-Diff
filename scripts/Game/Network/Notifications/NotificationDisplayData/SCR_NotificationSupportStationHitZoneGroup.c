/*!
Used to display which hitzone groups are healed when using support station
Param1: userRpl ID of who healed/Repaired, param2: EVehicleHitZoneGroup or ECharacterHitZoneGroup, param3: percentage or other number, set int.MIN to ignore!
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationSupportStationHitZoneGroup : SCR_NotificationDisplayData
{
	[Attribute(desc: "Config to get the correct hitzone group names from. It will always send over one group so it could be that a group contains multiple hitzone groups in the final notification")]
	protected ref SCR_HitZoneGroupNameHolder m_HitZoneGroupNameHolder;
	
	[Attribute(desc: "Config to get the correct damage intensity string")]
	protected ref SCR_DamageIntensityHolder m_DamageIntensityHolder;
	
	[Attribute("0", desc: "If true will get Character name (first, alias and surname) if the entity is a NPC otherwise will get the entity type name eg: Rifleman")]
	protected bool m_bGetCharacterName;
	
	//------------------------------------------------------------------------------------------------
	override string GetText(SCR_NotificationData data)
	{		
		if (!m_HitZoneGroupNameHolder)
		{
			Print("'SCR_NotificationSupportStationHitZoneGroup' notification: '" + typename.EnumToString(ENotification, m_NotificationKey) + "' is missing the HitzoneGroupDataHolder!", LogLevel.ERROR);
			return string.Empty;
		}
		
		if (!m_DamageIntensityHolder)
		{
			Print("'SCR_NotificationSupportStationHitZoneGroup' notification: '" + typename.EnumToString(ENotification, m_NotificationKey) + "' is missing the DamageIntensityHolder!", LogLevel.ERROR);
			return string.Empty;
		}
			
		int entityID, hitZoneGroup, health;
		data.GetParams(entityID, hitZoneGroup, health);
		
		//~ Get Entity name
		string entityName;
		data.GetNotificationTextEntries(entityName);
		if (!GetEditableEntityName(entityID, entityName, m_bGetCharacterName))
			return string.Empty;
		
		//~ Get group name(s)
		string hitZoneGroupString = m_HitZoneGroupNameHolder.GetHitZoneGroupName(hitZoneGroup);
		
		float healthScaled = health * 0.001;
		SCR_UIName intensityUIInfo = m_DamageIntensityHolder.GetValidIntensityUIInfo(healthScaled);
		if (!intensityUIInfo)
			return string.Empty;
		
		string hitZoneDamageIntensity = intensityUIInfo.GetName();
		
		data.SetNotificationTextEntries(entityName, hitZoneGroupString, hitZoneDamageIntensity);		
		return super.GetText(data);
	}
}
