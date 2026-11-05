[EntityEditorProps(category: "GameScripted/Weapon/Sights", description: "", color: "0 0 255 255")]
class SCR_NV2DSightsComponentClass : SCR_2DSightsComponentClass
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "HDR material resource\n", params: "emat", category: "Sights")]
	protected ResourceName m_rScopeHDRMaterial;

	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Grain material resource\n", params: "emat", category: "Sights")]
	protected ResourceName m_rScopeGrainMaterial;

	protected bool m_bMaterialChangesDisabled;

	//! default grain effect material used by AR
	const static ResourceName DEFAULT_GRAIN_EFFECT_MATERIAL = "{BA5DE2A5962C178C}Common/Postprocess/NightNoise.emat";
	//! default HDR material used by AR
	const static ResourceName DEFAULT_HDR_EFFECT_MATERIAL = "{9DEECCABE8357209}Common/Postprocess/HDR.emat";
	//! Priority to which by default assign HDR effect. They are practically an ID of the effect on a list of all effects.
	//! This priority value can be found f.e. on Arland as GenericWorldPPEffect named HDR_8
	const static int HDR_PRIORITY = 16;
	//! Priority to which by default assign grain effect. They are practically an ID of the effect on a list of all effects.
	//! This priority value can be found f.e. on Arland as GenericWorldPPEffect named PP_Noise_9
	const static int FILM_GRAIN_PRIORITY = 19;

	//------------------------------------------------------------------------------------------------
	//! Changes the postprocessing materials used for HDR and Filmgrain effect
	//! \param[in] world for which effects will be changed
	//! \param[in] camIndex id of the camera for which effects will be changed, if not provided then current one will be used
	//! \param[in] useDefaultMaterials true will reset effects to their default HDR and grain materials, otherwise it will use ones from the attributes
	void SetCameraMaterials(notnull BaseWorld world, int camIndex = -1, bool useDefaultMaterials = false)
	{
		if (camIndex < 0)
		{
			camIndex = world.GetCurrentCameraId();
			if (camIndex < 0)
				return;
		}

		if (m_bMaterialChangesDisabled)
			return;

		bool hdrIsEmpty = m_rScopeHDRMaterial.IsEmpty();
		bool grainIsEmpty = m_rScopeGrainMaterial.IsEmpty();
		if (hdrIsEmpty && grainIsEmpty)
		{
			m_bMaterialChangesDisabled = true;
			return;
		}

		if (useDefaultMaterials)
		{
			world.SetCameraPostProcessEffect(camIndex, HDR_PRIORITY, PostProcessEffectType.HDR, DEFAULT_HDR_EFFECT_MATERIAL);
			world.SetCameraPostProcessEffect(camIndex, FILM_GRAIN_PRIORITY, PostProcessEffectType.FilmGrain, DEFAULT_GRAIN_EFFECT_MATERIAL);
		}
		else
		{
			if (!hdrIsEmpty)
				world.SetCameraPostProcessEffect(camIndex, HDR_PRIORITY, PostProcessEffectType.HDR, m_rScopeHDRMaterial);

			if (!grainIsEmpty)
				world.SetCameraPostProcessEffect(camIndex, FILM_GRAIN_PRIORITY, PostProcessEffectType.FilmGrain, m_rScopeGrainMaterial);
		}
	}
}

//------------------------------------------------------------------------------------------------
class SCR_NV2DSightsComponent : SCR_2DSightsComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnSightADSActivated()
	{
		super.OnSightADSActivated();

		UpdateCameraMaterials(false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSightADSDeactivated()
	{
		super.OnSightADSDeactivated();

		UpdateCameraMaterials(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Switches current camera postprocessing effects between default HDR and Grain or ones configured for this component
	//! \param[in] useDefaultMaterials true will use make sure that game sets defualt HDR and grain materials to currently used camera
	protected void UpdateCameraMaterials(bool useDefaultMaterials)
	{
		const IEntity owner = GetOwner();
		const SCR_NV2DSightsComponentClass data = SCR_NV2DSightsComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		const BaseWorld world = owner.GetWorld();
		if (!world)
			return;

		data.SetCameraMaterials(world, useDefaultMaterials: useDefaultMaterials);
	}
}