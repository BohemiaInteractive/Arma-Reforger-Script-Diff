class SCR_PlacedCommandInfoDisplay : SCR_InfoDisplayExtended
{
	[Attribute("1000", desc: "Max distance to command position in meters.")]
	protected float m_fCommandVisualRange;

	[Attribute("0 1 0.5 0", UIWidgets.GraphDialog, params: "1 0.6 0 0.2")]
	protected ref Curve m_aScaleCurve;

	[Attribute("Frame")]
	protected string m_sCommandFrameWidgetName;

	[Attribute("Icon")]
	protected string m_sCommandIconWidgetName;

	[Attribute("Glow")]
	protected string m_sCommandGlowWidgetName;

	[Attribute("Pulse")]
	protected string m_sCommandPulseWidgetName;
	
	[Attribute("PulsePreview")]
	protected string m_sCommandPulsePreviewWidgetName;

	[Attribute("26", desc: "Default Image size of Command Icon Widget.")]
	protected float m_fCommandIconWidgetSize;

	[Attribute("48", desc: "Default Image size of Command Glow Widget.")]
	protected float m_fCommandGlowWidgetSize;
	
	[Attribute("64", desc: "Default Image size of Command Pulse Preview Widget.")]
	protected float m_fCommandPulsePreviewWidgetSize;

	protected bool m_bCanUpdate;
	protected bool m_bCanUpdatePosition;
	protected bool m_bIsPreview;

	protected float m_fDistance;

	protected vector m_vCommandPosition;
	protected WorkspaceWidget m_wWorkspace;

	protected Widget m_wCommandFrameWidget;
	protected ImageWidget m_wCommandIconWidget;
	protected ImageWidget m_wCommandGlowWidget;
	protected ImageWidget m_wCommandPulseWidget;
	protected ImageWidget m_wCommandPulsePreviewWidget;

	protected ref SCR_PhysicsHelper m_PhysicsHelper;

	protected SCR_ECommandVisualizationDuration m_eCommandVisualizationDuration;

	protected ref TraceParam m_TraceParam;

	protected static const int PULSE_WIDGET_MAX_SIZE = 70;

	protected static const float BASE_FOV = 100;
	protected static const float LINE_OF_SIGHT_THRESHOLD = 0.25;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wCommandIconWidget = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sCommandIconWidgetName));
		m_wCommandGlowWidget = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sCommandGlowWidgetName));
		m_wCommandPulseWidget = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sCommandPulseWidgetName));
		m_wCommandPulsePreviewWidget = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sCommandPulsePreviewWidgetName));
		m_wCommandFrameWidget = m_wRoot.FindAnyWidget(m_sCommandFrameWidgetName);
		m_wWorkspace = GetGame().GetWorkspace();

		if (!m_wCommandIconWidget || !m_wCommandGlowWidget || !m_wCommandFrameWidget || !m_wCommandPulseWidget)
			return;

		FrameSlot.SetAnchor(m_wCommandFrameWidget, 0, 0);
		m_wCommandFrameWidget.SetVisible(false);
		m_wCommandFrameWidget.SetOpacity(0);

		m_wCommandPulseWidget.SetVisible(false);
		m_wCommandPulseWidget.SetOpacity(0);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		if (m_wCommandPulseWidget)
			AnimateWidget.StopAllAnimations(m_wCommandPulseWidget);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_bCanUpdate || !m_wCommandFrameWidget)
			return;

		if (m_bCanUpdatePosition)
			UpdatePosition();

		if (m_vCommandPosition == vector.Zero)
			return;

		int screenW = m_wWorkspace.GetWidth();
		int screenH = m_wWorkspace.GetHeight();
		screenW = m_wWorkspace.DPIUnscale(screenW);
		screenH = m_wWorkspace.DPIUnscale(screenH);

		float posX, posY;
		GetWorldToScreenPosition(GetGame().GetWorld(), m_vCommandPosition, posX, posY);

		BaseWorld world = GetGame().GetWorld();
		int cameraIndex = world.GetCurrentCameraId();
		vector transform[4];
		world.GetCamera(cameraIndex, transform);

		if (posX < 0 || posX > screenW || posY < 0 || posY > screenH || !IsInLineOfSight(m_vCommandPosition, transform))
		{
			m_wCommandFrameWidget.SetVisible(false);
			return;
		}
		else
		{
			m_wCommandFrameWidget.SetVisible(true);
		}

		FrameSlot.SetPos(m_wCommandFrameWidget, posX, posY);

		float zoom = 1; // world.GetCameraVerticalFOV(cameraIndex) - missing crucial getter
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (cameraManager)
		{
			CameraBase camera = cameraManager.CurrentCamera();
			if (camera && camera.GetProjType() != CameraType.NONE)
				zoom = BASE_FOV / Math.Max(camera.GetVerticalFOV(), 1);
		}

		vector referencePos = transform[3];
		float distanceLimit = m_fCommandVisualRange * zoom;

		if (!m_bCanUpdatePosition)
		{
			float distanceSq = vector.DistanceSq(m_vCommandPosition, referencePos);
			m_fDistance = Math.Sqrt(distanceSq);
		}

		float sizeScale = 1;
		// Calculate the distance from the camera to the world position on a range from 0-1
		if (distanceLimit != 0)
			sizeScale = Math.Clamp(distanceLimit - m_fDistance, 0.2, distanceLimit) / distanceLimit;
		
		// Get the current Y value (size multiplier) based on the current distance from the curve
		sizeScale = LegacyCurve.Curve(ECurveType.CurveProperty2D, sizeScale, m_aScaleCurve)[1];
		
		float iconSize;
		if (m_wCommandIconWidget)
		{
			iconSize = Math.Clamp(m_fCommandIconWidgetSize * sizeScale, m_fCommandIconWidgetSize * 0.2, m_fCommandIconWidgetSize);
			m_wCommandIconWidget.SetSize(iconSize, iconSize);
		}

		if (m_wCommandGlowWidget)
		{
			iconSize = Math.Clamp(m_fCommandGlowWidgetSize * sizeScale, m_fCommandGlowWidgetSize * 0.2, m_fCommandGlowWidgetSize);
			m_wCommandGlowWidget.SetSize(iconSize, iconSize);
		}
		
		if (m_wCommandPulsePreviewWidget && m_bIsPreview)
		{
			iconSize = Math.Clamp((m_fCommandIconWidgetSize * sizeScale) * 2, m_fCommandPulsePreviewWidgetSize * 0.2, m_fCommandIconWidgetSize);
			m_wCommandPulsePreviewWidget.SetSize(iconSize, iconSize);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePosition()
	{
		if (m_PhysicsHelper)
			return;

		PlayerController controller = GetGame().GetPlayerController();
		PlayerCamera camera = PlayerCamera.Cast(GetGame().GetCameraManager().CurrentCamera());
		if (!camera)
			return;

		IEntity controlledEntity = controller.GetControlledEntity();

		vector mat[4];
		camera.GetTransform(mat);
		vector end = mat[3] + mat[2] * m_fCommandVisualRange;

		//ToDO:@Kuceramar check / optimize trace performance
		if (!m_TraceParam)
		{
			m_TraceParam = new TraceParam();
			m_TraceParam.Flags = TraceFlags.ENTS | TraceFlags.WORLD | TraceFlags.ANY_CONTACT;
			m_TraceParam.LayerMask = EPhysicsLayerDefs.Projectile;
		}
		
		array<IEntity> excludeArray = {};
		excludeArray.Insert(controlledEntity);

		ChimeraCharacter playerCharacter = ChimeraCharacter.Cast(controlledEntity);		
		if (playerCharacter && playerCharacter.IsInVehicle())
		{
			CompartmentAccessComponent compartmentComp = playerCharacter.GetCompartmentAccessComponent();
			if (compartmentComp)
			{
				IEntity vehicleIn = compartmentComp.GetVehicleIn(playerCharacter);
				if (vehicleIn)
					excludeArray.Insert(vehicleIn);
			}
		}

		m_TraceParam.Start = mat[3];
		m_TraceParam.End = end;
		m_TraceParam.ExcludeArray = excludeArray;
		m_TraceParam.TraceEnt = null;

		float traceResult = GetGame().GetWorld().TraceMove(m_TraceParam, null);

		if (m_TraceParam.TraceEnt || traceResult == 1)
			m_vCommandPosition = m_TraceParam.Start + (end - mat[3]) * (m_TraceParam.End - m_TraceParam.Start).Length() * traceResult;

		m_fDistance = traceResult * (m_TraceParam.End - m_TraceParam.Start).Length();
	}

	//------------------------------------------------------------------------------------------------
	//! Performs a dot product check against threshold whether point is in line of sight of provided transformation.
	//! \param[in] vector point Point to perform check for
	//! \return true if is in line of sight, flase otherwise
	protected bool IsInLineOfSight(vector point, vector transform[4])
	{
		vector direction = point - transform[3];
		direction.Normalize();

		return vector.Dot(direction, transform[2]) > LINE_OF_SIGHT_THRESHOLD;
	}

	//------------------------------------------------------------------------------------------------
	protected void FadeOutPulse(WidgetAnimationOpacity anim)
	{
		anim.GetOnCompleted().Remove(FadeOutPulse);

		//Delay the animation by 1 frame, so the previous animation gets deleted first
		GetGame().GetCallqueue().Call(FadeOutPulseDelayed);
	}

	//------------------------------------------------------------------------------------------------
	void FadeOutPulseDelayed()
	{
		AnimateWidget.Opacity(m_wCommandPulseWidget, 0, UIConstants.FADE_RATE_DEFAULT, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteWidget(WidgetAnimationOpacity anim = null)
	{
		if (anim)
			anim.GetOnCompleted().Remove(DeleteWidget);

		m_bCanUpdate = false;
		m_HUDManager.StopDrawing(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void HideWidget(bool deleteWidget = false)
	{
		AnimateWidget.StopAnimation(m_wCommandFrameWidget, WidgetAnimationOpacity);
		WidgetAnimationOpacity anim = AnimateWidget.Opacity(m_wCommandFrameWidget, 0, UIConstants.FADE_RATE_DEFAULT, true);

		if (deleteWidget && anim)
			anim.GetOnCompleted().Insert(DeleteWidget);
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleVisualizationDuration()
	{
		// Hide the widget after its definied show time in seconds
		if (!m_bIsPreview && m_eCommandVisualizationDuration == SCR_ECommandVisualizationDuration.BRIEF)
			GetGame().GetCallqueue().CallLater(HideWidget, SCR_ECommandVisualizationDuration.BRIEF * 1000, param1: true);
	}

	//------------------------------------------------------------------------------------------------
	//! Recalculates worldPosition to screen space.
	//! \param[in] world
	//! \param[in] worldPosition
	//! \param[out] posX X Position on the screen
	//! \param[out] posY Y Position on the screen
	//! \param[in] int Camera thats being used
	protected bool GetWorldToScreenPosition(BaseWorld world, vector worldPosition, out float posX, out float posY, int cameraIndex = -1)
	{
		vector screenPosition = GetGame().GetWorkspace().ProjWorldToScreen(worldPosition, world, cameraIndex);
		posX = screenPosition[0];
		posY = screenPosition[1];

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes the visualization of the provided command
	//! \param[in] targetPosition
	//! \param[in] command
	void VisualizeCommand(vector targetPosition, SCR_BaseGroupCommand command)
	{
		if (!command)
			return;

		string imageSet = command.GetIconImageset();
		string icon = command.GetIconName();

		if (imageSet == string.Empty)
			m_wCommandIconWidget.LoadImageTexture(0, icon);
		else
			m_wCommandIconWidget.LoadImageFromSet(0, imageSet, icon);

		m_eCommandVisualizationDuration = command.GetVisualizationDuration();
		HandleVisualizationDuration();

		m_vCommandPosition = targetPosition;

		m_bCanUpdate = true;

		m_wCommandPulsePreviewWidget.SetVisible(false);
		m_wCommandFrameWidget.SetVisible(true);
		AnimateWidget.StopAnimation(m_wCommandFrameWidget, WidgetAnimationOpacity);
		AnimateWidget.Opacity(m_wCommandFrameWidget, 1, UIConstants.FADE_RATE_DEFAULT);

		if (m_bIsPreview && m_wCommandPulsePreviewWidget)
		{
			m_wCommandPulsePreviewWidget.SetVisible(true);
			return;
		}
		
		if (!m_wCommandPulseWidget)
			return;

		float sizePulse[2] = {PULSE_WIDGET_MAX_SIZE, PULSE_WIDGET_MAX_SIZE};
		m_wCommandPulseWidget.SetVisible(true);
		WidgetAnimationOpacity anim = AnimateWidget.Opacity(m_wCommandPulseWidget, 1, UIConstants.FADE_RATE_DEFAULT);
		AnimateWidget.Size(m_wCommandPulseWidget, sizePulse, UIConstants.FADE_RATE_DEFAULT * 0.5);
		anim.GetOnCompleted().Insert(FadeOutPulse);
	}

	//------------------------------------------------------------------------------------------------
	void SetCanUpdatePosition(bool canUpdate)
	{
		m_bCanUpdatePosition = canUpdate;
	}

	//------------------------------------------------------------------------------------------------
	void SetIsPreview(bool isPreview)
	{
		m_bIsPreview = isPreview
	}
}
