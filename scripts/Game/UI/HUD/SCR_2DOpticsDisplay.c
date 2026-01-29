void On2DOpticsADSChange(SCR_2DOpticsComponent comp, bool inADS);
typedef func On2DOpticsADSChange;

void On2DOpticsIlluminationChange(Color color, Color glowColor);
typedef func On2DOpticsIlluminationChange;

class SCR_2DOpticsDisplay : SCR_InfoDisplayExtended
{
	protected bool m_bActivated;
	protected SCR_2DOpticsComponent m_Optic;

	protected ResourceName m_sReticleTexture;
	protected ResourceName m_sReticleGlowTexture;
	protected ResourceName m_sFilterTexture;

	// Optics data
	protected float m_fOpticBaseFov;
	protected float m_fOpticCurrentFov;
	protected float m_fReticleSize;
	protected float m_fReticleAngularSize;
	protected float m_fReticlePortion;
	protected float m_fReticleBaseZoom;
	protected float m_fReticleOffsetX;
	protected float m_fReticleOffsetY;
	protected float m_fVignetteScale;
	protected float m_fVignetteSize;
	protected float m_fVignetteMoveSpeed;
	protected float m_fObjectiveFov;
	protected float m_fObjectiveScale;
	protected float m_fObjectiveSize;
	protected float m_fMisalignmentScale;
	protected float m_fMisalignmentDampingSpeed;
	protected float m_fRotationScale;
	protected float m_fRotationDampingSpeed;
	protected float m_fMovementScale;
	protected float m_fMovementDampingSpeed;
	protected float m_fRollScale;
	protected float m_fRollDampingSpeed;
	protected float m_fRollOffset;

	protected vector m_vMisalignmentOffset;
	protected vector m_vObjectiveOffset; // Objective and reticle offset
	protected vector m_vVignetteOffset; // Vignette SetReticleOffset
	protected vector m_vRotation;
	protected vector m_vRotationSpeed;
	protected vector m_vRotationOffset;
	protected vector m_vMovement;
	protected vector m_vMovementSpeed;
	protected vector m_vMovementOffset;

	protected bool m_bInitialBlurOver;

	// Default widget names
	const string WIDGET_LAYOUT_REAR = "HRearEyePiece";
	const string WIDGET_LAYOUT_FRONT = "HFrontObjective";
	const string WIDGET_IMAGE_COVER = "ImgCover";

	const string WIDGET_SIZE_REAR = "SizeRear";
	const string WIDGET_SIZE_OBJECTIVE = "SizeFrontObjective";

	const string WIDGET_IMAGE_PADDING_LEFT = "ImgPaddingLeft";
	const string WIDGET_IMAGE_PADDING_RIGHT = "ImgPaddingRight";
	const string WIDGET_IMAGE_PADDING_TOP = "ImgPaddingTop";
	const string WIDGET_IMAGE_PADDING_BOTTOM = "ImgPaddingBottom";

	const string WIDGET_OVERLAY_RETICLES = "OverlayReticles";
	const string WIDGET_IMAGE_RETICLE = "ImgReticle";
	const string WIDGET_IMAGE_RETICLE_GLOW = "ImgReticleGlow";
	const string WIDGET_IMAGE_VIGNETTE = "ImgVignette";
	const string WIDGET_IMAGE_SCRATCHES = "ImgScratches";
	const string WIDGET_BLUR = "Blur";
	const string WIDGET_IMAGE_FILTER = "ImgFilter";

	const string WIDGET_SIZE_LEFT = "SizePaddingLeft";
	const string WIDGET_SIZE_TOP = "SizePaddingTop";

	const float OPACITY_INITIAL = 0.75;

	protected Widget m_wLayoutRear;
	protected Widget m_wLayoutFront;
	protected ImageWidget m_wImgCover;
	protected ImageWidget m_wImgScratches;

	protected Widget m_wOverlayReticles;
	protected ImageWidget m_wImgReticle;
	protected ImageWidget m_wImgReticleGlow;
	protected BlurWidget m_wBlur;
	protected ImageWidget m_wImgFilter;

	protected SizeLayoutWidget m_wRearFillLeft;
	protected SizeLayoutWidget m_wRearFillTop;
	protected SizeLayoutWidget m_wFrontFillLeft;
	protected SizeLayoutWidget m_wFrontFillTop;

	protected SizeLayoutWidget m_wSizeLayoutRear;
	protected SizeLayoutWidget m_wSizeLayoutObjective;

	//------------------------------------------------------------------------------------------------
	protected void OnLayoutChanged(SCR_HUDLayout newLayout, SCR_HUDLayout oldLayout, SCR_HUDManagerComponent hudManager)
	{
		if (!m_bActivated)
			DeactivateWidget();
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		m_wRoot.SetVisible(false);

		FindWidgets();

		SCR_2DOpticsComponent.s_On2DOpticADSChanged.Insert(OnADSChange);

		SCR_HUDManagerLayoutHandler layoutHandler = SCR_HUDManagerLayoutHandler.Cast(m_HUDManager.FindHandler(SCR_HUDManagerLayoutHandler));
		if (layoutHandler)
			layoutHandler.GetOnLayoutChange().Insert(OnLayoutChanged);

#ifdef ENABLE_DIAG
		if (!s_bOpticDiagRegistered)
		{
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_OPTICS_USE_2D, "", "Toggle 2D optics", "Weapons");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_OPTICS, "", "Show optics diag", "Weapons");
			s_bOpticDiagRegistered = true;
		}
#endif
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayStopDraw(IEntity owner)
	{
		SCR_2DOpticsComponent.s_On2DOpticADSChanged.Remove(OnADSChange);

		if (!m_Optic)
			return;

		m_Optic.OnSetupOpticImage().Remove(SetupOpticImage);
		m_Optic.OnIlluminationChange().Remove(ChangeReticleTint);
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayUpdate(IEntity owner, float timeSlice)
	{
#ifdef ENABLE_DIAG
		if (s_bToggle2DOptics != DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_OPTICS_USE_2D))
		{
			s_bToggle2DOptics = !s_bToggle2DOptics;
			SetScope2DEnabled(!SCR_Global.IsScope2DEnabled());
		}

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_OPTICS))
			DebugOptics();
#endif

		if (!m_Optic)
			return;

		if (!m_bInADS && !SCR_BinocularsComponent.IsZoomedView() && !m_Optic.IsSightADSActive())
			return;

		// Widget check
		if (!m_wLayoutRear || !m_wLayoutFront)
			return;

		UpdateCurrentFov();

		MoveScopeWidgets(timeSlice);
		LowerBlurIntensity(timeSlice, m_Optic.GetAnimationSpeedBlur());

		m_fReticleOffsetX = m_Optic.GetReticleOffsetX();
		m_fReticleOffsetY = m_Optic.GetCurrentReticleOffsetY();
		SetReticleOffset(m_fReticleOffsetX, m_fReticleOffsetY);
	}

	//------------------------------------------------------------------------------------------------
	//! Find widget refences
	protected void FindWidgets()
	{
		// Parts
		m_wLayoutRear = m_wRoot.FindAnyWidget(WIDGET_LAYOUT_REAR);
		m_wLayoutFront = m_wRoot.FindAnyWidget(WIDGET_LAYOUT_FRONT);
		m_wImgCover = ImageWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_IMAGE_COVER));
		m_wImgScratches = ImageWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_IMAGE_SCRATCHES));

		if (!m_wLayoutRear || !m_wLayoutFront)
		{
			Print("Scope vignette movement is not possible due to missing widget references!", LogLevel.WARNING);
			return;
		}

		// Rear part - eye piece
		m_wSizeLayoutRear = SizeLayoutWidget.Cast(m_wLayoutRear.FindAnyWidget(WIDGET_SIZE_REAR));
		m_wRearFillLeft = SizeLayoutWidget.Cast(m_wLayoutRear.FindAnyWidget(WIDGET_SIZE_LEFT));
		m_wRearFillTop = SizeLayoutWidget.Cast(m_wLayoutRear.FindAnyWidget(WIDGET_SIZE_TOP));

		// Front part - objective
		m_wSizeLayoutObjective = SizeLayoutWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_SIZE_OBJECTIVE));
		m_wFrontFillLeft = SizeLayoutWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_SIZE_LEFT));
		m_wFrontFillTop = SizeLayoutWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_SIZE_TOP));

		m_wOverlayReticles = m_wLayoutFront.FindAnyWidget(WIDGET_OVERLAY_RETICLES);
		m_wImgReticle = ImageWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_RETICLE));
		m_wImgReticleGlow = ImageWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_RETICLE_GLOW));
		m_wBlur = BlurWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_BLUR));
		m_wImgFilter = ImageWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_FILTER));
	}

	//------------------------------------------------------------------------------------------------
	//! Setup images and right texture sizing
	void SetupOpticImage()
	{
		// Setup reticle
		if (!m_sReticleTexture.IsEmpty())
			m_wImgReticle.LoadImageTexture(0, m_sReticleTexture);

		m_wImgReticleGlow.SetVisible(!m_sReticleTexture.IsEmpty());

		if (!m_sReticleGlowTexture.IsEmpty())
			m_wImgReticleGlow.LoadImageTexture(0, m_sReticleGlowTexture);

		m_wImgReticleGlow.SetVisible(!m_sReticleGlowTexture.IsEmpty());

		// Setup lens filter
		if (!m_sFilterTexture.IsEmpty())
			m_wImgFilter.LoadImageTexture(0, m_sFilterTexture);

		m_wImgFilter.SetVisible(!m_sFilterTexture.IsEmpty());

		UpdateCurrentFov(true);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateCurrentFov(bool forceUpdate = false)
	{
		float fov;
		if (m_Optic)
		{
			// for binoculars getFOV is wrong
			SCR_FixedFOVInfo fixedFOVInfo = SCR_FixedFOVInfo.Cast(m_Optic.GetFOVInfo());
			if (fixedFOVInfo)
				fov = m_Optic.GetFovZoomed();
			else
				fov = m_Optic.GetFOV();
		}
		else
		{
			CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
			if (camera)
				fov = camera.GetVerticalFOV();
		}

		if (!float.AlmostEqual(m_fOpticCurrentFov, fov))
			m_fOpticCurrentFov = fov;
		else if (!forceUpdate)
			return;

		UpdateReticleSize();
	}

	//------------------------------------------------------------------------------------------------
	//! Setup images and right texture sizing
	void UpdateReticleSize()
	{
		// Compute reticle base FOV once
		if (m_fReticleBaseZoom > 0)
			m_fReticleBaseZoom = -m_Optic.CalculateZoomFOV(m_fReticleBaseZoom);

		float fovReticle;
		if (m_fReticleBaseZoom == 0)
			fovReticle = m_fOpticCurrentFov;
		else
			fovReticle = -m_fReticleBaseZoom;

		// Account for part that represents the measurable reticle
		float reticleAngularSize;
		if (m_fReticlePortion > 0)
			reticleAngularSize = m_fReticleAngularSize / m_fReticlePortion;

		if (fovReticle > 0)
			m_fReticleSize = reticleAngularSize / fovReticle;
		else if (m_fOpticBaseFov > 0)
			m_fReticleSize = reticleAngularSize / m_fOpticBaseFov;

		UpdateScale(1, 1, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnADSChange(notnull SCR_2DOpticsComponent comp, bool inADS)
	{
		if (m_Optic != comp)
		{
			// Clear previous optic invokers as well
			if (m_Optic)
			{
				m_Optic.OnSetupOpticImage().Remove(SetupOpticImage);
				m_Optic.OnIlluminationChange().Remove(ChangeReticleTint);
			}

			m_Optic = comp;

			GetOpticData();
		}

		if (inADS)
		{
			m_Optic.OnSetupOpticImage().Insert(SetupOpticImage);
			m_Optic.OnIlluminationChange().Insert(ChangeReticleTint);
		}
		else
		{
			m_Optic.OnSetupOpticImage().Remove(SetupOpticImage);
			m_Optic.OnIlluminationChange().Remove(ChangeReticleTint);
			Deactivate();
			return;
		}

		m_wRoot.SetZOrder(-1);
		m_wRoot.SetVisible(false);

		m_bInitialBlurOver = false;

		m_vMisalignmentOffset = vector.Zero;

		m_Optic.GetRotation(m_vRotation, 1);
		m_vRotationSpeed = vector.Zero;
		m_vRotationOffset = vector.Zero;

		m_Optic.GetMovement(m_vMovement, 1);

		m_vMovementSpeed = vector.Zero;
		m_vMovementOffset = vector.Zero;

		SetupOpticImage();

		Activate();
	}

	//------------------------------------------------------------------------------------------------
	protected void Activate()
	{
		// Prevent calnceling animation
		AnimateWidget.StopAnimation(m_wRoot, WidgetAnimationOpacity);
		GetGame().GetCallqueue().Remove(DeactivateWidget);

		// Activate and setup hud
		m_wRoot.SetVisible(true);
		m_wImgCover.SetVisible(false);
		m_wRoot.SetOpacity(0);

		// Blur
		m_wBlur.SetIntensity(OPACITY_INITIAL);
		m_wBlur.SetOpacity(1);

		m_bActivated = true;

		int activationDelay;
		int enterTime;
		SCR_2DOpticsComponentClass data = SCR_2DOpticsComponentClass.Cast(m_Optic.GetComponentData(m_Optic.GetOwner()));
		if (data)
		{
			activationDelay = data.GetAnimationActivationDelay();
			enterTime = data.GetAnimationEnterTime();
		}

		//Play fadeIn animation after given delay
		GetGame().GetCallqueue().CallLater(PlayEntryAnimation, activationDelay, false, enterTime);
	}

	//------------------------------------------------------------------------------------------------
	//! Stop any running animation for entering Scope and disable the layout after some delay
	protected void Deactivate()
	{
		m_bActivated = false;

		// Prevent entering animation
		AnimateWidget.StopAnimation(m_wRoot, WidgetAnimationOpacity);
		GetGame().GetCallqueue().Remove(PlayEntryAnimation);

		if (!m_Optic)
			return;
		
		int deactivationDelay;
		SCR_2DOpticsComponentClass data = SCR_2DOpticsComponentClass.Cast(m_Optic.GetComponentData(m_Optic.GetOwner()));
		if (data)
			deactivationDelay = data.GetAnimationDeactivationDelay();

		//Deactivate the HUD after set delay
		GetGame().GetCallqueue().CallLater(DeactivateWidget, deactivationDelay, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Start optics entring animation
	//! \param animationSpeed
	protected void PlayEntryAnimation(float animationSpeed)
	{
		if (animationSpeed > 0)
			AnimateWidget.Opacity(m_wRoot, 1, animationSpeed);
		else
			m_wRoot.SetOpacity(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Open optics hud with animation delay
	protected void DeactivateWidget()
	{
		if (m_wImgCover)
			m_wImgCover.SetOpacity(0);

		if (m_wRoot)
			m_wRoot.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Get optic data for quick retrieval later
	void GetOpticData()
	{
		SCR_2DOpticsComponentClass data = SCR_2DOpticsComponentClass.Cast(m_Optic.GetComponentData(m_Optic.GetOwner()));
		if (!data)
			return;

		data.GetReticleTextures(m_sReticleTexture, m_sReticleGlowTexture, m_sFilterTexture);
		m_Optic.GetReticleData(m_fReticleAngularSize, m_fReticlePortion, m_fReticleBaseZoom);
		m_fOpticBaseFov = m_Optic.GetFovZoomed();
		m_fObjectiveFov = m_Optic.GetObjectiveFov();
		m_fObjectiveScale = m_Optic.GetObjectiveScale();
		m_fVignetteScale = data.GetVignetteScale();
		m_fVignetteMoveSpeed = data.GetVignetteMoveSpeed();
		m_fMisalignmentScale = data.GetMisalignmentScale();
		m_fMisalignmentDampingSpeed = data.GetMisalignmentDampingSpeed();
		m_fRotationScale = data.GetRotationScale();
		m_fRotationDampingSpeed = data.GetRotationDampingSpeed();
		m_fMovementScale = data.GetMovementScale();
		m_fMovementDampingSpeed = data.GetMovementDampingSpeed();
		m_fRollScale = data.GetRollScale();
		m_fRollDampingSpeed = data.GetRollDampingSpeed();

#ifdef ENABLE_DIAG
		s_bDebugOpticsReset = true;
#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Update widget sizes according to the screen height and DPI scale
	void UpdateScale(float reticleScale, float vignetteScale, float objectiveScale)
	{
		float uiScale = 1;
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (workspace)
			uiScale = workspace.DPIUnscale(workspace.GetHeight());

		// Reticle size setup
		if (m_wOverlayReticles)
		{
			float reticleSize = m_fReticleSize * uiScale * reticleScale;
			FrameSlot.SetSize(m_wOverlayReticles, reticleSize, reticleSize);
		}

		// Vignette size setup
		if (m_wSizeLayoutRear)
		{
			m_fVignetteSize = m_fObjectiveFov * m_fVignetteScale * uiScale * vignetteScale / m_fOpticBaseFov;

			m_wSizeLayoutRear.SetWidthOverride(m_fVignetteSize);
			m_wSizeLayoutRear.SetHeightOverride(m_fVignetteSize);
		}

		// Ocular size setup
		if (m_wSizeLayoutObjective)
		{
			m_fObjectiveSize = m_fObjectiveFov * m_fObjectiveScale * uiScale * objectiveScale / m_fOpticBaseFov;

			m_wSizeLayoutObjective.SetWidthOverride(m_fObjectiveSize);
			m_wSizeLayoutObjective.SetHeightOverride(m_fObjectiveSize);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Change reticle texture position with angular offset
	//! \param x x-axis offset value left (-) / right (+)
	//! \param y y-axis offset value down (-) / up (+)
	void SetReticleOffset(float x, float y)
	{
		if (!m_wOverlayReticles)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		float fov = m_fOpticBaseFov;
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (cameraManager)
		{
			CameraBase camera = cameraManager.CurrentCamera();
			if (camera)
				fov = camera.GetVerticalFOV();
		}

		float uiHeight = workspace.DPIUnscale(workspace.GetHeight());
		float offsetX = x * uiHeight / fov;
		float offsetY = y * uiHeight / fov;

		FrameSlot.SetPos(m_wOverlayReticles, offsetX, offsetY);

		vector size = FrameSlot.GetSize(m_wOverlayReticles);
		if (m_wImgReticle)
		{
			float pivotX = 0.5 - (offsetX / size[0]);
			float pivotY = 0.5 - (offsetY / size[1]);
			m_wImgReticle.SetPivot(pivotX, pivotY);
		}

		if (m_wImgReticleGlow)
		{
			float pivotX = 0.5 - (offsetX / size[0]);
			float pivotY = 0.5 - (offsetY / size[1]);
			m_wImgReticleGlow.SetPivot(pivotX, pivotY);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Bring down lens blur intensity in steps
	//! \param timeSlice
	//! \param speed
	protected void LowerBlurIntensity(float timeSlice, float speed)
	{
		if (!m_wBlur)
			return;

		float intensity = m_wBlur.GetIntensity();
		intensity = Math.Lerp(intensity, 0, timeSlice * speed);

		// Clear initial bluer
		if (intensity < 0.1 && !m_bInitialBlurOver)
		{
			m_bInitialBlurOver = true;
			intensity = 0;
		}

		m_wBlur.SetIntensity(intensity);
	}

	//------------------------------------------------------------------------------------------------
	//! Change color of Reticle Image
	//! \param color
	//! \param colorGlow
	protected void ChangeReticleTint(Color color, Color colorGlow)
	{
		if (m_wImgReticleGlow)
			m_wImgReticleGlow.SetColor(colorGlow);

		if (m_wImgReticle)
			m_wImgReticle.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	//! Apply result movement to scope widgets
	protected void MoveScopeWidgets(float timeSlice)
	{
		// Only valid if there is main camera
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return;

		CameraBase camera = cameraManager.CurrentCamera();
		if (!camera)
			return;

		// Only positive FOV allowed
		float fov = camera.GetVerticalFOV();
		if (fov <= 0)
			return;

		// Update vignette and reticle direction and roll
		vector misalignment = m_Optic.GetMisalignmentAngles(camera);

		// Stabilize roll
		float roll = misalignment[2] * m_fRollScale;
		if (m_fRollDampingSpeed > 0)
		{
			float rollSmooth = Math.Min(1, timeSlice * m_fRollDampingSpeed);
			m_fRollOffset = Math.Lerp(m_fRollOffset, roll, rollSmooth);
			roll -= m_fRollOffset;
		}

		// Stabilize binocular reticle over time
		if (m_fMisalignmentDampingSpeed > 0)
		{
			float offsetDamping = Math.Min(timeSlice * m_fMisalignmentDampingSpeed, 1);
			m_vMisalignmentOffset = vector.Lerp(m_vMisalignmentOffset, misalignment, offsetDamping);
			misalignment = misalignment - m_vMisalignmentOffset;
		}

		// Stabilize rotation speed over time
		vector rotation = m_Optic.GetRotation(m_vRotation, timeSlice);
		if (m_fRotationDampingSpeed > 0)
		{
			float rotationDamping = Math.Min(timeSlice * m_fRotationDampingSpeed, 1);
			m_vRotationOffset = vector.Lerp(m_vRotationOffset, rotation, rotationDamping);
			rotation = rotation - m_vRotationOffset;
		}

		// Movement is also stabilizable
		vector movement = m_Optic.GetMovement(m_vMovement, timeSlice);
		if (m_fMovementDampingSpeed > 0)
		{
			float movementDamping = Math.Min(timeSlice * m_fMovementDampingSpeed, 1);
			m_vMovementOffset = vector.Lerp(m_vMovementOffset, movement, movementDamping);
			movement = movement - m_vMovementOffset;
		}

		// Objective must be set instantly
		m_vObjectiveOffset = m_fMisalignmentScale * misalignment;

		// Make sure lerp doesn't go out of bounds
		float vignetteMove = Math.Min(timeSlice * m_fVignetteMoveSpeed, 1);

		m_vVignetteOffset = vector.Lerp(m_vVignetteOffset, m_vObjectiveOffset, vignetteMove);

		if (m_fRotationScale != 0)
			m_vVignetteOffset = m_vVignetteOffset - rotation * m_fRotationScale;

		if (m_fMovementScale != 0)
			m_vVignetteOffset = m_vVignetteOffset - movement * m_fMovementScale;

		// Clamp vignette distance from objective center down to objective size
		vector vignetteObjectiveOffset = m_vVignetteOffset - m_vObjectiveOffset;
		float vignetteDistance = vignetteObjectiveOffset.Length();
		if (vignetteDistance > 0 && vignetteDistance > m_fObjectiveFov * m_fObjectiveScale)
		{
			vignetteObjectiveOffset *= m_fObjectiveFov * m_fObjectiveScale / vignetteDistance;
			m_vVignetteOffset = m_vObjectiveOffset + vignetteObjectiveOffset;
		}

		WorkspaceWidget workspace = GetGame().GetWorkspace();

		float screenW, screenH;
		workspace.GetScreenSize(screenW, screenH);
		screenW = workspace.DPIUnscale(screenW);
		screenH = workspace.DPIUnscale(screenH);

		float pixelsPerDegree = screenH / fov;

		// Objective horizontal movement
		float defaultFrontLeft = LayerDefaultPosition(screenW, m_fObjectiveSize);
		float objectiveX = m_vObjectiveOffset[0] * pixelsPerDegree;
		float frontPaddingLeft = defaultFrontLeft + objectiveX;
		float frontEdgeRight = screenW - m_fObjectiveSize;

		// If objective is passing right edge of the screen, horizontal movement is halved
		if (frontPaddingLeft > frontEdgeRight)
			frontPaddingLeft = frontEdgeRight + (frontPaddingLeft - frontEdgeRight) * 2;

		m_wFrontFillLeft.SetWidthOverride(frontPaddingLeft);

		// Objective vertical movement
		float defaultFrontTop = LayerDefaultPosition(screenH, m_fObjectiveSize);
		float objectiveY = m_vObjectiveOffset[1] * pixelsPerDegree;
		float frontPaddingTop = defaultFrontTop - objectiveY;
		float frontEdgeBottom = screenH - m_fObjectiveSize;

		// If objective is passing bottom edge of the screen, vertical movement is halved
		if (frontPaddingTop > frontEdgeBottom)
			frontPaddingTop = frontEdgeBottom + (frontPaddingTop - frontEdgeBottom) * 2;

		m_wFrontFillTop.SetHeightOverride(frontPaddingTop);

		// Vignette horizontal movement
		float defaultRearLeft = LayerDefaultPosition(screenW, m_fVignetteSize);
		float vignetteX = 2 * objectiveX - m_vVignetteOffset[0] * pixelsPerDegree;
		float rearPaddingLeft = defaultRearLeft + vignetteX;
		float rearEdgeLeft = screenW - m_fVignetteSize;

		// If vignette is passing right edge of the screen, horizontal movement is halved
		if (rearPaddingLeft > rearEdgeLeft)
			rearPaddingLeft = rearEdgeLeft + (rearPaddingLeft - rearEdgeLeft) * 2;

		m_wRearFillLeft.SetWidthOverride(rearPaddingLeft);

		// Vignette vertical movement
		float defaultRearTop = LayerDefaultPosition(screenH, m_fVignetteSize);
		float vignetteY = 2 * objectiveY - m_vVignetteOffset[1] * pixelsPerDegree;
		float rearPaddingTop = defaultRearTop - vignetteY;
		float rearEdgeBottom = screenH - m_fVignetteSize;

		// If vignette is passing bottom edge of the screen, vertical movement is halved
		if (rearPaddingTop > rearEdgeBottom)
			rearPaddingTop = rearEdgeBottom + (rearPaddingTop - rearEdgeBottom) * 2;

		m_wRearFillTop.SetHeightOverride(rearPaddingTop);

		// Reticle rotation
		if (m_wImgReticle)
			m_wImgReticle.SetRotation(roll);

		if (m_wImgReticleGlow)
			m_wImgReticleGlow.SetRotation(roll);

		if (m_wImgFilter)
			m_wImgFilter.SetRotation(roll);

		if (m_wImgCover)
			m_wImgCover.SetRotation(roll);

		if (m_wImgScratches)
			m_wImgScratches.SetRotation(roll + m_Optic.GetScratchesRoll());

		// Motion offset blur
		if (m_bInitialBlurOver)
			MotionBlur(m_vVignetteOffset[0], m_vVignetteOffset[1]);
	}

	//------------------------------------------------------------------------------------------------
	protected void MotionBlur(float posX, float posY)
	{
		if (!m_wBlur)
			return;

		// Check movement
		if (!m_Optic.GetIsMoving() || !m_Optic.GetIsRotating())
			return;

		SCR_2DOpticsComponentClass data = SCR_2DOpticsComponentClass.Cast(m_Optic.GetComponentData(m_Optic.GetOwner()));
		if (!data)
			return;

		// Pick axis
		float intensity = Math.AbsFloat(posX);
		if (intensity > Math.AbsFloat(posY))
			intensity = Math.AbsFloat(posY);

		// Scale and limit
		intensity = intensity * data.GetMotionBlurScale();
		if (intensity > data.GetMotionBlurMax())
			intensity = data.GetMotionBlurMax();

		m_wBlur.SetIntensity(intensity);
	}

	//------------------------------------------------------------------------------------------------
	static float LayerDefaultPosition(float screenSize, float layerSize)
	{
		return (screenSize - layerSize) * 0.5;
	}

#ifdef ENABLE_DIAG
	protected static bool s_bOpticDiagRegistered;
	protected static bool s_bToggle2DOptics;
	protected static bool s_bDebugOptics;
	protected static bool s_bDebugOpticsReset;
	protected static bool s_bDebug2DOptics;
	protected static bool s_bDebug2DOpticsReset;
	protected static bool s_bDebugDrawReset;
	protected static bool s_bDebugDrawCross = true;
	protected static bool s_bDebugDrawNotches = true;
	protected static bool s_bDebugDrawFieldOfView = true;
	protected static bool s_bDebugDrawRandomColors;

	protected static SCR_EOpticsAngleUnits s_eCrossUnit = SCR_EOpticsAngleUnits.RETICLE;
	protected static float s_fCrossUnit = 0.05625;
	protected static float s_fCrossRadius = 100;

	protected static SCR_EOpticsAngleUnits s_eNotchesUnit = SCR_EOpticsAngleUnits.RETICLE;
	protected static float s_fNotchesUnit = 0.05625;
	protected static float s_fNotchesSize = 100;
	protected static float s_fNotchesSmallLength = 5;
	protected static float s_fNotchesSmallSpacing = 5;
	protected static float s_fNotchesLargeLength = 10;
	protected static float s_fNotchesLargeSpacing = 10;

	protected static SCR_EOpticsAngleUnits s_eFieldOfViewUnit = SCR_EOpticsAngleUnits.OBJECTIVE;
	protected static float s_fFieldOfViewUnit = 0.05625;
	protected static float s_fFieldOfViewAngle = 100;

	protected static ref array<int> s_aRandomColors = {COLOR_BLUE, COLOR_RED, COLOR_GREEN, COLOR_YELLOW};
	protected static int s_iLineColor;

	//------------------------------------------------------------------------------------------------
	protected void InputFloatClamped(inout float value, string label, float min, float max, int pxWidth = 100)
	{
		DbgUI.InputFloat(label, value, pxWidth);
		value = Math.Clamp(value, min, max);
	}

	//------------------------------------------------------------------------------------------------
	void DebugOptics()
	{
		if (s_bDebugOpticsReset)
		{
			s_bDebugOpticsReset = false;

			if (!SCR_Global.IsScope2DEnabled())
			{
				SetScope2DEnabled(true);
				s_bToggle2DOptics = !DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_OPTICS_USE_2D);
			}

			return;
		}

		if (s_bDebugDrawReset)
		{
			s_bDebugDrawReset = false;
			s_bDebugDrawCross = true;
			s_bDebugDrawNotches = true;
			s_bDebugDrawFieldOfView = true;
			s_bDebugDrawRandomColors = false;

			s_eCrossUnit = SCR_EOpticsAngleUnits.RETICLE;
			s_fCrossUnit = 0.05625;
			s_fCrossRadius = 100;

			s_eNotchesUnit = SCR_EOpticsAngleUnits.RETICLE;
			s_fNotchesUnit = 0.05625;
			s_fNotchesSize = 100;
			s_fNotchesSmallLength = 5;
			s_fNotchesSmallSpacing = 5;
			s_fNotchesLargeLength = 10;
			s_fNotchesLargeSpacing = 10;

			s_eFieldOfViewUnit = SCR_EOpticsAngleUnits.OBJECTIVE;
			s_fFieldOfViewUnit = 0.05625;
			s_fFieldOfViewAngle = 100;

			return;
		}

		vector sightMat[4];

		if (m_Optic)
		{
			m_Optic.GetSightsTransform(sightMat, true);
			sightMat[3] = m_Optic.GetSightsFrontPosition(true);
			vector ownerMat[4];
			m_Optic.GetOwner().GetWorldTransform(ownerMat);
			Math3D.MatrixMultiply4(ownerMat, sightMat, sightMat);
		}

		// In case of binoculars, side and up vector is expected to match camera
		CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
		if (camera && m_fRollDampingSpeed != 0)
		{
			vector cameraMat[4];
			camera.GetTransform(cameraMat);
			sightMat[0] = cameraMat[0];
			sightMat[1] = cameraMat[1];
		}

		DbgUI.Begin("Optics", 100, 300);

		array<string> units = {};
		SCR_Enum.GetEnumNames(SCR_EOpticsAngleUnits, units);

		//! OPTICS
		{
			DbgUI.Text("________________________________________");
			DbgUI.Check("OPTICS                           ", s_bDebugOptics);
		}

		// Basic reticle and objective FOV settings
		if (s_bDebugOptics)
		{
			{
				InputFloatClamped(m_fObjectiveFov, 				"m_fObjectiveFov           deg", 0, 100);
				InputFloatClamped(m_fObjectiveScale, 				"m_fObjectiveScale           %", 0, 100);
				InputFloatClamped(m_fReticleBaseZoom, 				"m_fReticleBaseZoom          x", -60, 200);
				InputFloatClamped(m_fReticleAngularSize, 			"m_fReticleAngularSize     deg", 0, 100);
				InputFloatClamped(m_fReticlePortion, 				"m_fReticlePortion           %", 0, 100);
				InputFloatClamped(m_fReticleOffsetX, 				"m_fReticleOffsetX         deg", -90, 90);
				InputFloatClamped(m_fReticleOffsetY, 				"m_fReticleOffsetY         deg", -90, 90);
			}

			if (m_Optic && DbgUI.Button("LOAD OPTICS ATTRIBUTES"))
				GetOpticData();

			if (m_Optic && DbgUI.Button("SAVE OPTICS ATTRIBUTES"))
			{
				m_Optic.SetObjectiveFov(m_fObjectiveFov);
				m_Optic.SetObjectiveScale(m_fObjectiveScale);
				m_Optic.SetReticleBaseZoom(m_fReticleBaseZoom);
				m_Optic.SetReticleAngularSize(m_fReticleAngularSize);
				m_Optic.SetReticlePortion(m_fReticlePortion);
				m_Optic.SetReticleOffsetX(m_fReticleOffsetX);
				m_Optic.SetReticleOffsetY(m_fReticleOffsetY);
			}
		}

		//! 2D OPTICS
		{
			DbgUI.Text("________________________________________");
			DbgUI.Check("2D OPTICS                        ", s_bDebug2DOptics);
		}

		// 2D optics features attributes
		if (s_bDebug2DOptics)
		{
			{
				InputFloatClamped(m_fVignetteScale, 				"m_fVignetteScale             ", 0, 100);
				InputFloatClamped(m_fVignetteMoveSpeed, 			"m_fVignetteMoveSpeed         ", -100, 100);
				InputFloatClamped(m_fMisalignmentScale,			"m_fMisalignmentScale         ", -100, 100);
				InputFloatClamped(m_fMisalignmentDampingSpeed,		"m_fMisalignmentDampingSpeed  ", -100, 100);
				InputFloatClamped(m_fRotationScale,				"m_fRotationScale             ", -100, 100);
				InputFloatClamped(m_fRotationDampingSpeed,			"m_fRotationDampingSpeed      ", -100, 100);
				InputFloatClamped(m_fMovementScale,				"m_fMovementScale             ", -100, 100);
				InputFloatClamped(m_fMovementDampingSpeed,			"m_fMovementDampingSpeed      ", -100, 100);
				InputFloatClamped(m_fRollScale,					"m_fRollScale                 ", -100, 100);
				InputFloatClamped(m_fRollDampingSpeed,				"m_fRollDampingSpeed          ", -100, 100);
			}
		}

		// Debug settings
		{
			DbgUI.Text("________________________________________");

			if (DbgUI.Button("RESET DEBUG ATTRIBUTES"))
				s_bDebugDrawReset = true;

			DbgUI.Check("RANDOM LINE COLORS               ", s_bDebugDrawRandomColors);

			if (s_bDebugDrawRandomColors)
				s_iLineColor = s_aRandomColors.GetRandomElement();
			else
				s_iLineColor = COLOR_RED;

			DbgUI.Text(" ");
		}

		//! CROSS
		{
			DbgUI.Text("________________________________________");
			DbgUI.Check("CROSS                            ", s_bDebugDrawCross);
		}

		// Draw basic crosshair matching reticle size
		if (s_bDebugDrawCross)
		{
			{
				DbgUI.Combo("Cross units              ", s_eCrossUnit, units);
				if (s_eCrossUnit == SCR_EOpticsAngleUnits.CUSTOM)
					InputFloatClamped(s_fCrossUnit,				"Custom cross unit         deg", 0, 180);

				InputFloatClamped(s_fCrossRadius, 		"Cross radius                 ", 0, 1000);
			}

			float unit = DebugAngleUnitsToDegrees(s_eCrossUnit, s_fCrossUnit);

			DebugDrawCross(sightMat, s_fCrossRadius * unit);
		}

		//! NOTCHES
		{
			DbgUI.Text("________________________________________");
			DbgUI.Check("NOTCHES                          ", s_bDebugDrawNotches);
		}

		// Draw notches for the angle measurement across reticle
		if (s_bDebugDrawNotches)
		{
			{
				DbgUI.Combo("Notches units            ", s_eNotchesUnit, units);
				if (s_eNotchesUnit == SCR_EOpticsAngleUnits.CUSTOM)
					InputFloatClamped(s_fNotchesUnit,				"Custom notches unit       deg", 0, 180);

				InputFloatClamped(s_fNotchesSize, 				"Size                         ", 0, 100);
				InputFloatClamped(s_fNotchesSmallLength, 		"Small length                 ", 0, 100);
				InputFloatClamped(s_fNotchesSmallSpacing, 		"Small spacing                ", 0, 100);
				InputFloatClamped(s_fNotchesLargeLength, 		"Large length                 ", 0, 100);
				InputFloatClamped(s_fNotchesLargeSpacing, 		"Large spacing                ", 0, 100);
			}

			float unit = DebugAngleUnitsToDegrees(s_eNotchesUnit, s_fNotchesUnit);
			DebugDrawNotches(sightMat, s_fNotchesSmallLength * unit, s_fNotchesSmallSpacing * unit, s_fNotchesSize * unit * 0.5);
			DebugDrawNotches(sightMat, s_fNotchesLargeLength * unit, s_fNotchesLargeSpacing * unit, s_fNotchesSize * unit * 0.5);
		}

		//! FIELD OF VIEW
		{
			DbgUI.Text("________________________________________");
			DbgUI.Check("FIELD OF VIEW                    ", s_bDebugDrawFieldOfView);
		}

		// Draw basic circle and frame
		if (s_bDebugDrawFieldOfView)
		{
			{
				DbgUI.Combo("Field of view units      ", s_eFieldOfViewUnit, units);
				if (s_eFieldOfViewUnit == SCR_EOpticsAngleUnits.CUSTOM)
					InputFloatClamped(s_fFieldOfViewUnit,				"Custom field of view unit deg", 0, 180);

				InputFloatClamped(s_fFieldOfViewAngle, "Field of view angle          ", 0, 1000);
			}

			float unit = DebugAngleUnitsToDegrees(s_eFieldOfViewUnit, s_fFieldOfViewUnit);

			DebugDrawFieldOfView(sightMat, s_fFieldOfViewAngle * unit);
		}

		DbgUI.End();

		if (m_Optic && s_bDebugOptics)
			UpdateCurrentFov(true);

		if (m_Optic && s_bDebug2DOptics)
			UpdateScale(1, 1, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected float DebugAngleUnitsToDegrees(SCR_EOpticsAngleUnits unit, float custom = 0)
	{
		if (unit == SCR_EOpticsAngleUnits.DEGREES)
			return 1;

		if (unit == SCR_EOpticsAngleUnits.RETICLE)
		{
			float reticleAngularSize = m_fReticleAngularSize * 0.01;

			if (m_fReticleBaseZoom < 0)
				reticleAngularSize *= m_fOpticCurrentFov / -m_fReticleBaseZoom;
			else if (m_fReticleBaseZoom > 0)
				reticleAngularSize *= m_fOpticCurrentFov / m_Optic.CalculateZoomFOV(m_fReticleBaseZoom);

			return reticleAngularSize;
		}

		if (unit == SCR_EOpticsAngleUnits.OBJECTIVE)
		{
			float objectiveSize = m_fObjectiveFov * m_fObjectiveScale * 0.01;

			if (m_fOpticBaseFov > 0)
				objectiveSize *= m_fOpticCurrentFov / m_fOpticBaseFov;

			return objectiveSize;
		}

		if (unit == SCR_EOpticsAngleUnits.MILS_WP)
			return 360 / 6000;

		if (unit == SCR_EOpticsAngleUnits.MILS_NATO)
			return 360 / 6400;

		if (unit == SCR_EOpticsAngleUnits.MILS_STRECK)
			return 360 / 6300;

		if (unit == SCR_EOpticsAngleUnits.MILLIRADIANS)
			return Math.RAD2DEG * 0.001;

		return custom;
	}

	//------------------------------------------------------------------------------------------------
	static void DebugDrawFieldOfView(vector sightMat[4], float angle)
	{
		if (float.AlmostEqual(angle, 0))
			return;

		// References
		float targetSize = Math.Tan(Math.DEG2RAD * angle * 0.5);
		float range = 1000;
		CameraBase currentCamera = GetGame().GetCameraManager().CurrentCamera();
		if (currentCamera)
			range = currentCamera.GetFarPlane() * 0.5;

		vector fovMat[4];
		fovMat[0] = sightMat[0];
		fovMat[1] = sightMat[2];
		fovMat[2] = sightMat[1];
		fovMat[3] = sightMat[3] + sightMat[2] * range;

		SCR_Shape.DrawCircle(fovMat, targetSize * range, s_iLineColor, COLOR_YELLOW_A, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE | ShapeFlags.NOZWRITE);
	}

	//------------------------------------------------------------------------------------------------
	protected void DebugDrawNotches(vector sightMat[4], float length, float spacing, float size)
	{
		if (float.AlmostEqual(length, 0))
			return;

		if (float.AlmostEqual(spacing, 0))
			return;

		vector opticSide = sightMat[0];
		vector opticUp = sightMat[1];
		vector opticDir = sightMat[2];
		vector opticPos = sightMat[3];

		// References
		int count = Math.Floor(size / spacing);
		float notchSpacing = Math.DEG2RAD * spacing;
		float notchHalf = Math.Tan(Math.DEG2RAD * length * 0.5);
		float range = 1000;
		CameraBase currentCamera = GetGame().GetCameraManager().CurrentCamera();
		if (currentCamera)
			range = currentCamera.GetFarPlane() * 0.5;

		vector zeroMat[4] = sightMat;
		zeroMat[3] = opticPos + opticDir * range;

		vector notchMat[4] = zeroMat;
		vector notchSide = opticSide * notchHalf * range;
		vector notchUp = opticUp * notchHalf * range;
		vector notch[2];

		// Horizontal markings - left
		notchMat = zeroMat;
		for (int i; i < count; i++)
		{
			SCR_Math3D.RotateAround(notchMat, opticPos, opticUp, -notchSpacing, notchMat);
			notch[0] = notchMat[3] + notchUp;
			notch[1] = notchMat[3] - notchUp;
			Shape.CreateLines(s_iLineColor, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, notch, 2);
		}

		// Horizontal markings - right
		notchMat = zeroMat;
		for (int i; i < count; i++)
		{
			SCR_Math3D.RotateAround(notchMat, opticPos, opticUp, notchSpacing, notchMat);
			notch[0] = notchMat[3] + notchUp;
			notch[1] = notchMat[3] - notchUp;
			Shape.CreateLines(s_iLineColor, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, notch, 2);
		}

		// Vertical markings - up
		notchMat = zeroMat;
		for (int i; i < count; i++)
		{
			SCR_Math3D.RotateAround(notchMat, opticPos, opticSide, -notchSpacing, notchMat);
			notch[0] = notchMat[3] + notchSide;
			notch[1] = notchMat[3] - notchSide;
			Shape.CreateLines(s_iLineColor, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, notch, 2);
		}

		// Vertical markings - down
		notchMat = zeroMat;
		for (int i; i < count; i++)
		{
			SCR_Math3D.RotateAround(notchMat, opticPos, opticSide, notchSpacing, notchMat);
			notch[0] = notchMat[3] + notchSide;
			notch[1] = notchMat[3] - notchSide;
			Shape.CreateLines(s_iLineColor, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, notch, 2);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DebugDrawCross(vector sightMat[4], float radius)
	{
		if (float.AlmostEqual(radius, 0))
			return;

		vector opticSide = sightMat[0];
		vector opticUp = sightMat[1];
		vector opticDir = sightMat[2];
		vector opticPos = sightMat[3];

		// References
		float crossRadius = Math.Tan(Math.DEG2RAD * radius * 0.5);
		float range = 1000;
		CameraBase currentCamera = GetGame().GetCameraManager().CurrentCamera();
		if (currentCamera)
			range = currentCamera.GetFarPlane() * 0.5;

		vector crossPos = opticPos + opticDir * range;
		vector cross[2];

		// Line of sight
		cross[0] = opticPos;
		cross[1] = crossPos;
		Shape.CreateLines(s_iLineColor, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, cross, 2);

		// Horizontal
		vector crossSide = opticSide * crossRadius * range;
		cross[0] = crossPos + crossSide;
		cross[1] = crossPos - crossSide;
		Shape.CreateLines(s_iLineColor, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, cross, 2);

		// Vertical
		vector crossUp = opticUp * crossRadius * range;
		cross[0] = crossPos + crossUp;
		cross[1] = crossPos - crossUp;
		Shape.CreateLines(s_iLineColor, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, cross, 2);

		// Diagonal #1
		cross[0] = crossPos - crossUp - crossSide;
		cross[1] = crossPos + crossUp + crossSide;
		Shape.CreateLines(s_iLineColor, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, cross, 2);

		// Diagonal #2
		cross[0] = crossPos + crossUp - crossSide;
		cross[1] = crossPos - crossUp + crossSide;
		Shape.CreateLines(s_iLineColor, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, cross, 2);
	}

	//------------------------------------------------------------------------------------------------
	static void SetScope2DEnabled(bool enabled)
	{
		BaseContainer gameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		if (gameplaySettings)
			gameplaySettings.Set("m_b2DScopes", enabled);
	}
#endif
}
