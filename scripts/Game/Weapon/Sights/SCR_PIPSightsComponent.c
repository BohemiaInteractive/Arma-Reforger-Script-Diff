[EntityEditorProps(category: "GameScripted/Weapon/Sights/Picture in Picture Sights", description: "", color: "0 0 255 255")]
class SCR_PIPSightsComponentClass: ScriptedSightsComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PIPSightsComponent : ScriptedSightsComponent
{
	[Attribute("{EF091399D840192D}UI/layouts/Sights/PictureInPictureSightsLayout.layout", UIWidgets.ResourceNamePicker, "The layout used for this PIP component", params: "layout")]
	protected ResourceName m_sLayoutResource;

	[Attribute("RTTexture0", UIWidgets.EditBox, "Name of RTTexture widget within provided layout")]
	protected string m_sRTTextureWidgetName;

	[Attribute("RenderTarget0", UIWidgets.EditBox, "Name of RenderTarget widget within provided layout")]
	protected string m_sRTargetWidgetName;

	[Attribute("1", UIWidgets.Slider, "Camera index used for this PIP component", params: "0 31 1")]
	protected int m_iCameraIndex;

	[Attribute("12.5", UIWidgets.Slider, "Camera field of view used by this PIP component", params: "0 89.9 0.1")]
	protected float m_fCameraFOV;

	[Attribute("0.2", UIWidgets.Slider, "Camera near clipping plane", params: "0 1000 0.01")]
	protected float m_fNearPlane;

	[Attribute("1500", UIWidgets.Slider, "Camera field of view used by this PIP component", params: "0 5000 1")]
	protected float m_fFarPlane;

	// Point info?
	[Attribute("1.0", UIWidgets.EditBox, "Scale of resolution used by the PIP", params: "0.1 1 0.1")]
	protected float m_fResolutionScale;

	// Point info?
	[Attribute("0 0 0", UIWidgets.EditBox, "Camera offset used for this PIP")]
	protected vector m_vCameraPoint;


	//! Is this PIP currently enabled?
	protected bool m_bIsEnabled;

	//! The root layout hierarchy widget
	protected ref Widget m_wRoot;

	//! Parent entity owner
	protected IEntity m_pOwner;

	//! The render target texture found within our layout
	protected RTTextureWidget m_wRenderTargetTextureWidget;

	//! The render target found within our layout
	protected RenderTargetWidget m_wRenderTargetWidget;

	//! The camera to be used by this pip component
	protected ScriptCamera m_PIPCamera;

	#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntity owner, IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}

	//------------------------------------------------------------------------------------------------
	protected override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		const float axisLength = 0.1;

		vector origin = owner.GetOrigin() + m_vCameraPoint;
		vector right = origin + owner.GetTransformAxis(0) * axisLength;
		vector up = origin + owner.GetTransformAxis(1) * axisLength;
		vector fwd = origin + owner.GetTransformAxis(2) * axisLength;

		ref auto rightArrow = Shape.CreateArrow(origin, right, 0.01, ARGB(240,255,0,0), ShapeFlags.ONCE);
		ref auto upArrow = Shape.CreateArrow(origin, up, 0.01, ARGB(240,0,255,0), ShapeFlags.ONCE);
		ref auto fwdArrow = Shape.CreateArrow(origin, fwd, 0.01, ARGB(240,0,0,255), ShapeFlags.ONCE);
	}
	#endif

	//------------------------------------------------------------------------------------------------
	void SetEnabled(bool enabled)
	{
		// A component without an owner shouldnt exist
		if (!m_pOwner)
			return;

		// disabled->enabled
		// Create neccessary items
		if (enabled && !m_bIsEnabled)
		{
			// Try to create UI for PIP,
			// output params are either set to valid ones,
			// or root itself is set to null and destroyed
			if (!m_wRoot || !m_wRenderTargetTextureWidget || !m_wRenderTargetWidget)
				m_wRoot = CreateUI(m_sLayoutResource, m_sRTTextureWidgetName, m_sRTargetWidgetName, m_wRenderTargetTextureWidget, m_wRenderTargetWidget);

			if (!m_wRoot)
			{
				Print("Could not create PIP layout!");
				return;
			}

			// Create PIP camera
			if (!m_PIPCamera)
				m_PIPCamera = CreateCamera(m_pOwner, m_vCameraPoint, m_iCameraIndex, m_fCameraFOV, m_fNearPlane, m_fFarPlane);

			// Set camera index of render target widget
			BaseWorld baseWorld = m_pOwner.GetWorld();
			m_wRenderTargetWidget.SetWorld(baseWorld, m_iCameraIndex);

			// Set resolution scale
			m_wRenderTargetWidget.SetResolutionScale(m_fResolutionScale, m_fResolutionScale);

			// Set RTT on parent
			m_wRenderTargetTextureWidget.SetRenderTarget(m_pOwner);

			m_bIsEnabled = true;
			return;
		}

		// enabled -> disabled
		if (!enabled && m_bIsEnabled)
		{
			Destroy();

			m_bIsEnabled = false;
			return;
		}
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
	protected void DestroyUI(Widget root)
	{
		if (root)
		{
			root.RemoveFromHierarchy();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param parent Parent entity this camera should be attached to
	//! \param position Local offset from parent entity
	//! \param cameraIndex The index created camera should use, 0-31
	//! \param fov The field of view of camera 0-89
	//! \param nearPlane Near clipping plane of the camera in metres
	//! \param farPlane Far clipping plane of the camera in metres
	protected ScriptCamera CreateCamera(IEntity parent, vector position, int cameraIndex, float fov, float nearPlane, float farPlane)
	{
		// Spawn camera
		BaseWorld baseWorld = parent.GetWorld();
		ScriptCamera pipCamera = ScriptCamera.Cast(GetGame().SpawnEntity(ScriptCamera, baseWorld));

		// Since we use autotransform,
		// we set camera matrix to "local" coordinates
		vector mat[4];
		Math3D.MatrixIdentity3(mat);
		mat[3] = position;


		pipCamera.SetTransform(mat);
		pipCamera.Index = cameraIndex;
		pipCamera.FreeFly = false;

		// These are attributes used in constructor of
		// scriptCamera so they will most likely not work

		/*
		pipCamera.FOV = m_fCameraFOV;
		pipCamera.NearPlane = nearPlane;
		pipCamera.FarPlane = farPlane;
		*/

		// Instead we do it the old fashionate way
		baseWorld.SetCameraVerticalFOV(cameraIndex, m_fCameraFOV);
		baseWorld.SetCameraFarPlane(cameraIndex, farPlane);
		baseWorld.SetCameraNearPlane(cameraIndex, nearPlane);

		// Set camera to hierarchy
		parent.AddChild(pipCamera, -1, EAddChildFlags.AUTO_TRANSFORM);
		return pipCamera;
	}

	//------------------------------------------------------------------------------------------------
	protected void DestroyCamera(ScriptCamera camera)
	{
		if (camera)
		{
			IEntity cameraParent = camera.GetParent();
			if (cameraParent)
				cameraParent.RemoveChild(camera);

			delete camera;
		}
	}

	//------------------------------------------------------------------------------------------------
	// Destroy everything this component created during its lifetime
	protected void Destroy()
	{
		m_wRenderTargetTextureWidget.RemoveRenderTarget(m_pOwner);
		DestroyCamera(m_PIPCamera);
		DestroyUI(m_wRoot);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSActivated()
	{
		SetEnabled(true);
		super.OnSightADSActivated();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSDeactivated()
	{
		SetEnabled(false);
		super.OnSightADSDeactivated();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnInit(IEntity owner)
	{
		m_pOwner = owner;
	}

	//------------------------------------------------------------------------------------------------
	//! Constructor
	void SCR_PIPSightsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Destructor
	void ~SCR_PIPSightsComponent()
	{
		Destroy();
	}
};