[EntityEditorProps(category: "GameScripted/Components", description: "")]
class SCR_PlaceableItemComponentClass : ScriptComponentClass
{
	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Can this entity be attached to dynamic objects.")]
	protected bool m_bCanAttachToDynamicObject;

	[Attribute(defvalue: "0", desc: "Max Allowed Tilt\n0 = no tilt allowed\nvalue below 0 means that there will be no tilt validation", params: "-1 180 0.01")]
	protected float m_fMaxAllowedTilt;

	[Attribute(uiwidget: UIWidgets.Flags, desc: "Set of flags that will be used to ignore objects based on their physics layer.\nWARNING: To prevent players from attaching objects to weapons use Ignored Components list\nas checking weapon layer will make it impossible to attach the object to the armed vehicles!", enums: ParamEnumArray.FromEnum(EPhysicsLayerDefs))]
	protected EPhysicsLayerDefs m_eIgnoredPhysicsLayers;

	[Attribute(uiwidget: UIWidgets.Object, desc: "List of components that when present on target entity or its parent will make it impossible to attach placeable item to it")]
	protected ref array<string> m_aIgnoredComponents;

	[Attribute(SCR_ECharacterDistanceMeasurementMethod.FROM_EYES.ToString(), UIWidgets.ComboBox, "How the distance between player and placment position should be calculated", enums: ParamEnumArray.FromEnum(SCR_ECharacterDistanceMeasurementMethod))]
	protected SCR_ECharacterDistanceMeasurementMethod m_eMeasurementMethod;

	[Attribute(params: "xob")]
	protected ResourceName m_sPreviewObject;
	
	[Attribute("0.5084", desc: "Max placement distance in meters.")]
	protected float m_fMaxPlacementDistance;
	
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EPlacementType))]
	protected SCR_EPlacementType m_ePlacementType;

	[Attribute(defvalue: "0", desc: "Should forward vector be facing away from the player when item is placed.")]
	protected bool m_bForwardAwayFromPlayer;

	[Attribute(defvalue: "1", desc: "Should item be attached to the hierachy of the entity on which item is placed")]
	protected bool m_bAttachPlacedItemToTheSurfaceEntity;

	[Attribute(defvalue: "0", desc: "Determines if player should be able to force game to place the item.\nWhen force placment is used, then game does not care about having enough space, but tilt is still evaluated")]
	protected bool m_bCanBeForcedPlaced;

	protected ref array<typename> m_aIgnoredComponentTypes = {};
	protected bool m_bValidated;

	//------------------------------------------------------------------------------------------------
	//! Returns true if it should be possible to attach this object to dynamic object like f.e. vehicle
	bool CanAttachToDynamicObject()
	{
		return m_bCanAttachToDynamicObject;
	}

	//------------------------------------------------------------------------------------------------
	//!
	float GetMaxAllowedTilt()
	{
		return m_fMaxAllowedTilt;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns physics layer mask that can be used to determin which layers should be ignored
	EPhysicsLayerDefs GetIgnoredPhysicsLayers()
	{
		return m_eIgnoredPhysicsLayers;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns preferred Measurement method
	SCR_ECharacterDistanceMeasurementMethod GetDistanceMeasurementMethod()
	{
		return m_eMeasurementMethod;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns physics layer mask that can be used to determin which layers should be ignored
	int GetIgnoredComponents(notnull out array<typename> outIgnoredComponents)
	{
		outIgnoredComponents.Clear();
		foreach (typename comp : m_aIgnoredComponentTypes)
		{
			outIgnoredComponents.Insert(comp);
		}

		return outIgnoredComponents.Count();
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetPreviewObjectResource()
	{
		return m_sPreviewObject;
	}

	//------------------------------------------------------------------------------------------------
	float GetMaxPlacementDistance()
	{
		return m_fMaxPlacementDistance;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EPlacementType GetPlacementType()
	{
		return m_ePlacementType;
	}

	//------------------------------------------------------------------------------------------------
	bool IsForwardFacingAwayFromPlayer()
	{
		return m_bForwardAwayFromPlayer;
	}

	//------------------------------------------------------------------------------------------------
	bool CanAttachToTheSurfaceEntity()
	{
		return m_bCanAttachToDynamicObject;
	}

	//------------------------------------------------------------------------------------------------
	bool CanBeForcePlaced()
	{
		return m_bCanBeForcedPlaced;
	}

	//------------------------------------------------------------------------------------------------
	//! Removes unwanted types from ignored components list
	void ValidateIgnoredComponents()
	{
		if (m_bValidated || !m_aIgnoredComponents)
			return;

		m_bValidated = true;
		typename comp;
		for (int i = m_aIgnoredComponents.Count() - 1; i >= 0; i--)
		{
			comp = m_aIgnoredComponents[i].ToType();

			if (!comp.IsInherited(GenericComponent))
			{
#ifdef WORKBENCH
				Print("WARNING! " + m_aIgnoredComponents[i] + " is not inherited from GenericComponent and thus will be removed from the list of ignored components!", LogLevel.ERROR);
#endif
				m_aIgnoredComponents.Remove(i);
			}
			else
			{
				m_aIgnoredComponentTypes.Insert(comp);
			}
		}
	}
}

class SCR_PlaceableItemComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EPlacementType GetPlacementType()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return false;

		return data.GetPlacementType();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMaxPlacementDistance()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return 0;

		return data.GetMaxPlacementDistance();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetForwardAwayFromPlayer()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return false;

		return data.IsForwardFacingAwayFromPlayer();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetAttachPlacedItemToTheSurfaceEntity()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return false;

		return data.CanAttachToTheSurfaceEntity();
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if item can be forced to be placed at specified position, despite not having enough space for it
	bool GetCanBeForcedPlaced()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return false;

		return data.CanBeForcePlaced();
	}

	//------------------------------------------------------------------------------------------------
	//!
	float GetMaxAllowedTilt()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return -1;

		return data.GetMaxAllowedTilt();
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to execute custom space validation
	//! \param[in] caller
	//! \param[in,out] transform pointer containing rotation and position at which item will be placed. This position has already applied offset of 1% of its up vector (1cm)
	//! \param[out] cantPlaceReason
	//! \return true if custom space validation was performed, otherwise false
	bool OverrideSpaceValidation(notnull SCR_ItemPlacementComponent caller, inout vector transform[4], out ENotification cantPlaceReason);

	//------------------------------------------------------------------------------------------------
	//! Method executed when player confirms item placement by pressing SCR_ItemPlacementComponent.ACTION_NAME_PLACEMENT
	//! \param[in] caller
	//! \param[out] skipItemUsage true if game should not try to player item usage animation and immidietly proceed to SCR_ItemPlacementComponent.OnPlacingEnded
	//! \return true if result of this mehtod should be used
	bool OverrideStartPlaceAction(notnull SCR_ItemPlacementComponent caller, out bool skipItemUsage = false);

	//------------------------------------------------------------------------------------------------
	//! Method called after item placement was already requested, but depending on the client it may have not yet finished
	//! \param[in] caller
	//! \return false if placement should be finished by disabling the preview, if custom functionality is reqired then return true
	bool OverrideOnPlacingEnded(notnull SCR_ItemPlacementComponent caller);

	//------------------------------------------------------------------------------------------------
	//! Method called after item was succesfully removed from clients inventory
	//! \param[in] caller
	//! \param[in] item
	//! \param[in] succes
	//! \param[in] equipNext
	void OverrideAfterItemPlaced(notnull SCR_ItemPlacementComponent caller, notnull IEntity item, bool success, bool equipNext);

	//------------------------------------------------------------------------------------------------
	//! Override this in order to add a custom callback that is going to be triggered after item is removed from this client inventory
	//! \param[out] callBackHolder
	//! \param[in] caller component which called this method
	//! \param[in] placedItem 
	//! \param[in] autoEquipNext bool value representing player settings for automatic equipping of next place
	//! \return true if default mechanism should not be used, otherwise false
	bool OverrideAutoEquipMechanism(out ScriptedInventoryOperationCallback callBackHolder, notnull SCR_ItemPlacementComponent caller, IEntity placedItem, bool autoEquipNext);

	//------------------------------------------------------------------------------------------------
	//! \return
	VObject GetPreviewVobject()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return GetOwner().GetVObject();

		if (data.GetPreviewObjectResource().IsEmpty())
			return GetOwner().GetVObject();
		
		Resource resource = Resource.Load(data.GetPreviewObjectResource());
		if (!resource.IsValid())
			return GetOwner().GetVObject();
		
		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
			return GetOwner().GetVObject();
		
		return resourceObject.ToVObject();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if it should be possible to attach this object to dynamic object like f.e. vehicle
	bool CanAttachToDynamicObject()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return false;

		return data.CanAttachToDynamicObject();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns physics layer mask that can be used to determin which layers should be ignored
	EPhysicsLayerDefs GetIgnoredPhysicsLayers()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return 0;

		return data.GetIgnoredPhysicsLayers();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes unwanted types from ignored components list
	int GetIgnoredComponents(notnull out array<typename> outIgnoredComponents)
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return 0;

		return data.GetIgnoredComponents(outIgnoredComponents);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		data.ValidateIgnoredComponents();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns preferred Measurement method
	SCR_ECharacterDistanceMeasurementMethod GetDistanceMeasurementMethod()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return SCR_ECharacterDistanceMeasurementMethod.FROM_EYES;

		return data.GetDistanceMeasurementMethod();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] char
	//! \param[in] destination
	//! \param[in] method
	//! \return
	static float GetDistanceFromCharacter(notnull ChimeraCharacter char, vector destination, SCR_ECharacterDistanceMeasurementMethod method = SCR_ECharacterDistanceMeasurementMethod.FROM_EYES)
	{
		vector pos;
		switch (method)
		{
			case (SCR_ECharacterDistanceMeasurementMethod.FROM_ORIGIN):
			{
				pos = char.GetOrigin();
				break;
			}

			case (SCR_ECharacterDistanceMeasurementMethod.FROM_CENTER_OF_MASS):
			{
				pos = char.AimingPosition(); //Physics cannot be used for characters
				break;
			}

			default:	//SCR_ECharacterDistanceMeasurementMethod.FROM_EYES
			{
				pos = char.EyePosition();
				break;
			}
		}

		return vector.Distance(pos, destination);
	}

	//------------------------------------------------------------------------------------------------
	protected override bool RplSave(ScriptBitWriter writer)
	{
		RplId parentId = RplId.Invalid();
		int nodeId = -1;
		SCR_PlaceableInventoryItemComponent placeableIIC = SCR_PlaceableInventoryItemComponent.Cast(GetOwner().FindComponent(SCR_PlaceableInventoryItemComponent));
		if (placeableIIC)
		{
			parentId = placeableIIC.GetParentRplId();
			nodeId = placeableIIC.GetParentNodeId();
		}
		writer.WriteRplId(parentId);
		if (parentId.IsValid())
			writer.WriteInt(nodeId);

		return super.RplSave(writer);
	}

	//------------------------------------------------------------------------------------------------
	protected override bool RplLoad(ScriptBitReader reader)
	{
		RplId parentId = RplId.Invalid();
		reader.ReadRplId(parentId);
		if (parentId.IsValid())
		{
			int nodeId = -1;
			reader.ReadInt(nodeId);

			SCR_PlaceableInventoryItemComponent placeableIIC = SCR_PlaceableInventoryItemComponent.Cast(GetOwner().FindComponent(SCR_PlaceableInventoryItemComponent));
			if (placeableIIC)
				placeableIIC.SetNewParent(parentId, nodeId);
		}

		return super.RplLoad(reader);
	}
}
