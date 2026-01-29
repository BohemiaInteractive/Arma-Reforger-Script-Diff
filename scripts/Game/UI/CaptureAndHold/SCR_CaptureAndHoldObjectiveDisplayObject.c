//! This object serves as a wrapper for individual HUD objective widgets.
class SCR_CaptureAndHoldObjectiveDisplayObject
{
	//! Topmost widget of this object
	protected Widget m_wRoot;
	//! Text that displays objective distance
	protected TextWidget m_wDistanceText;
	//! Text that displays name or symbol of the objective
	protected TextWidget m_wNameText;

	//! The objective image
	protected ImageWidget m_wIcon;
	//! The objective is under attack symbol image
	protected ImageWidget m_wUnderAttackIcon;
	//! The major objective symbol image
	protected ImageWidget m_wMajorIcon;
	//! The backdrop image of this element
	protected ImageWidget m_wBackdropIcon;

	protected VerticalLayoutWidget m_wLayout;

	//! Blend progress of when within the area
	protected float m_fBlendScreenPosition;
	//! Color blending rate
	protected const float COLOR_BLEND_SPEED = 5.5;
	//! Position blending rate
	protected const float POSITION_BLEND_SPEED = 2.5;

	// Initial values
	protected float m_fOriginalXSize;
	protected float m_fOriginalYSize;
	protected float m_fOriginalTitleSize;
	protected float m_fOriginalTextSize;

	// Loaded texture indices
	protected const int ATLAS_INDEX_NEUTRAL = 0;
	protected const int ATLAS_INDEX_BLUFOR = 1;
	protected const int ATLAS_INDEX_OPFOR = 2;


	//! The area this object represents
	protected SCR_CaptureAndHoldArea m_AffiliatedArea;

	//------------------------------------------------------------------------------------------------
	//! Returns the area this element represents.
	SCR_CaptureAndHoldArea GetArea()
	{
		return m_AffiliatedArea;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns root widget of this element.
	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	//------------------------------------------------------------------------------------------------
	//! Create new wrapper for objective display.
	void SCR_CaptureAndHoldObjectiveDisplayObject(notnull Widget root, notnull SCR_CaptureAndHoldArea area)
	{
		m_wRoot = root;
		m_AffiliatedArea = area;
		m_wNameText = TextWidget.Cast(m_wRoot.FindAnyWidget("Name"));
		m_fOriginalTitleSize = 15;

		m_wDistanceText = TextWidget.Cast(m_wRoot.FindAnyWidget("Distance"));
		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_Faction"));
		m_wBackdropIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_Backdrop"));

		m_wUnderAttackIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_UnderAttack"));
		m_wLayout = VerticalLayoutWidget.Cast(m_wRoot.FindAnyWidget("VerticalLayout"));
		m_fOriginalXSize = FrameSlot.GetSizeX(m_wLayout);
		m_fOriginalYSize = FrameSlot.GetSizeY(m_wLayout);

		m_wMajorIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_MajorMark"));

		m_fOriginalTextSize= 14;
	}

	//------------------------------------------------------------------------------------------------
	//! Update image state according to provided area state
	protected void UpdateImageState(Widget root, float timeSlice)
	{
		Faction owningFaction = m_AffiliatedArea.GetOwningFaction();

		Color targetColor;
		if (!owningFaction)
			targetColor = Color.FromRGBA(249, 210, 103, 255);
		else
			targetColor = owningFaction.GetFactionColor();

		int rootColor = m_wRoot.GetColor().PackToInt();
		Color currentColor = Color.FromInt(rootColor);
		if (currentColor != targetColor)
			currentColor.Lerp(targetColor, timeSlice * COLOR_BLEND_SPEED);

		// Only show major symbol when area is marked as one
		m_wMajorIcon.SetVisible(m_AffiliatedArea.IsMajor());

		// Imageset for Base state icon
		const string AtlasImageset = "{225B7CAD5CEC4AE3}UI/Imagesets/CaptureAndHold/CaptureAndHoldAtlas.imageset";

		// Area is completely neutral
		if (!owningFaction)
		{
			m_wIcon.LoadImageFromSet(0,AtlasImageset, "CAH_NEUTRAL_LARGE");
			m_wBackdropIcon.LoadImageFromSet(0,AtlasImageset, "CAH_NEUTRAL_BACKDROP");

			m_wRoot.SetColor(currentColor);
			// With no owning faction, there is no contesting faction
			m_wUnderAttackIcon.SetVisible(false);
			return;
		}

		// TODO: Improvement desirable
		// This is far from ideal and is not sandbox enough.
		FactionKey factionKey = owningFaction.GetFactionKey();
		if (factionKey == "US" || factionKey == "FIA")
		{
			m_wIcon.LoadImageFromSet(0,AtlasImageset, "CAH_BLUFOR_LARGE");
			m_wBackdropIcon.LoadImageFromSet(0,AtlasImageset, "CAH_BLUFOR_BACKDROP");

			m_wUnderAttackIcon.SetRotation(0);
		}
		else if (factionKey == "USSR")
		{
			m_wIcon.LoadImageFromSet(0,AtlasImageset, "CAH_OPFOR_LARGE");
			m_wBackdropIcon.LoadImageFromSet(0,AtlasImageset, "CAH_OPFOR_BACKDROP");

			m_wUnderAttackIcon.SetRotation(45);
		}

		m_wRoot.SetColor(currentColor);

		Faction contestingFaction = m_AffiliatedArea.GetContestingFaction();
		if (contestingFaction)
		{
			m_wUnderAttackIcon.SetVisible(true);
			// Pulsing animation
			//Color col = new Color(1.0, 1.0, 1.0, 1.0);
			Color col = Color.FromRGBA(249, 210, 103, 255);
			float val01 = Math.Sin( m_AffiliatedArea.GetWorld().GetWorldTime() * 0.01 ) * 0.5 + 0.5;
			col.Lerp(contestingFaction.GetFactionColor(), val01);
			m_wUnderAttackIcon.SetColor(col);

			//Color selfCol = new Color(1.0, 1.0, 1.0, 1.0);
			Color selfCol = Color.FromRGBA(249, 210, 103, 255);
			selfCol.Lerp(owningFaction.GetFactionColor(), val01);
			m_wRoot.SetColor(selfCol);
		}
		else
		{
			m_wUnderAttackIcon.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Update this widget as static UI element, ie. not reprojecting it automatically.
	void UpdateStatic(float timeSlice)
	{
		m_wDistanceText.SetVisible(false);
		UpdateImageState(m_wRoot, timeSlice);
		// Set objective symbol
		m_wNameText.SetText(m_AffiliatedArea.GetAreaSymbol());
	}

	//------------------------------------------------------------------------------------------------
	//! Update this widget as dynamic UI element projected to screen space
	void UpdateDynamic(IEntity playerEntity, float timeSlice)
	{
		vector objectiveWorldPosition = m_AffiliatedArea.GetWorldObjectiveCenter();
		vector projectedScreenPosition = m_wRoot.GetWorkspace().ProjWorldToScreen(objectiveWorldPosition, m_AffiliatedArea.GetWorld());

		vector cameraMatrix[4];
		m_AffiliatedArea.GetWorld().GetCurrentCamera(cameraMatrix);

		float alpha = 1.0;
		bool visible = true;

		// This case should rarely happen as we do not expect spawn protection area
		// to overlay a capture area, but regardless..
		bool isPopupDrawn;
		SCR_PopUpNotification popupNotifications = SCR_PopUpNotification.GetInstance();
		if (popupNotifications && popupNotifications.GetCurrentMsg())
			isPopupDrawn = true;

		// Center to screen and stay fixed
		if (playerEntity && m_AffiliatedArea.IsCharacterInside(SCR_ChimeraCharacter.Cast(playerEntity)) && !isPopupDrawn)
		{
			// Update screen space blend
			m_fBlendScreenPosition += (timeSlice * POSITION_BLEND_SPEED);
			visible = true; // Always draw inside
		}
		else
		{
			// Worldprojection
			vector dirToCamera = (objectiveWorldPosition - cameraMatrix[3]).Normalized();
			float dot = vector.Dot(dirToCamera, cameraMatrix[2]);
			if (dot < 0.6666 && m_fBlendScreenPosition <= 0) // Force animation
				visible = false;

			float alphaScale = Math.InverseLerp(0.667, 1.0, dot);
			alphaScale = Math.Clamp(alphaScale, 0.0, 1.0);

			if (m_fBlendScreenPosition <= 0.0)
				alpha = Math.Lerp(0.5, 1.0, alphaScale);
			else
				alpha = 1.0;

			// Blend the point out significantly in optics, because the projection is misaligned
			if (ArmaReforgerScripted.IsScreenPointInPIPSights(projectedScreenPosition, ArmaReforgerScripted.GetCurrentPIPSights()))
				alpha = Math.Min(alpha, 0.65);

			m_fBlendScreenPosition -= (timeSlice * POSITION_BLEND_SPEED);
		}

		// Clamp the blend so we don't overshoot
		m_fBlendScreenPosition = Math.Clamp(m_fBlendScreenPosition, 0.0, 1.0);

		// If not visible, do not draw and that's it
		if (!visible)
		{
			m_wRoot.SetVisible(false);
			return;
		}

		// Otherwise update widget
		m_wDistanceText.SetVisible(true);

		// Distance text
		float distance = vector.Distance(cameraMatrix[3], objectiveWorldPosition);
		distance = Math.Round(distance);
		m_wDistanceText.SetTextFormat("#AR-CAH-Objective_Distance", distance);

		// Update image state
		UpdateImageState(m_wRoot, timeSlice);

		// Opacity
		m_wRoot.SetOpacity(alpha);

		float x, y;
		float scale = 1.0;
		float textScale = 1.0;
		// Interpolate position
		if (m_fBlendScreenPosition > 0.0)
		{
			WorkspaceWidget ww = m_wRoot.GetWorkspace();
			int w = ww.GetWidth();
			int h = ww.GetHeight();
			float fixedX = ww.DPIUnscale(0.5 * w);
			float fixedY = ww.DPIUnscale(0.15 * h);

			SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
			if (editorManager && editorManager.IsOpened())
				fixedY += 40;

			x = Math.Lerp(projectedScreenPosition[0], fixedX, m_fBlendScreenPosition);
			y = Math.Lerp(projectedScreenPosition[1], fixedY, m_fBlendScreenPosition);
			scale = m_fBlendScreenPosition * 2.25;
			textScale = m_fBlendScreenPosition * 1.85;
		}
		else
		{
			x = projectedScreenPosition[0];
			y = projectedScreenPosition[1];
		}

		float xScale = m_fOriginalXSize * scale;
		float yScale = m_fOriginalYSize * scale;
		// Apply
		FrameSlot.SetPos(m_wRoot, x - 0.5 * xScale, y - 0.5 * yScale);

		// Scale
		FrameSlot.SetSize(m_wLayout, xScale, yScale);

		m_wNameText.SetExactFontSize(m_fOriginalTitleSize * textScale);
		m_wDistanceText.SetExactFontSize(m_fOriginalTextSize * textScale);

		// Set objective symbol
		m_wNameText.SetText(m_AffiliatedArea.GetAreaSymbol());

		// And make widget visible
		m_wRoot.SetVisible(true);
	}
}