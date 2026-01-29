class SCR_EditableEntitySceneSlotUIComponent : SCR_EditableEntityBaseSlotUIComponent
{
	[Attribute(defvalue: "0")]
	protected float m_fOffScreenPadding;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_OffScreenPrefab;
	
	protected WorkspaceWidget m_Workspace;
	protected BaseWorld m_World;
	protected bool m_bShowOffScreen = false;
	protected bool m_bIsOffScreen;
	protected vector m_vCornersPadding[4];
	protected ImageWidget m_OffScreenWidget;
	protected bool m_bIsPreview;
	
	//------------------------------------------------------------------------------------------------
	//! Set if the icon should be shown on screen borders when its position is off-screen.
	void UpdateShowOffScreen()
	{
		bool showOffScreen = false;
		foreach (SCR_BaseEditableEntityUIComponent component: m_EntityWidgets)
		{
			showOffScreen |= component.IsShownOffScreen();
		}
		
		if (showOffScreen != m_bShowOffScreen)
		{
			if (showOffScreen)
			{
				m_OffScreenWidget = ImageWidget.Cast(m_Workspace.CreateWidgets(m_OffScreenPrefab, m_Widget));
				if (m_OffScreenWidget)
				{
					FrameSlot.SetPos(m_OffScreenWidget, m_fOffScreenPadding * 0.5, m_fOffScreenPadding * 0.5);
					FrameSlot.SetSize(m_OffScreenWidget, m_fOffScreenPadding * 2, m_fOffScreenPadding * 2);
					FrameSlot.SetAlignment(m_OffScreenWidget, 0.5, 0.5);
					m_OffScreenWidget.SetVisible(false);
				}
			}
			else
			{
				if (m_OffScreenWidget)
				{
					m_OffScreenWidget.RemoveFromHierarchy();
					m_OffScreenWidget = null;
				}
			}
		}

		m_bShowOffScreen = showOffScreen;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the icon is supposed to be shown on screen borders when its position is off-screen.
	//! \return True when marked to shown off-screen
	bool IsShownOffScreen()
	{
		return m_bShowOffScreen;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set padding from screen border when the icon is rendered off-screen.
	//! \param[in] padding
	void SetOffScreenPadding(float padding)
	{
		m_fOffScreenPadding = padding;
		m_vCornersPadding = {
			Vector(0, m_fOffScreenPadding, 0), //--- A1
			Vector(1, m_fOffScreenPadding, 0), //--- A2
			Vector(m_fOffScreenPadding, 0, 0), //--- B1
			Vector(m_fOffScreenPadding, 1, 0) //--- B2
		};

		if (m_OffScreenWidget)
			FrameSlot.SetSize(m_OffScreenWidget, m_fOffScreenPadding * 2, m_fOffScreenPadding * 2);
	}

	//------------------------------------------------------------------------------------------------
	//! Get padding from screen border when the icon is rendered off-screen.
	//! \return Padding
	float GetOffScreenPadding()
	{
		return m_fOffScreenPadding;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	Widget GetOffScreenWidget()
	{
		return m_OffScreenWidget;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsPreview()
	{
		return m_bIsPreview;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] A1
	//! \param[in] A2
	//! \param[in] B1
	//! \param[in] B2
	//! \return
	vector GetIntersectionPoint(vector A1, vector A2, vector B1, vector B2)
	{
	    float tmp = (B2[0] - B1[0]) * (A2[1] - A1[1]) - (B2[1] - B1[1]) * (A2[0] - A1[0]);
		if (tmp == 0)
			return vector.Zero;
	 
	    float mu = ((A1[0] - B1[0]) * (A2[1] - A1[1]) - (A1[1] - B1[1]) * (A2[0] - A1[0])) / tmp;
		return {
	        B1[0] + (B2[0] - B1[0]) * mu,
	        B1[1] + (B2[1] - B1[1]) * mu,
			0
		};
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pos
	//! \param[in] screenW
	//! \param[in] screenH
	//! \param[in] posCenter
	//! \return
	vector ShowOffScreen(vector pos, int screenW, int screenH, vector posCenter)
	{
		bool isOffScreenPrev = m_bIsOffScreen;
		float screenWPadded = screenW - m_fOffScreenPadding;
		float screenHPadded = screenH - m_fOffScreenPadding;
		m_bIsOffScreen = pos[2] < 0 || pos[0] < m_fOffScreenPadding || pos[1] < m_fOffScreenPadding || pos[0] > screenWPadded || pos[1] > screenHPadded;
		if (m_bIsOffScreen)
		{
			float intersect = GetIntersectionPoint(posCenter, pos, m_vCornersPadding[0], m_vCornersPadding[1])[0];
			if (intersect > m_fOffScreenPadding && intersect < screenWPadded)
			{
				if (pos[1] < posCenter[1])
				{
					pos = { intersect, m_fOffScreenPadding, 0 }; //--- Top
					if (m_OffScreenWidget)
						m_OffScreenWidget.SetRotation(0);
				}
				else
				{
					pos = { screenW - intersect, screenHPadded, 0 }; //--- Bottom
					if (m_OffScreenWidget)
						m_OffScreenWidget.SetRotation(180);
				}
			}
			else
			{
				intersect = GetIntersectionPoint(posCenter, pos, m_vCornersPadding[2], m_vCornersPadding[3])[1];
				if (pos[0] < posCenter[0])
				{
					pos = { m_fOffScreenPadding, intersect, 0 }; //--- Left
					if (m_OffScreenWidget)
						m_OffScreenWidget.SetRotation(270);
				}
				else
				{
					pos = { screenWPadded, screenH - intersect, 0 }; //--- Right
					if (m_OffScreenWidget)
						m_OffScreenWidget.SetRotation(90);
				}
			}

			m_bIsOffScreen = true;
		}
		
		//--- When off-screen presence changed, execute event in all slot widgets
		if (m_bIsOffScreen != isOffScreenPrev)
		{
			foreach (SCR_BaseEditableEntityUIComponent widgetComponent: m_EntityWidgets)
			{
				widgetComponent.OnShownOffScreen(m_bIsOffScreen);
			}

			if (m_OffScreenWidget)
				m_OffScreenWidget.SetVisible(m_bIsOffScreen);
		}
		
		FrameSlot.SetPos(m_Widget, pos[0], pos[1]);
		m_Widget.SetVisible(true);
		return pos;
	}

	//------------------------------------------------------------------------------------------------
	override vector UpdateSlot(int screenW, int screenH, vector posCenter, vector posCam)
	{
		if (m_Entity)
		{
			vector posWorld;
			if (m_Entity.GetPos(posWorld))
			{
				vector posScreen = m_Workspace.ProjWorldToScreen(posWorld, m_World);
	
				if (m_bShowOffScreen)
				{
					//--- Show on screen border when off-screen
					return ShowOffScreen(posScreen, screenW, screenH, posCenter);
				}
				else
				{
					//--- On-screen only
					if (posScreen[2] > 0)
					{
						FrameSlot.SetPos(m_Widget, posScreen[0], posScreen[1]);
						m_Widget.SetZOrder(-vector.Distance(posWorld, posCam)); //--- Don't use DistanceSq, can overflow int
						m_Widget.SetVisible(true);
						return posScreen;
					}
				}
			}
		}

		m_Widget.SetVisible(false);
		return vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	override Widget CreateWidgetForEntity(SCR_EditableEntityComponent entity, SCR_EntitiesEditorUIRule rule, ResourceName layout)
	{
		Widget widget = super.CreateWidgetForEntity(entity, rule, layout);
		UpdateShowOffScreen();
		return widget;
	}

	//------------------------------------------------------------------------------------------------
	override void DeleteWidget(SCR_EntitiesEditorUIRule rule)
	{
		super.DeleteWidget(rule);
		UpdateShowOffScreen();
	}

	//------------------------------------------------------------------------------------------------
	override void InitSlot(SCR_EditableEntityComponent entity)
	{
		m_Workspace = GetGame().GetWorkspace();
		m_World = entity.GetOwner().GetWorld();
		m_bIsPreview = entity.HasEntityState(EEditableEntityState.PREVIEW);
		SetOffScreenPadding(m_fOffScreenPadding);
		
		super.InitSlot(entity);
	}
}
