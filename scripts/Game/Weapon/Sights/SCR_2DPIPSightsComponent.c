[EntityEditorProps(category: "GameScripted/Weapon/Sights", description: "", color: "0 0 255 255")]
class SCR_2DPIPSightsComponentClass : SCR_2DSightsComponentClass
{
	// PIP -----------------------------
	[Attribute("{EF091399D840192D}UI/layouts/Sights/PictureInPictureSightsLayout.layout", UIWidgets.ResourcePickerThumbnail, "The layout used for the PIP component", params: "layout", category: "PiPSights")]
	protected ResourceName m_sPIPLayoutResource;

	[Attribute("RTTexture0", UIWidgets.EditBox, "Name of RTTexture widget within provided layout\n", category: "PiPSights")]
	protected string m_sRTTextureWidgetName;

	[Attribute("RenderTarget0", UIWidgets.EditBox, "Name of RenderTarget widget within provided layout\n", category: "PiPSights")]
	protected string m_sRTargetWidgetName;

	[Attribute("8", UIWidgets.Slider, "Camera index used for this PIP component\n", params: "0 31 1", category: "PiPSights")]
	protected int m_iCameraIndex;

	[Attribute("0.2", UIWidgets.Slider, "Camera near clipping plane\n[m]", params: "0 1000 0.01", category: "PiPSights")]
	protected float m_fNearPlane;

	[Attribute("0", UIWidgets.Slider, "Camera far clipping plane. Set to zero to match maximum view distance\n[m]", params: "0 10000 1", category: "PiPSights")]
	protected float m_fFarPlane;

	// Point info?
	[Attribute("1.0", UIWidgets.Slider, "Scale of resolution used by the PIP\n[x]", params: "0.1 1 0.1", category: "PiPSights")]
	protected float m_fResolutionScale;

	[Attribute("{972DF18CB9BFCBD4}Common/Postprocess/HDR_ScopePiP.emat", UIWidgets.EditBox, "HDR material resource\n", params: "emat", category: "PiPSights")]
	protected ResourceName m_rScopeHDRMatrial;

	// 0 = immediately when entering ADS camera, 1.0 = only after full blend
	[Attribute("0.25", UIWidgets.Slider, "Percentage of camera transition at which sights activate\n[%]", params: "0.0 1.0 0.01", category: "PiPSights")]
	protected float m_fADSActivationPercentagePIP;

	// 0 = immediately when leaving ADS camera, 1.0 = only after full blend
	[Attribute("0.75", UIWidgets.Slider, "Percentage of camera transition at which sights deactivate\n[%]", params: "0.0 1.0 0.01", category: "PiPSights")]
	protected float m_fADSDeactivationPercentagePIP;

	[Attribute("100", UIWidgets.Slider, "Vignette parallax angle scale, divided by objective FOV.\n[x]", params: "-1000 1000 0.1", category: "PiPSights-Parallax")]
	protected float m_fVignetteParallaxScale;

	[Attribute("0.014", UIWidgets.Slider, "Optimal distance from scope, where no parallax distant effect is present\n[m]", params: "-0.2 0.2 0.001", category: "PiPSights-Parallax")]
	protected float m_fCenterDistance;

	[Attribute("0.8", UIWidgets.Slider, "How the parallax is affected when the eye moves closer to scope\n[x]", params: "0 1 0.001", category: "PiPSights-Parallax")]
	protected float m_fDistanceMoveNear;

	[Attribute("0.4", UIWidgets.Slider, "How the parallax is affected when the eye moves farther from scope\n[x]", params: "0 1 0.001", category: "PiPSights-Parallax")]
	protected float m_fDistanceMoveFar;

	[Attribute("0.5", UIWidgets.Slider, "Basic value for simulating parallax effect\n", params: "0 2 0.001", category: "PiPSights-Parallax")]
	protected float m_fBasicParallax;

	[Attribute("1.75", UIWidgets.Slider, "Max parallax value for simulating parallax effect\nShould be bigger than BasicParallax\n", params: "0 2 0.001", category: "PiPSights-Parallax")]
	protected float m_fMaxParallax;

	[Attribute("0.0", UIWidgets.Slider, "Parallax axis offset in X\n[°]", params: "-90 90 0.001", category: "PiPSights-Parallax")]
	protected float m_fCenterOffsetX;

	[Attribute("0.0", UIWidgets.Slider, "Parallax axis offset in Y\n[°]", params: "-90 90 0.001", category: "PiPSights-Parallax")]
	protected float m_fCenterOffsetY;

	[Attribute("1.05", UIWidgets.Slider, "PIP reticle additional scale to compensate discrepancy between camera and reticle\n[x]", params: "0.01 10 0.00001", category: "PiPSights", precision: 5)]
	protected float m_fReticlePIPScale;

	[Attribute("0.98", UIWidgets.Slider, "PIP objective inner edge. Should be lower than max\n[x]", params: "0.001 10 0.00001", category: "PiPSights-Parallax", precision: 5)]
	protected float m_fObjectivePIPEdgeMin;

	[Attribute("1.00", UIWidgets.Slider, "PIP objective outer edge\n[x]", params: "0.001 10 0.00001", category: "PiPSights-Parallax", precision: 5)]
	protected float m_fObjectivePIPEdgeMax;

	[Attribute("0 0 0", UIWidgets.Slider, "Offset when not unfocused to improve view\n[m]", params: "-1 1 0.001", category: "PiPSights")]
	protected vector m_vMainCameraOffsetUnfocused;

	[Attribute("{5366CEDE2A151631}Terrains/Common/Water/UnderWater/oceanUnderwater.emat", UIWidgets.ResourcePickerThumbnail, "Underwater postprocess material\n", params: "emat", category: "PiPSights")]
	protected ResourceName m_sUnderwaterPPMaterial;

	[Attribute("{FA4DE95A7276143D}Common/Postprocess/rain.emat", UIWidgets.ResourcePickerThumbnail, "Rain postprocess material\n", params: "emat", category: "PiPSights")]
	protected ResourceName m_sRainPPMaterial;

	//------------------------------------------------------------------------------------------------
	int GetCameraIndex()
	{
		return m_iCameraIndex;
	}

	//------------------------------------------------------------------------------------------------
	float GetNearPlane()
	{
		return m_fNearPlane;
	}

	//------------------------------------------------------------------------------------------------
	float GetFarPlane()
	{
		return m_fFarPlane;
	}

	//------------------------------------------------------------------------------------------------
	float GetResolutionScale()
	{
		return m_fResolutionScale;
	}

	//------------------------------------------------------------------------------------------------
	float GetADSActivationPercentagePIP()
	{
		return m_fADSActivationPercentagePIP;
	}

	//------------------------------------------------------------------------------------------------
	float GetADSDeactivationPercentagePIP()
	{
		return m_fADSDeactivationPercentagePIP;
	}

	//------------------------------------------------------------------------------------------------
	float GetVignetteParallaxScale()
	{
		return m_fVignetteParallaxScale;
	}

	//------------------------------------------------------------------------------------------------
	float GetCenterDistance()
	{
		return m_fCenterDistance;
	}

	//------------------------------------------------------------------------------------------------
	float GetDistanceMoveNear()
	{
		return m_fDistanceMoveNear;
	}

	//------------------------------------------------------------------------------------------------
	float GetDistanceMoveFar()
	{
		return m_fDistanceMoveFar;
	}

	//------------------------------------------------------------------------------------------------
	float GetBasicParallax()
	{
		return m_fBasicParallax;
	}

	//------------------------------------------------------------------------------------------------
	float GetMaxParallax()
	{
		return m_fMaxParallax;
	}

	//------------------------------------------------------------------------------------------------
	float GetCenterOffsetX()
	{
		return m_fCenterOffsetX;
	}

	//------------------------------------------------------------------------------------------------
	float GetCenterOffsetY()
	{
		return m_fCenterOffsetY;
	}

	//------------------------------------------------------------------------------------------------
	float GetReticlePIPScale()
	{
		return m_fReticlePIPScale;
	}

	//------------------------------------------------------------------------------------------------
	float GetObjectivePIPEdgeMin()
	{
		return m_fObjectivePIPEdgeMin;
	}

	//------------------------------------------------------------------------------------------------
	float GetObjectivePIPEdgeMax()
	{
		return m_fObjectivePIPEdgeMax;
	}

	//------------------------------------------------------------------------------------------------
	vector GetMainCameraOffsetUnfocused()
	{
		return m_vMainCameraOffsetUnfocused;
	}

	//------------------------------------------------------------------------------------------------
	string GetRTTextureWidgetName()
	{
		return m_sRTTextureWidgetName;
	}

	//------------------------------------------------------------------------------------------------
	string GetRTargetWidgetName()
	{
		return m_sRTargetWidgetName;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetPIPLayoutResource()
	{
		return m_sPIPLayoutResource;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetScopeHDRMaterial()
	{
		return m_rScopeHDRMatrial;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetUnderwaterPPMaterial()
	{
		return m_sUnderwaterPPMaterial;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetRainPPMaterial()
	{
		return m_sRainPPMaterial;
	}
	
#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	void DrawSharedDataDiag()
	{
		SCR_2DPIPSightsComponent.InputFloatClamped(m_fCenterDistance, "m_fCenterDistance", -0.2, 0.2);
		SCR_2DPIPSightsComponent.InputFloatClamped(m_fDistanceMoveNear, "m_fDistanceMoveNear", 0.0, 1.0);
		SCR_2DPIPSightsComponent.InputFloatClamped(m_fDistanceMoveFar, "m_fDistanceMoveFar", 0.0, 1.0);
		SCR_2DPIPSightsComponent.InputFloatClamped(m_fBasicParallax, "m_fBasicParallax", 0.0, 2.0);
		SCR_2DPIPSightsComponent.InputFloatClamped(m_fMaxParallax, "m_fMaxParallax", 0.0, 2.0);
		SCR_2DPIPSightsComponent.InputFloatClamped(m_fCenterOffsetX, "m_fCenterOffsetX", -90.0, 90.0);
		SCR_2DPIPSightsComponent.InputFloatClamped(m_fCenterOffsetY, "m_fCenterOffsetY", -90.0, 90.0);
		SCR_2DPIPSightsComponent.InputFloatClamped(m_fReticlePIPScale, "m_fReticlePIPScale", 0.01, 100.0);
		SCR_2DPIPSightsComponent.InputFloatClamped(m_fObjectivePIPEdgeMin, "m_fObjectivePIPEdgeMin", 0.0, 10.0);
		SCR_2DPIPSightsComponent.InputFloatClamped(m_fObjectivePIPEdgeMax, "m_fObjectivePIPEdgeMax", 0.0, 10.0);
		SCR_2DPIPSightsComponent.InputFloatClamped(m_fVignetteParallaxScale, "m_fVignetteParallaxScale", -100.0, 100.0);
	}
#endif
}

//! Defines different means of adjusting zeroing for PIP sights
enum SCR_EPIPZeroingType
{
	EPZ_NONE = 0,
	EPZ_RETICLE_OFFSET = 1,
	EPZ_CAMERA_TURN = 2
}

//------------------------------------------------------------------------------------------------
class SCR_2DPIPSightsComponent : SCR_2DSightsComponent
{
	[Attribute("0.01", UIWidgets.Slider, "Radius of PIP scope ocular\n[m]", params: "0.001 1 0.0001", category: "PiPSights", precision: 5)]
	protected float m_fScopeRadius;

	[Attribute("21", UIWidgets.Slider, "Camera field of view used by this PIP component. Determines LOD used. Set to 0 to use Focus FOV.\n[°]", params: "0 89.99 0.01", category: "PiPSights")]
	protected float m_fMainCameraFOV;

	// 2D -----------------------------
	//! Is this 2D currently enabled?
	protected bool m_b2DIsEnabled;

	protected int m_iLastProjectionFrame = -1;
	protected vector m_vScreenScopeCenter;
	protected float m_fScreenScopeRadiusSq;
	protected float m_fScreenScopeRadius;
	protected float m_fReticleScale;
	protected float m_fEdgeScale = 1;
	protected float m_fParallaxX;
	protected float m_fParallaxY;

	protected IEntity m_ControlledEntity;

	//! Current PIP reticle color
	protected ref Color m_cReticleColor;

	/*!
		Returns whether screen position is in located within the sights radius.
		\param screenPosition Point on screen
		\return Returns true if point is in picture in picture sights, false otherwise.
	*/
	bool IsScreenPositionInSights(vector screenPosition)
	{
		if (!m_PIPCamera || !IsSightADSActive()) // With no active camera or disabled sights, the point is never in PIP
			return false;

		BaseWorld world = GetGame().GetWorld();
		int index = world.GetFrameNumber();

		// Recalculate the projection data only once per frame, so we save some perf on huge queries
		if (m_iLastProjectionFrame != index)
		{
			WorkspaceWidget workspaceWidget = GetGame().GetWorkspace();

			vector sightsRear = GetSightsRearPosition();
			m_vScreenScopeCenter = workspaceWidget.ProjWorldToScreen(sightsRear, world);
			m_vScreenScopeCenter[2] = 0.0;

			vector extent = workspaceWidget.ProjWorldToScreen(sightsRear + GetOwner().GetWorldTransformAxis(0) * m_fScopeRadius, world);
			extent[2] = 0.0;

			const float invSqrt2 = 0.70710678118; // 1.0 / Math.Sqrt(2.0);
			m_fScreenScopeRadiusSq = invSqrt2 * vector.DistanceSq(extent, m_vScreenScopeCenter);
			m_iLastProjectionFrame = index;
		}

		float screenDistanceSq = vector.DistanceSq(screenPosition, m_vScreenScopeCenter);
		return screenDistanceSq < m_fScreenScopeRadiusSq;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calculate the radius of PIP sight in screen coordinates
	//------------------------------------------------------------------------------------------------
	float GetSightsRadiusScreen()
	{
		BaseWorld world = GetGame().GetWorld();

		WorkspaceWidget workspaceWidget = GetGame().GetWorkspace();

		vector sightsRear = GetSightsRearPosition();
		vector scopeScreenCenter = workspaceWidget.ProjWorldToScreen(sightsRear, world);
		scopeScreenCenter[2] = 0;

		vector extent = workspaceWidget.ProjWorldToScreen(sightsRear + GetOwner().GetWorldTransformAxis(0) * m_fScopeRadius, world);
		extent[2] = 0;

		m_fScreenScopeRadius = vector.Distance(extent, scopeScreenCenter);

		return m_fScreenScopeRadius;
	}

	//! The root layout hierarchy widget for PIP
	protected ref Widget m_wPIPRoot;

	//! Is PIP currently enabled?
	protected bool m_bPIPIsEnabled;
	protected static int s_bIsPIPActive;

	//! The render target texture found within our layout
	protected RTTextureWidget m_wRenderTargetTextureWidget;

	//! The render target found within our layout
	protected RenderTargetWidget m_wRenderTargetWidget;

	//! The camera to be used by this pip component
	protected SCR_PIPCamera m_PIPCamera;

	ref Material m_pMaterial;
	int			m_iVignetteCenterXIndex	= -1;
	int			m_iVignetteCenterYIndex	= -1;
	int			m_iVignettePowerIndex	= -1;
	int			m_iLensDistortIndex		= -1;
	int			m_iReticleOffsetXIndex	= -1;
	int			m_iReticleOffsetYIndex	= -1;
	int			m_iReticleColorIndex	= -1;
	int			m_iReticleScaleIndex	= -1;
	int			m_iEdgeMinIndex			= -1;
	int			m_iEdgeMaxIndex			= -1;
	int			m_iEdgeCenterXIndex		= -1;
	int			m_iEdgeCenterYIndex		= -1;

	//------------------------------------------------------------------------------------------------
	/*!
		Returns the camera used for picture in picture mode or null if none.
	*/
	SCR_PIPCamera GetPIPCamera()
	{
		return m_PIPCamera;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns the camera index used by this component when enabled.
	*/
	int GetPIPCameraIndex()
	{
		SCR_2DPIPSightsComponentClass data = SCR_2DPIPSightsComponentClass.Cast(GetComponentData(GetOwner()));
		if (data)
			return data.GetCameraIndex();

		return 8; // 8 as it is the default value for m_iCameraIndex so it might be fairly safe as a default fallback option
	}

	//------------------------------------------------------------------------------------------------
	override float GetADSActivationPercentageScript()
	{
		if (SCR_Global.IsScope2DEnabled())
			return super.GetADSActivationPercentageScript();

		SCR_2DPIPSightsComponentClass data = SCR_2DPIPSightsComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return 1;

		return data.GetADSActivationPercentagePIP();
	}

	//------------------------------------------------------------------------------------------------
	override float GetADSDeactivationPercentageScript()
	{
		if (SCR_Global.IsScope2DEnabled())
			return super.GetADSDeactivationPercentageScript();

		SCR_2DPIPSightsComponentClass data = SCR_2DPIPSightsComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return 0;

		return data.GetADSDeactivationPercentagePIP();
	}

	//------------------------------------------------------------------------------------------------
	bool IsPIPEnabled()
	{
		return m_bPIPIsEnabled;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns true if any sights PIP is active.
	*/
	static bool IsPIPActive()
	{
		return s_bIsPIPActive;
	}

	//------------------------------------------------------------------------------------------------
	float GetMainCameraFOV()
	{
		if (SCR_Global.IsScope2DEnabled())
			return GetFOV();

		if (m_fMainCameraFOV <= 0)
			m_fMainCameraFOV = CalculateZoomFOV(1);

		return GetFocusFOV();
	}

	//------------------------------------------------------------------------------------------------
	protected float GetFocusFOV()
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return m_fMainCameraFOV;

		PlayerCamera camera = PlayerCamera.Cast(cameraManager.CurrentCamera());
		if (camera)
			return Math.Lerp(cameraManager.GetFirstPersonFOV(), m_fMainCameraFOV, camera.GetFocusMode());

		return m_fMainCameraFOV;
	}

	//------------------------------------------------------------------------------------------------
	vector GetMainCameraOffset()
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return vector.Zero;

		PlayerCamera camera = PlayerCamera.Cast(cameraManager.CurrentCamera());
		if (camera)
		{
			SCR_2DPIPSightsComponentClass data = SCR_2DPIPSightsComponentClass.Cast(GetComponentData(GetOwner()));
			if (!data)
				return vector.Zero;

			return vector.Lerp(data.GetMainCameraOffsetUnfocused(), vector.Zero, camera.GetFocusMode());
		}

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsLocalControlledEntity(IEntity pEntity, bool checkHierarchy = true)
	{
		m_ControlledEntity = null;

		if (!pEntity)
			return false;

		// We make sure that we only perform any local items on our local controlled entity
		IEntity controlledEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!controlledEntity)
			return false;

		// Straight hit
		if (controlledEntity == pEntity)
		{
			m_ControlledEntity = controlledEntity;
			return true;
		}

		// Common hierarchy means common root
		if (checkHierarchy && controlledEntity.GetRootParent() == pEntity.GetRootParent())
		{
			m_ControlledEntity = controlledEntity;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param parent Parent entity this camera should be attached to
	//! \param position Local offset from parent entity
	//! \param cameraIndex The index created camera should use, 0-31
	//! \param fov The field of view of camera 0-89
	//! \param nearPlane Near clipping plane of the camera in metres
	//! \param farPlane Far clipping plane of the camera in metres
	protected CameraBase CreateCamera(IEntity parent, vector position, vector angles, int cameraIndex, float fov, float nearPlane, float farPlane)
	{
		// Limit camera's far distance to game's settings.
		farPlane = GetMaxViewDistance(farPlane);
		
		// Spawn camera
		BaseWorld baseWorld = parent.GetWorld();

		// Spawn it close
		EntitySpawnParams params = new EntitySpawnParams();
		parent.GetWorldTransform(params.Transform);
		SCR_PIPCamera pipCamera = SCR_PIPCamera.Cast(GetGame().SpawnEntity(SCR_PIPCamera, baseWorld, params));

		vector mat[4];
		parent = GetCameraLocalTransform(mat);

		pipCamera.SetCameraIndex(cameraIndex);
		pipCamera.SetVerticalFOV(fov);
		pipCamera.SetNearPlane(nearPlane);
		pipCamera.SetFarPlane(farPlane);
		pipCamera.ApplyProps(cameraIndex);
		baseWorld.SetCameraLensFlareSet(cameraIndex, CameraLensFlareSetType.FirstPerson, string.Empty);

		// Set camera to hierarchy
		parent.AddChild(pipCamera, -1, EAddChildFlags.AUTO_TRANSFORM);
		pipCamera.SetLocalTransform(mat);
		pipCamera.UpdatePIPCamera(1.0);
		return pipCamera;
	}

	//------------------------------------------------------------------------------------------------
	protected Widget CreateUI(string layout, string rtTextureName, string rtName, out RTTextureWidget RTTexture, out RenderTargetWidget RTWidget)
	{
		// Empty layout, cannot create any widget
		if (layout == string.Empty)
			return null;

		// Create layout
		Widget root = GetGame().GetWorkspace().CreateWidgets(layout);

		// Layout was not created successfully
		if (!root)
			return null;

		// We dont have required RT widgets, delete layout and terminate
		RTTexture = RTTextureWidget.Cast(root.FindAnyWidget(rtTextureName));
		RTWidget = RenderTargetWidget.Cast(root.FindAnyWidget(rtName));
		if (!RTTexture || !RTWidget)
		{
			root.RemoveFromHierarchy();
			return null;
		}

		return root;
	}

	//------------------------------------------------------------------------------------------------
	void SetPIPEnabled(bool enabled)
	{
		// disabled->enabled
		// Create neccessary items
		if (enabled && !m_bPIPIsEnabled)
		{
			IEntity owner = GetOwner();
			SCR_2DPIPSightsComponentClass data = SCR_2DPIPSightsComponentClass.Cast(GetComponentData(owner));
			if (!data)
				return;

			// Try to create UI for PIP,
			// output params are either set to valid ones,
			// or root itself is set to null and destroyed
			if (!m_wPIPRoot || !m_wRenderTargetTextureWidget || !m_wRenderTargetWidget)
				m_wPIPRoot = CreateUI(data.GetPIPLayoutResource(), data.GetRTTextureWidgetName(), data.GetRTargetWidgetName(), m_wRenderTargetTextureWidget, m_wRenderTargetWidget);

			if (!m_wPIPRoot)
			{
				Print("Could not create PIP layouts!", LogLevel.ERROR);
				return;
			}

			// Create PIP camera
			if (!m_PIPCamera)
			{
				// TODO: restart camera when view distance changes
				float viewDistance = GetGame().GetViewDistance();
				float farPlane = data.GetFarPlane();
				if (farPlane > 0)
					viewDistance = Math.Min(viewDistance, farPlane);

				m_PIPCamera = SCR_PIPCamera.Cast(CreateCamera(owner, GetSightsFrontPosition(true) + data.GetCameraOffset(), data.GetCameraAngles(), data.GetCameraIndex(), GetFOV(), data.GetNearPlane(), viewDistance));
			}

			if (!m_PIPCamera)
			{
				Print("Could not create PIP camera!", LogLevel.ERROR);
				return;
			}

			// Now that everything is ready, we can start observing possible changes of the game settings, so we can react to them
			SCR_MenuHelper.GetOnMenuClose().Insert(OnSettingsMenuClosed);

			const int cameraIndex = data.GetCameraIndex();
			// Set camera index of render target widget
			BaseWorld baseWorld = owner.GetWorld();
			m_wRenderTargetWidget.SetWorld(baseWorld, cameraIndex);

			float resolutionScale = data.GetResolutionScale();
			// Set resolution scale
			m_wRenderTargetWidget.SetResolutionScale(resolutionScale, resolutionScale);

			if (!owner.IsDeleted())
				m_wRenderTargetTextureWidget.SetRenderTarget(owner);

			if (m_pMaterial)
				GetGame().GetWorld().SetCameraPostProcessEffect(cameraIndex, 10, PostProcessEffectType.HDR, data.GetScopeHDRMaterial());

			string material = data.GetUnderwaterPPMaterial();
			if (!material.IsEmpty())
				GetGame().GetWorld().SetCameraPostProcessEffect(cameraIndex, 2, PostProcessEffectType.UnderWater, material);

			material = data.GetRainPPMaterial();
			if (!material.IsEmpty())
				GetGame().GetWorld().SetCameraPostProcessEffect(cameraIndex, 4, PostProcessEffectType.Rain, material);

			s_bIsPIPActive = true;
			m_bPIPIsEnabled = true;
			return;
		}

		// enabled -> disabled
		if (!enabled && m_bPIPIsEnabled)
		{
			Destroy();
			s_bIsPIPActive = false;
			m_bPIPIsEnabled = false;
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DestroyCamera(CameraBase camera)
	{
		if (!camera)
			return;

		IEntity cameraParent = camera.GetParent();
		if (cameraParent)
			cameraParent.RemoveChild(camera);

		SCR_2DPIPSightsComponentClass data = SCR_2DPIPSightsComponentClass.Cast(GetComponentData(GetOwner()));
		if (data)
		{
			const int cameraIndex = data.GetCameraIndex();
			camera.GetWorld().SetCameraPostProcessEffect(cameraIndex, 10, PostProcessEffectType.HDR, string.Empty);
			camera.GetWorld().SetCameraPostProcessEffect(cameraIndex, 2, PostProcessEffectType.UnderWater, string.Empty);
			camera.GetWorld().SetCameraLensFlareSet(cameraIndex, CameraLensFlareSetType.None, string.Empty);
		}

		delete camera;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to limit desired distance to the values set by the game or the server
	//! \param[in] desiredDistance distance that should be evaluated against the game settings
	//! \return maximum possible render distance value
	protected float GetMaxViewDistance(float desiredDistance)
	{
		// Min view distance value allowed by the project
		float absoluteMax = GetGame().GetMaximumViewDistance();
		// Max view distance value allowed by the project
		float absoluteMin = GetGame().GetMinimumViewDistance();
		// Limit camera's far distance to game's settings.
		float serverMax = GetGame().GetViewDistanceServerLimit();

		// Max view distance is enforced by server, then consider that while clamping.
		if (serverMax > 0)
			return Math.Clamp(desiredDistance, absoluteMin, Math.Min(serverMax, absoluteMax));

		return Math.Clamp(desiredDistance, absoluteMin, absoluteMax);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to update PIP properties when settings menu is closed
	//! \param[in] menu which was just closed
	protected void OnSettingsMenuClosed(ChimeraMenuBase menu)
	{
		if (!SCR_SettingsSuperMenu.Cast(menu))
			return;

		if (!m_PIPCamera)
			return;

		float maxViewDistance = GetMaxViewDistance(GetGame().GetViewDistance());
		if (m_PIPCamera.GetFarPlane() != maxViewDistance)
			m_PIPCamera.SetFarPlane(maxViewDistance);
	}

	//------------------------------------------------------------------------------------------------
	// Destroy everything this component created during its lifetime
	protected void Destroy()
	{
		IEntity owner = GetOwner();
		if (m_wRenderTargetTextureWidget && owner && !owner.IsDeleted())
			m_wRenderTargetTextureWidget.RemoveRenderTarget(owner);

		if (m_wPIPRoot)
		{
			m_wPIPRoot.RemoveFromHierarchy();
			m_wPIPRoot = null;
		}

		SCR_MenuHelper.GetOnMenuClose().Remove(OnSettingsMenuClosed);
		DestroyCamera(m_PIPCamera);
	}

	//------------------------------------------------------------------------------------------------
	//! Toggle between illumination modes
	protected override void EnableReticleIllumination(bool enable)
	{
		SCR_2DOpticsComponentClass data = SCR_2DOpticsComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		super.EnableReticleIllumination(enable);

		if (data.HasIllumination() && enable)
			m_cReticleColor = data.GetReticleIlluminationColor();
		else
			m_cReticleColor = data.GetReticleColor();
	}

	//------------------------------------------------------------------------------------------------
	protected override void HandleSightActivation()
	{
		// A component without an owner shouldnt exist
		IEntity owner = GetOwner();
		if (!owner)
			return;

		// We make sure that we only perform any local items on our local controlled entity
		if (!IsLocalControlledEntity(owner))
			return;

		SCR_2DOpticsComponentClass data = SCR_2DOpticsComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		// Set fov TODO@AS: Copied from base?
		if (!m_SightsFovInfo)
		{
			m_SightsFovInfo = SCR_SightsZoomFOVInfo.Cast(GetFOVInfo());
			SetupFovInfo();
		}

		// Initialize to current zero value
		m_fCurrentReticleOffsetY = GetReticleOffsetYTarget();

		bool scope2d = SCR_Global.IsScope2DEnabled();
		if (!scope2d)
		{
			SetPIPEnabled(true);

			if (m_pMaterial)
				UpdateScopeMaterial();

			// Setup illumination
			if (data.HasIllumination())
				EnableReticleIllumination(m_bIsIlluminationOn);
		}

		s_OnSightsADSChanged.Invoke(true, m_fMainCameraFOV);

		if (scope2d)
		{
			if (m_bPIPIsEnabled)
				SetPIPEnabled(false);

			m_b2DIsEnabled = true;
		}
		else if (!scope2d && m_b2DIsEnabled)
		{
			m_b2DIsEnabled = false;
		}

		if (m_b2DIsEnabled)
			super.HandleSightActivation();
		else
			super.HandleSightDeactivation();
	}

	//------------------------------------------------------------------------------------------------
	protected override void HandleSightDeactivation()
	{
		// A component without an owner shouldnt exist
		IEntity owner = GetOwner();
		if (!owner)
			return;

		if (!m_ControlledEntity)
		{
			if (IsPIPEnabled())
			{
				// Make sure to revert overlay camera to default
				if (m_PIPCamera)
				{
					auto camMgr = GetGame().GetCameraManager();
					if (camMgr && camMgr.GetOverlayCamera() == m_PIPCamera)
						camMgr.SetOverlayCamera(null);
				}

				SetPIPEnabled(false);
			}

			return;
		}

		s_OnSightsADSChanged.Invoke(false, 0);

		bool scope2d = SCR_Global.IsScope2DEnabled();
		if (scope2d)
		{
			super.HandleSightDeactivation();
			m_b2DIsEnabled = false;
		}
		else
		{
			SetPIPEnabled(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdateCamera(float timeSlice)
	{
		if (!m_PIPCamera)
			return;

		if (m_fScopeRadius <= 0)
			return;

		// Compute FOV scale based on radius of the scope
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return;

		CameraBase mainCamera = cameraManager.CurrentCamera();
		if (!mainCamera)
			return;

		// Get camera to sight matrix
		vector cameraToSightMat[4];
		SCR_EntityHelper.GetRelativeLocalTransform(GetOwner(), mainCamera, cameraToSightMat);
		vector localCameraPosition = cameraToSightMat[3]; // Camera's position in sight local space

		vector pipSurfaceCenter = GetSightsRearPosition(true);
		float distance = vector.Distance(pipSurfaceCenter, localCameraPosition);
		if (distance == 0)
			return;

		// Get base FOV for camera, zoom levels can vary
		float fov;
		SightsFOVInfo fovInfo = GetFOVInfo();
		if (fovInfo)
		{
			fov = fovInfo.GetFOV();
			SCR_CharacterCameraHandlerComponent.SetOverlayCameraFOV(fov);
		}
		else
		{
			fov = mainCamera.GetVerticalFOV();
		}

		// Account for distance of camera to scope and its diameter
		float scopeSize = Math.Atan2(m_fScopeRadius * 2, distance) * Math.RAD2DEG;
		// PIPCQB Account for focus
		float screenPortion = scopeSize / m_fMainCameraFOV;

		float currentFOV = m_PIPCamera.GetVerticalFOV();
		bool fovChanged = !float.AlmostEqual(fov * screenPortion, currentFOV, currentFOV * 0.005);

		// Smooth adjustment for variable FOV sights
		SCR_BaseVariableSightsFOVInfo variableSights = SCR_BaseVariableSightsFOVInfo.Cast(fovInfo);
		if (variableSights && !fovChanged)
			fovChanged = variableSights.IsAdjusting();

		if (fovChanged)
		{
			m_PIPCamera.SetVerticalFOV(fov * screenPortion);

			// Adjust objective edge field of view
			if (variableSights)
				m_fEdgeScale = fov / variableSights.GetBaseFOV();
		}

		if (fovChanged && m_fReticleAngularSize != 0)
		{
			// Compute reticle base FOV once to be scaled with screenPortion
			if (m_fReticleBaseZoom > 0)
				m_fReticleBaseZoom = -CalculateZoomFOV(m_fReticleBaseZoom);

			float fovReticle;
			if (m_fReticleBaseZoom == 0)
				fovReticle = fov;
			else if (m_fReticleBaseZoom < 0)
				fovReticle = -m_fReticleBaseZoom;

			if (fovReticle > 0)
				m_fReticleScale = fovReticle * screenPortion * m_fReticlePortion / m_fReticleAngularSize;
		}

		// Get local tm in relation to actual parent
		vector mat[4];
		GetCameraLocalTransform(mat);

		// Compute direction to center of ocular
		// Based on eye position to avoid constant errors in case of inaccurate placement of rear sight and eye position
		vector sightsTransform[4];
		GetSightsTransform(sightsTransform, true);

		vector parallaxPoint = sightsTransform[3] + sightsTransform[2] * distance;
		vector parallaxDirection = (parallaxPoint - localCameraPosition).Normalized();
		vector angles = SCR_Math3D.FixEulerVector180(parallaxDirection.VectorToAngles());

		SCR_2DPIPSightsComponentClass data = SCR_2DPIPSightsComponentClass.Cast(GetComponentData(GetOwner()));
		if (data)
			angles += Vector(data.GetCenterOffsetX(), data.GetCenterOffsetY(), 0); // Parallax offset

		// Scale parallax angle with apparent FOV
		angles *= fov / m_fMainCameraFOV;

		// Limit angles again after they are scaled with parallax
		angles = SCR_Math3D.FixEulerVector180(angles);

		// modify camera, reticle and objective with the same angles
		m_fParallaxX = -angles[0];
		m_fParallaxY = angles[1];

		// Modify camera with same angles as reticle
		vector parallax[4];
		Math3D.AnglesToMatrix(angles, parallax);
		Math3D.MatrixMultiply3(mat, parallax, mat);

		// Apply zero angles to default rot
		m_PIPCamera.SetLocalTransform(mat);

		// Finally update camera props
		m_PIPCamera.ApplyTransform(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateHDR()
	{
		int mainCameraIndex = 0;
		CameraManager manager = GetGame().GetCameraManager();
		if (manager)
		{
			CameraBase cam = manager.CurrentCamera();
			if (cam)
				mainCameraIndex = cam.GetCameraIndex();
		}

		IEntity owner = GetOwner();
		//don't forget to disable preexposure on scope material !
		BaseWorld world = owner.GetWorld();
		float hdrBrightness = world.GetCameraHDRBrightness(mainCameraIndex);
		
		SCR_2DPIPSightsComponentClass data = SCR_2DPIPSightsComponentClass.Cast(GetComponentData(owner));
		if (data)
			world.SetCameraHDRBrightness(data.GetCameraIndex(), hdrBrightness);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateScopeMaterial()
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if (!weaponManager)
			return;

		CameraBase mainCamera = cameraManager.CurrentCamera();
		if (!mainCamera)
			return;

		//main camera matrix in local coordinates
		vector mainCam[4];
		mainCamera.GetLocalTransform(mainCam);

		//sights matrix in local coordinates
		vector sightsLSCam[4];
		float fov;

		// if there is no sight, do not continue	
		if (!weaponManager.GetCurrentSightsCameraTransform(sightsLSCam, fov))
			return;

		//position of scope in main camera space
		vector scopePosCam = sightsLSCam[3] - mainCam[3];

		//vectors
		vector scopeFwd = sightsLSCam[2];//m_pOwner.GetWorldTransformAxis(2);
		vector camFwd = mainCam[2];//m_PIPCamera.GetWorldTransformAxis(2);

		//Print(scopeFwd);
		//Print(camFwd);

		//project to reticle plane at scopePosCam, plane = n*P + d = 0 -> scopeFwd dot scopePosLS + d = 0
		//ray = C + s*t = camPos + camFwd*t
		// t = (n*P - n*C) / (n*s)
		//camPos is zero in this case
		float ns = vector.Dot(camFwd, scopeFwd);
		float nP = vector.Dot(scopePosCam, scopeFwd);

		float t = nP/ns;
		vector projP = camFwd*t;

		vector locPos = scopePosCam - projP;

		//final  move of vignette
		// float centerX = locPos[0]*m_fProjectionDifferenceScale + m_fCenterOffsetX;
		// float centerY = locPos[1]*m_fProjectionDifferenceScale + m_fCenterOffsetY;

		SCR_2DPIPSightsComponentClass data = SCR_2DPIPSightsComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		//distance from center distance - base of the eye pos from the projection plane
		//TODO: t is for ARTII weapon negative !
		float distance = Math.AbsFloat(t) - data.GetCenterDistance();

		//change some material setting based on parameters
		float vignettePower = 0;
		//float lensPower     = 0;

		//bigger power = much faster change with distance
		float distancePower;
		if (distance > 0)
		{
			//when we are farther to scope
			distancePower = data.GetDistanceMoveFar();
		}
		else
		{
			//when we are closer to scope
			distancePower = data.GetDistanceMoveNear();
			//lensPower 		= -5.5;
		}

		//stronger at beginning then slower -> some power function, log2 as Math from some reason doesn't contain power function
		float fc = 1 - 1 /(Math.Log2(distancePower*Math.AbsFloat(distance) + 2));

		//max vignette power is 2, let a little opened
		vignettePower = Math.Clamp(fc* 100, data.GetBasicParallax(), data.GetMaxParallax());

		//how is it with lens distortion? now at zero is the -5.5
		//lensPower = lensPower*Math.Clamp(-distance/m_fCenterDistance, 0, 5);


		//modify vignete power also by distance from scope center
		//locPos[2] = 0;
		//const float ProjectionCenterDistanceModif = 140.0;
		//vignettePower += locPos.Length()*ProjectionCenterDistanceModif;

		//Print(centerX);
		//Print(centerY);
		//Print(vignettePower);

		//centerX = Math.Clamp(centerX, -4, 4);
		//centerY = Math.Clamp(centerY, -4, 4);

		// All effects are relative to actual FOV of the PIP surface
		float pipFOV = m_PIPCamera.GetVerticalFOV();

		// Vignette
		float vignetteParallaxScale;
		if (m_fObjectiveFov > 0)
			vignetteParallaxScale = data.GetVignetteParallaxScale() / (m_fObjectiveFov * m_fObjectiveScale);
		
		m_pMaterial.SetParamByIndex(m_iVignetteCenterXIndex, -m_fParallaxX * vignetteParallaxScale / pipFOV);
		m_pMaterial.SetParamByIndex(m_iVignetteCenterYIndex, -m_fParallaxY * vignetteParallaxScale / pipFOV);
		m_pMaterial.SetParamByIndex(m_iVignettePowerIndex, vignettePower);

		float reticlePIPScale = data.GetReticlePIPScale();
		// Objective edge
		m_pMaterial.SetParamByIndex(m_iEdgeCenterXIndex, m_fParallaxX * reticlePIPScale / pipFOV);
		m_pMaterial.SetParamByIndex(m_iEdgeCenterYIndex, m_fParallaxY * reticlePIPScale / pipFOV);
		float edgeSize = m_fObjectiveFov * m_fObjectiveScale * m_fEdgeScale / pipFOV;
		m_pMaterial.SetParamByIndex(m_iEdgeMinIndex, 0.5 * reticlePIPScale * edgeSize * edgeSize * data.GetObjectivePIPEdgeMin());
		m_pMaterial.SetParamByIndex(m_iEdgeMaxIndex, 0.5 * reticlePIPScale * edgeSize * edgeSize * data.GetObjectivePIPEdgeMax());

		// Reticle
		m_pMaterial.SetParamByIndex(m_iReticleScaleIndex, m_fReticleScale / reticlePIPScale);
		m_pMaterial.SetParamByIndex(m_iReticleOffsetXIndex, m_fReticleScale * (m_fParallaxX + m_fReticleOffsetX) / pipFOV);
		m_pMaterial.SetParamByIndex(m_iReticleOffsetYIndex, m_fReticleScale * (m_fParallaxY + m_fCurrentReticleOffsetY) / pipFOV);

		// Reticle color
		float reticleRGBA[4] = {m_cReticleColor.R(), m_cReticleColor.G(), m_cReticleColor.B(), m_cReticleColor.A()};
		m_pMaterial.SetParamByIndex(m_iReticleColorIndex, reticleRGBA);

		//m_pMaterial.SetParamByIndex(m_iLensDistortIndex, lensPower); // disabled for now, set in material directly
	}


	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSPostFrame(IEntity owner, float timeSlice)
	{
		super.OnSightADSPostFrame(owner, timeSlice);


		// Before any updates make sure that we are in target state
		bool use2D = SCR_Global.IsScope2DEnabled();
		if ((m_bPIPIsEnabled && use2D) || (!m_bPIPIsEnabled && !use2D))
		{
			// Reactivate on change
			OnSightADSDeactivated();
			OnSightADSActivated();
		}

		if (m_bPIPIsEnabled)
		{
			UpdateCamera(timeSlice);
			UpdateHDR();

			if (m_pMaterial && m_PIPCamera)
				UpdateScopeMaterial();
		}

		#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_PIP_SIGHTS))
			DrawDiagWindow();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnInit(IEntity owner)
	{
		super.OnInit(owner);

		SCR_2DPIPSightsComponentClass data = SCR_2DPIPSightsComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		m_cReticleColor = data.GetReticleColor();

		ResourceName mat = data.GetScopeHDRMaterial();
		if (mat.IsEmpty())
			Print("Scope HDR material is empty!", LogLevel.WARNING);

		m_pMaterial = Material.GetOrLoadMaterial(mat, 0);

		if (m_pMaterial)
		{
			m_iVignetteCenterXIndex = m_pMaterial.GetParamIndex("VignetteCenterX");
			m_iVignetteCenterYIndex = m_pMaterial.GetParamIndex("VignetteCenterY");
			m_iVignettePowerIndex = m_pMaterial.GetParamIndex("Vignette");
			m_iLensDistortIndex = m_pMaterial.GetParamIndex("LensDistort");
			m_iReticleOffsetXIndex = m_pMaterial.GetParamIndex("ReticleOffsetX");
			m_iReticleOffsetYIndex = m_pMaterial.GetParamIndex("ReticleOffsetY");
			m_iReticleColorIndex = m_pMaterial.GetParamIndex("ReticleColor");
			m_iReticleScaleIndex = m_pMaterial.GetParamIndex("ReticleScale");
			m_iEdgeMinIndex = m_pMaterial.GetParamIndex("EdgeMin");
			m_iEdgeMaxIndex = m_pMaterial.GetParamIndex("EdgeMax");
			m_iEdgeCenterXIndex = m_pMaterial.GetParamIndex("EdgeCenterX");
			m_iEdgeCenterYIndex = m_pMaterial.GetParamIndex("EdgeCenterY");
		}
		else
		{
			Print("Cannot initialize PiP HDR PP !", LogLevel.ERROR);
		}
	}


	//------------------------------------------------------------------------------------------------
	protected override void ApplyRecoilToCamera(inout vector pOutCameraTransform[4], vector aimModAngles)
	{
		if (m_bPIPIsEnabled)
			return;

		vector weaponAnglesMat[3];
		Math3D.AnglesToMatrix(aimModAngles, weaponAnglesMat);
		Math3D.MatrixMultiply3(pOutCameraTransform, weaponAnglesMat, pOutCameraTransform);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanFreelook()
	{
		if (m_bPIPIsEnabled)
			return true;

		return super.CanFreelook();
	}

	#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntity owner, IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}

	//------------------------------------------------------------------------------------------------
	protected override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		const ShapeFlags shapeFlags = ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;
		const float axisLength = 0.1;

		vector ownerTransform[4];
		owner.GetWorldTransform(ownerTransform);

		SCR_2DOpticsComponentClass data = SCR_2DOpticsComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		vector objectiveTransform[4];
		Math3D.AnglesToMatrix(data.GetCameraAngles(), objectiveTransform);
		Math3D.MatrixMultiply3(objectiveTransform, ownerTransform, objectiveTransform);
		objectiveTransform[3] = (GetSightsFrontPosition(true) + data.GetCameraOffset()).Multiply4(ownerTransform);

		vector objectiveSide = objectiveTransform[0];
		vector objectiveUp = objectiveTransform[1];
		vector objectiveForward = objectiveTransform[2];
		vector objectiveCenter = objectiveTransform[3];

		Shape rightArrow = Shape.CreateArrow(objectiveCenter, objectiveCenter + objectiveSide * axisLength, 0.01, Color.DARK_RED, shapeFlags);
		Shape upArrow = Shape.CreateArrow(objectiveCenter, objectiveCenter + objectiveUp * axisLength, 0.01, Color.DARK_GREEN, shapeFlags);
		Shape fwdArrow = Shape.CreateArrow(objectiveCenter, objectiveCenter + objectiveForward * axisLength, 0.01, Color.DARK_BLUE, shapeFlags);

		// Flip the transform for objective circle drawing
		objectiveTransform[1] = objectiveForward;
		objectiveTransform[2] = objectiveUp;
		objectiveTransform[3] = objectiveCenter;
		SCR_Shape.DrawCircle(objectiveTransform, m_fScopeRadius, Color.DARK_RED, COLOR_YELLOW_A, shapeFlags);

		// Draw ocular circle
		vector ocularTransform[4];
		ocularTransform[0] = ownerTransform[0];
		ocularTransform[1] = ownerTransform[2];
		ocularTransform[2] = ownerTransform[1];
		ocularTransform[3] = GetSightsRearPosition(true).Multiply4(ownerTransform);

		SCR_Shape.DrawCircle(ocularTransform, m_fScopeRadius, Color.DARK_RED, COLOR_YELLOW_A, shapeFlags);

		// Draw ocular cross
		vector ocularHorizontal[2];
		vector ocularSide = ownerTransform[0] * m_fScopeRadius;
		ocularHorizontal[0] = ocularTransform[3] + ocularSide;
		ocularHorizontal[1] = ocularTransform[3] - ocularSide;

		vector ocularVertical[2];
		vector ocularUp = ownerTransform[1] * m_fScopeRadius;
		ocularVertical[0] = ocularTransform[3] - ocularUp;
		ocularVertical[1] = ocularTransform[3] + ocularUp;

		Shape.CreateLines(Color.DARK_RED, shapeFlags, ocularHorizontal, 2);
		Shape.CreateLines(Color.DARK_RED, shapeFlags, ocularVertical, 2);

		// Draw eye position
		vector eyeTransform[4]
		GetSightsTransform(eyeTransform);
		Shape.CreateArrow(eyeTransform[3], ocularTransform[3], m_fScopeRadius* 0.3, Color.DARK_CYAN, shapeFlags);
		Shape.CreateSphere(Color.DARK_CYAN, shapeFlags, eyeTransform[3], 0.001);
	}
	#endif

	#ifdef ENABLE_DIAG
	protected static bool s_PIPDiagRegistered;

	//------------------------------------------------------------------------------------------------
	static void InputFloatClamped(inout float value, string label, float min, float max, int pxWidth = 100)
	{
		DbgUI.InputFloat(label, value, pxWidth);
		value = Math.Clamp(value, min, max);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDiagWindow()
	{
		DbgUI.Begin("2DPIPSights");
		{
			
			SCR_2DPIPSightsComponentClass data = SCR_2DPIPSightsComponentClass.Cast(GetComponentData(GetOwner()));
			if (data)
				data.DrawSharedDataDiag();

			InputFloatClamped(m_fCurrentCameraPitch, "m_fCurrentCameraPitch", -90.0, 90.0);
			InputFloatClamped(m_fScopeRadius, "m_fScopeRadius", -90.0, 90.0);
		}

		DbgUI.End();
	}
	#endif

	//------------------------------------------------------------------------------------------------
	//! Constructor
	void SCR_2DPIPSightsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifdef ENABLE_DIAG
		if (!s_PIPDiagRegistered)
		{
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_PIP_SIGHTS,"","Show PIP settings diag","Weapons");
			s_PIPDiagRegistered = true;
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Destructor
	void ~SCR_2DPIPSightsComponent()
	{
		Destroy();
		if (m_pMaterial)
		{
			m_pMaterial = null;
		}

		/*
			This should prevent some static leakness.
		*/
		if (IsPIPEnabled())
			SetPIPEnabled(false);
	}
}
