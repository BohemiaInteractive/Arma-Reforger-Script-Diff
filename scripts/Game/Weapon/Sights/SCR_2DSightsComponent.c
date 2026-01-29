[EntityEditorProps(category: "GameScripted/Weapon/Sights", description: "", color: "0 0 255 255")]
class SCR_2DSightsComponentClass : SCR_2DOpticsComponentClass
{
	[Attribute("0", UIWidgets.CheckBox, "Should hide parent of parent object when using 2D sights", category: "2DSights")]
	protected bool m_bShouldHideParentParentObject;

	//------------------------------------------------------------------------------------------------
	bool ShouldHideParentParentObject()
	{
		return m_bShouldHideParentParentObject;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_2DSightsComponent : SCR_2DOpticsComponent
{
	protected SCR_SightsZoomFOVInfo m_SightsFovInfo;
	
	protected static bool s_bIsUsingSights;
	
	//------------------------------------------------------------------------------------------------
	static bool IsUsingSights()
	{
		return s_bIsUsingSights;
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterInputs()
	{
		// Clear previous inputs for safety sake
		UnregisterInputs();

		// Zoom
		GetGame().GetInputManager().AddActionListener(ACTION_ZOOM_IN, EActionTrigger.DOWN, ActionZoomIn);
		GetGame().GetInputManager().AddActionListener(ACTION_ZOOM_OUT, EActionTrigger.DOWN, ActionZoomOut);

		// Illumination
		GetGame().GetInputManager().AddActionListener(ACTION_ILLUMINATION, EActionTrigger.DOWN, ToggleIllumination);
	}

	//------------------------------------------------------------------------------------------------
	protected void UnregisterInputs()
	{
		// Zoom
		GetGame().GetInputManager().RemoveActionListener(ACTION_ZOOM_IN, EActionTrigger.DOWN, ActionZoomIn);
		GetGame().GetInputManager().RemoveActionListener(ACTION_ZOOM_OUT, EActionTrigger.DOWN, ActionZoomOut);

		// Illumination
		GetGame().GetInputManager().RemoveActionListener(ACTION_ILLUMINATION, EActionTrigger.DOWN, ToggleIllumination);
	}

	//------------------------------------------------------------------------------------------------
	protected override void HandleSightActivation()
	{
		super.HandleSightActivation();

		// Set fov
		if (!m_SightsFovInfo)
		{
			m_SightsFovInfo = SCR_SightsZoomFOVInfo.Cast(GetFOVInfo());
			SetupFovInfo();
		}

		SelectZoomLevel(m_iSelectedZoomLevel);
		
		s_bIsUsingSights = true;

		// Set parent
		IEntity owner = GetOwner();
		if (!owner)
			return;

		SCR_2DSightsComponentClass data = SCR_2DSightsComponentClass.Cast(GetComponentData(owner));

		IEntity parent = owner.GetParent();
		if (data && data.ShouldHideParentObject())
		{
			owner.ClearFlags(EntityFlags.VISIBLE, false);
			if (parent && data.ShouldHideParentParentObject())
				parent.ClearFlags(EntityFlags.VISIBLE, false);
		}

		TurretControllerComponent turretController = TurretControllerComponent.Cast(owner.FindComponent(TurretControllerComponent));
		if (turretController)
		{
			BaseCompartmentSlot slot = turretController.GetCompartmentSlot();
			if (slot)
				m_ParentCharacter = ChimeraCharacter.Cast(slot.GetOccupant());
		}
		else
		{
			while (parent)
			{
				m_ParentCharacter = ChimeraCharacter.Cast(parent);
				if (m_ParentCharacter)
					break;

				parent = parent.GetParent();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void HandleSightDeactivation()
	{
		IEntity owner = GetOwner();
		SCR_2DSightsComponentClass data = SCR_2DSightsComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		if (data.ShouldHideParentObject() && owner && !owner.IsDeleted())
		{
			owner.SetFlags(EntityFlags.VISIBLE, true);
			IEntity parent = owner.GetParent();
			if (parent && data.ShouldHideParentParentObject())
				parent.SetFlags(EntityFlags.VISIBLE, true);
		}

		if (data.ShouldHideParentCharacter() && m_ParentCharacter)
		{
			if (!m_ParentCharacter.IsDeleted())
				m_ParentCharacter.SetFlags(EntityFlags.VISIBLE, false);

			m_ParentCharacter = null;
		}
		
		s_bIsUsingSights = false;

		super.HandleSightDeactivation();
	}

	//------------------------------------------------------------------------------------------------
	override void OnSightADSActivated()
	{
		super.OnSightADSActivated();

		RegisterInputs();
	}

	//------------------------------------------------------------------------------------------------
	override void OnSightADSDeactivated()
	{
		super.OnSightADSDeactivated();

		UnregisterInputs();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanFreelook()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected override void ApplyRecoilToCamera(inout vector pOutCameraTransform[4], vector aimModAngles)
	{
		vector weaponAnglesMat[3];
		Math3D.AnglesToMatrix(aimModAngles, weaponAnglesMat);
		Math3D.MatrixMultiply3(pOutCameraTransform, weaponAnglesMat, pOutCameraTransform);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupFovInfo()
	{
		if (!m_SightsFovInfo)
			return;

		// Set zooms
		//m_SightsFovInfo.SetupZooms();
		array<float> zooms = m_SightsFovInfo.ZoomsToArray();

		// Get zooms
		//int zoomCount = m_SightsFovInfo.GetStepsCount();
		int zoomCount = zooms.Count();

		for (int i; i < zoomCount; i++)
		{
			// Calculate fov for each zoom
			float zoom = zooms[i];
			float fov = CalculateZoomFOV(zoom);
			m_SightsFovInfo.InsertFov(fov);

			// Initial setup
			if (i == 0)
				m_SightsFovInfo.SetCurrentFov(fov);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Switch betweem zoom levels from sights
	protected void SelectZoomLevel(int id)
	{
		// Check sights and id
		if (!m_SightsFovInfo)
			return;

		if (id < 0 || id > m_SightsFovInfo.GetStepsCount() - 1)
			return;

		// Set zoom
		m_SightsFovInfo.SetIndex(id);
		m_fMagnification = m_SightsFovInfo.GetCurrentZoom();
	}

	//------------------------------------------------------------------------------------------------
	//! Move zoom level up and down based on mouse wheel value
	protected void ActionZoomIn(float value = 0.0, EActionTrigger reason = 0)
	{
		AdjustZoom(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Move zoom level up and down based on mouse wheel value
	protected void ActionZoomOut(float value = 0.0, EActionTrigger reason = 0)
	{
		AdjustZoom(-1);
	}

	//------------------------------------------------------------------------------------------------
	//! Move zoom level up and down based on mouse wheel value
	protected void AdjustZoom(float value)
	{
		if (!m_SightsFovInfo || value == 0)
			return;

		// TEMPORARY SOLUTION
		// There is a 'mistake' in inheritance, m_pParentCharacter is never set because ADS changes are not propagated (super is not called in SCR_2DPIPSightsComponent)
		SCR_ChimeraCharacter pParentCharacter = null;
		IEntity parent = null;
		IEntity owner = GetOwner();
		if (owner)
		{
			parent = owner.GetParent();

			IEntity parentParent = parent;
			while (parentParent)
			{
				SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(parentParent);
				if (character)
				{
					pParentCharacter = character;
					break;
				}

				parentParent = parentParent.GetParent();
			}
		}

		// Down
		if (value > 0 && m_iSelectedZoomLevel < m_SightsFovInfo.GetStepsCount())
		{
			m_iSelectedZoomLevel++;
			SelectZoomLevel(m_iSelectedZoomLevel);

			if (parent && pParentCharacter)
				pParentCharacter.SetNewZoomLevel(m_iSelectedZoomLevel, true, Replication.FindId(parent.FindComponent(WeaponComponent)));
		}

		// Up
		if (value < 0 && m_iSelectedZoomLevel > 0)
		{
			m_iSelectedZoomLevel--;
			SelectZoomLevel(m_iSelectedZoomLevel);

			if (parent && pParentCharacter)
				pParentCharacter.SetNewZoomLevel(m_iSelectedZoomLevel, false, Replication.FindId(parent.FindComponent(WeaponComponent)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Action for toggling illumination
	protected void ToggleIllumination(float value, EActionTrigger trigger)
	{
		m_bIsIlluminationOn = !m_bIsIlluminationOn;
		EnableReticleIllumination(m_bIsIlluminationOn);

		// TEMPORARY SOLUTION
		// There is a 'mistake' in inheritance, m_pParentCharacter is never set because ADS changes are not propagated (super is not called in SCR_2DPIPSightsComponent)
		SCR_ChimeraCharacter parentCharacter = GetCharacterOwner();
		IEntity soundSource = GetOwner();
		if (!Turret.Cast(soundSource))//turrets have sight component in them and not as an attachment
		{
			IEntity parent = soundSource.GetParent();
			while (parent)
			{
				if (ChimeraCharacter.Cast(parent))
					break;

				soundSource = parent;
				parent = parent.GetParent();
			}
		}

		if (!parentCharacter || !soundSource)
			return;

		RplComponent wpnRplComp = SCR_EntityHelper.GetEntityRplComponent(soundSource);
		if (!wpnRplComp)
			return;

		parentCharacter.SetIllumination(m_bIsIlluminationOn, wpnRplComp.Id());
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ChimeraCharacter GetCharacterOwner()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return null;

		if (Turret.Cast(owner))
		{
			SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));
			if (!compartmentManager || !compartmentManager.AnyCompartmentsOccupiedOrLocked())
				return null;

			array<BaseCompartmentSlot> outCompartments = {};
			if (compartmentManager.GetCompartments(outCompartments) < 1)
				return null;

			foreach (BaseCompartmentSlot compartment: outCompartments)
			{
				if (TurretCompartmentSlot.Cast(compartment))
					return SCR_ChimeraCharacter.Cast(compartment.GetOccupant());
			}
		}
		else
		{
			IEntity parent = owner.GetParent();
			SCR_ChimeraCharacter character;
			while (parent)
			{
				character = SCR_ChimeraCharacter.Cast(parent);
				if (character)
					return character;

				parent = parent.GetParent();
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected override float GetReticleOffsetYTarget()
	{
		SCR_2DOpticsComponentClass data = SCR_2DOpticsComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return 0;

		if (data.GetZeroType() == SCR_EPIPZeroingType.EPZ_RETICLE_OFFSET)
			return m_fReticleOffsetY + GetCurrentSightsRange()[0];

		return m_fReticleOffsetY;
	}

	//------------------------------------------------------------------------------------------------
	// Get Set API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	SCR_SightsZoomFOVInfo GetSightsFovInfo()
	{
		return m_SightsFovInfo;
	}
}
