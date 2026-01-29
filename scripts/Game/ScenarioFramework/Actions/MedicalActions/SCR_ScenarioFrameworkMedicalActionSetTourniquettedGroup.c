[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetTourniquettedGroup : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute("10", UIWidgets.ComboBox, "Select Character hit zone group to add/remove tourniquets", "", ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	ECharacterHitZoneGroup	m_eCharacterHitZoneGroup;
	
	[Attribute(defvalue: "{D70216B1B2889129}Prefabs/Items/Medicine/Tourniquet_01/Tourniquet_US_01.et", desc: "Resource name of the Tourniquet you want to use")]
	ResourceName m_sTourniquetPrefab;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Whether target hit zone group is tourniquetted or not")]
	bool m_bTourniquetted;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		if (!m_bTourniquetted)
		{
			SCR_TourniquetStorageComponent tourniquetStorage = SCR_TourniquetStorageComponent.Cast(m_DamageManager.GetOwner().FindComponent(SCR_TourniquetStorageComponent));
			if (!tourniquetStorage)
			{
				Print(string.Format("ScenarioFramework Action: Tourniquet Storage Component not found for Action %1.", this), LogLevel.ERROR);
				return;
			}
	
			tourniquetStorage.RemoveTourniquetFromSlot(m_eCharacterHitZoneGroup, m_DamageManager.GetOwner());
			
			return;
		}
		
		Resource resource = Resource.Load(m_sTourniquetPrefab);
		if (!resource && !resource.IsValid())
			return;
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		m_DamageManager.GetOwner().GetWorldTransform(spawnParams.Transform);
		spawnParams.TransformMode = ETransformMode.WORLD;
		
		IEntity tourniquet = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		if (!tourniquet)
			return;
		
		SCR_ConsumableItemComponent consumableItemComp = SCR_ConsumableItemComponent.Cast(tourniquet.FindComponent(SCR_ConsumableItemComponent));
		if (!consumableItemComp)
			return;
		
		SCR_ConsumableEffectBase consumableEffect = consumableItemComp.GetConsumableEffect();
		if (!consumableEffect)
			return;
		
		ItemUseParameters params = ItemUseParameters();
		params.SetEntity(tourniquet);
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(false);
		params.SetIntParam(m_DamageManager.FindAssociatedBandagingBodyPart(m_eCharacterHitZoneGroup));
		
		consumableEffect.ApplyEffect(m_DamageManager.GetOwner(), m_DamageManager.GetOwner(), tourniquet, params);
	}
}