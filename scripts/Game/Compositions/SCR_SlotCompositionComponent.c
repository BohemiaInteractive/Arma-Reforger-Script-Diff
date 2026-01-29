[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "")]
class SCR_SlotCompositionComponentClass : ScriptComponentClass
{
	[Attribute(params: "et", category: "Composition", desc: "Slot this composition fits into.")]
	protected ResourceName m_SlotPrefab;
	
	[Attribute(defvalue: "1", category: "Composition", desc: "When enabled, children will be snapped and oriented to terrain when the composition is transformed.")]
	protected bool m_bOrientChildrenToTerrain;
	
	[Attribute(defvalue: "1", category: "Composition", desc: "When enabled, children will not be turned into editable entities. The composition will appear as a single entity in in-game editor.")]
	protected bool m_bEditableChildren; //--- Not read in the script, but accessed by EditablePrefabsConfig when generating editable prefabs
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetSlotPrefab()
	{
		return m_SlotPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool CanOrientChildrenToTerrain()
	{
		return m_bOrientChildrenToTerrain;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get slot prefab from SCR_EditorFactionsAccessComponent source
	//! \param componentSource Component source
	//! \return Slot prefab
	//!
	static ResourceName GetSlotPrefab(IEntityComponentSource componentSource)
	{
		ResourceName slotPrefab;
		componentSource.Get("m_SlotPrefab", slotPrefab);
		return slotPrefab;
	}
};

//! @ingroup Editable_Entities

//! Entity composition which is supposed to fit into a slot
class SCR_SlotCompositionComponent : ScriptComponent
{
	protected GenericEntity m_Owner;
	protected static bool m_bIgnoreOrientChildrenToTerrain;

	//------------------------------------------------------------------------------------------------
	//! Get prefab of the slot to which the composition fits.
	//! \return Slot prefab
	ResourceName GetSlotPrefab()
	{
		SCR_SlotCompositionComponentClass prefabData = SCR_SlotCompositionComponentClass.Cast(GetComponentData(m_Owner));
		if (prefabData)
			return prefabData.GetSlotPrefab();
		else
			return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if composition children should be snapped and oriented to terrain.
	//! \return True if children are affected
	bool CanOrientChildrenToTerrain()
	{
		SCR_SlotCompositionComponentClass prefabData = SCR_SlotCompositionComponentClass.Cast(GetComponentData(m_Owner));
		if (prefabData)
			return prefabData.CanOrientChildrenToTerrain();
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Bool to disable the terrain snapping for next spawned composition
	static void IgnoreOrientChildrenToTerrain()
	{
		m_bIgnoreOrientChildrenToTerrain = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Orient composition and its children to terrain.
	//! Applied only if "Can Orient Children To Terrain" attribute is enabled.
	void OrientToTerrain()
	{
		//--- Don't initialize child compositions (they will be initialized as part of parent's init)
		if (!CanOrientChildrenToTerrain() || m_Owner.GetParent())
			return;
		
		BaseWorld world = m_Owner.GetWorld();
		SetChildTransform(m_Owner, world);
		m_Owner.Update();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetChildTransform(IEntity owner, BaseWorld world)
	{		
		//--- Find entity's height
		float height = 0;
		if (owner.GetParent())
		{
			//--- Use original local height (used only during initial transformation)
			height = owner.GetLocalTransformAxis(3)[1];
		}
		else
		{
			//--- Maintain existing height ATL
			height = SCR_TerrainHelper.GetHeightAboveTerrain(owner.GetOrigin(), world);
		}
		
		//--- Apply transformation
		vector transform[4];
		owner.GetWorldTransform(transform);
		if (owner == m_Owner)
			SCR_TerrainHelper.SnapToTerrain(transform, world); //--- Don't rotate root entity. Its transformation is broadcasted to clients, who would then think the rotated state is default
		else
			SCR_TerrainHelper.SnapAndOrientToTerrain(transform, world);

		transform[3] = transform[3] + Vector(0, height, 0);
		
		//If entity should be horizontally aligned rather then aligned to terrain
		SCR_HorizontalAlignComponent horizontalAlignComponent = SCR_HorizontalAlignComponent.Cast(owner.FindComponent(SCR_HorizontalAlignComponent));
		if (horizontalAlignComponent)
		{
			vector angles = Math3D.MatrixToAngles(transform);
			angles[1] = 0;
			angles[2] = 0;
			Math3D.AnglesToMatrix(angles, transform);
		}
		
#ifdef WORKBENCH
		//--- When in World Editor, apply changes to attributes directly
		if (SCR_Global.IsEditMode(owner))
		{
			WorldEditorAPI api = GenericEntity.Cast(owner)._WB_GetEditorAPI();
			IEntity parent = owner.GetParent();
			vector pos = transform[3];
			if (parent)
			{
				vector parentTransform[3];
				parent.GetWorldTransform(parentTransform);
				Math3D.MatrixInvMultiply3(parentTransform, transform, transform);
				pos = parent.CoordToLocal(pos);
			}
			vector angles = Math3D.MatrixToAngles(transform);
			IEntitySource ownerSrc = api.EntityToSource(owner);
			api.SetVariableValue(ownerSrc, null, "coords", pos.ToString(false));
			api.SetVariableValue(ownerSrc, null, "angles", string.Format("%1 %2 %3", angles[1], angles[0], angles[2]));
		}
		else
		{
			owner.SetWorldTransform(transform);
		}
#else			
		owner.SetWorldTransform(transform);
#endif

		//--- Process children recursively
		SCR_SlotCompositionComponent composition = SCR_SlotCompositionComponent.Cast(owner.FindComponent(SCR_SlotCompositionComponent));
		if (composition && composition.CanOrientChildrenToTerrain())
		{
			IEntity child = owner.GetChildren();
			while (child)
			{
				SetChildTransform(child, world);
				child = child.GetSibling();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;

		if (m_bIgnoreOrientChildrenToTerrain)
		{
			m_bIgnoreOrientChildrenToTerrain = false;
			return;
		}

		//--- Initialize transform on every machine (ToDo: Remove SCR_EditableEntityComponent check; rather, have editable entities suppress this functionality)
		if (!owner.FindComponent(SCR_EditableEntityComponent))
			OrientToTerrain();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_Owner = GenericEntity.Cast(owner);
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] api
	//! \param[in] entitySource
	//! \param[in] isRoot
	//! \param[in] delayed
	//! \return
	static bool _WB_ConfigureComposition(WorldEditorAPI api, IEntitySource entitySource, bool isRoot = true, bool delayed = false)
	{
		//--- Skip sub-compositions
		if (!isRoot && SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_SlotCompositionComponent))
			return false;
		
		bool isChange;
		IEntityComponentSource hierarchySource = SCR_BaseContainerTools.FindComponentSource(entitySource, "Hierarchy");
		if (hierarchySource)
		{
			//--- Has hierarchy, make sure it's enabled
			hierarchySource.Set("Enabled", true);
		}
		else
		{
			//--- Doesn't have hierarchy, add one
			isChange = true;
			if (delayed)
				SCR_SlotCompositionHelperEntity.CreateHierarchy(api, entitySource);
			else
				api.CreateComponent(entitySource, "Hierarchy");
		}
		
		//--- Process children recursively
		for (int i = 0, count = entitySource.GetNumChildren(); i < count; i++)
		{
			isChange |= _WB_ConfigureComposition(api, entitySource.GetChild(i), false);
		}
		
		return isChange;
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref WB_UIMenuItem> _WB_GetContextMenuItems(IEntity owner)
	{
		return { 
			new WB_UIMenuItem("Configure composition", 0),
			new WB_UIMenuItem("Orient to terrain", 1)
		};
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnContextMenu(IEntity owner, int id)
	{
		WorldEditorAPI api = GenericEntity.Cast(owner)._WB_GetEditorAPI();
		switch (id)
		{
			case 0:
			
				//--- Gen entity instance
				IEntitySource entitySource = api.EntityToSource(owner);
				if (!entitySource) break;
			
				//--- Get entity prefab
				entitySource = entitySource.GetAncestor();
				if (!entitySource) break;
				
				//--- Process the prefab
				api.BeginEntityAction();
				_WB_ConfigureComposition(api, entitySource, true, true);
				api.EndEntityAction();
				break;
			case 1:
				api.BeginEntityAction();
				OrientToTerrain();
				api.EndEntityAction();
				break;
		}
	}
#endif
}

#ifdef WORKBENCH
//--- Adding a component from inside a context action causes error, this is a hotfix
class SCR_SlotCompositionHelperEntityClass : GenericEntityClass {}
class SCR_SlotCompositionHelperEntity : GenericEntity
{
	protected IEntitySource m_EntitySource;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] api
	//! \param[in] entitySource
	static void CreateHierarchy(WorldEditorAPI api, IEntitySource entitySource)
	{
		IEntitySource src = api.CreateEntity("SCR_SlotCompositionHelperEntity", "SCR_SlotCompositionHelperEntity", 0, null, vector.Zero, vector.Zero);
		SCR_SlotCompositionHelperEntity helper = SCR_SlotCompositionHelperEntity.Cast(api.SourceToEntity(src));
		helper.m_EntitySource = src;
	}

//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}
	
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		if (!m_EntitySource)
			return;
		
		WorldEditorAPI api = _WB_GetEditorAPI();
		api.BeginEntityAction();
		api.CreateComponent(m_EntitySource, "Hierarchy");
		api.DeleteEntity(m_EntitySource);
		api.EndEntityAction();
	}
}
#endif
