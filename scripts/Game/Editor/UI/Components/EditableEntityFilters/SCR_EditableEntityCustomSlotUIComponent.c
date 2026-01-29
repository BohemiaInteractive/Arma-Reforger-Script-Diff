class SCR_EditableEntityCustomSlotUIComponent : SCR_EditableEntitySceneSlotUIComponent
{
	[Attribute()]
	protected string m_sBoneName;
	
	[Attribute()]
	protected vector m_vOffset;
	
	[Attribute(defvalue: "0", desc: "How large the widget is going to be at 1 m distance.")]
	protected float m_bScaleCoef;
	
	protected GenericEntity m_Owner;
	protected int m_iBoneIndex;
	protected CameraManager m_CameraManager;
	
	//------------------------------------------------------------------------------------------------
	override vector UpdateSlot(int screenW, int screenH, vector posCenter, vector posCam)
	{
		if (m_iBoneIndex == -1)
			return super.UpdateSlot(screenW, screenH, posCenter, posCam);
		
		if (m_Entity)
		{
			vector transform[4];
			m_Owner.GetAnimation().GetBoneMatrix(m_iBoneIndex, transform);
			
			vector posWorld = m_Owner.CoordToParent(transform[3] + m_vOffset);
			vector pos = m_Workspace.ProjWorldToScreen(posWorld, m_World);

			if (m_bShowOffScreen)
			{
				//--- Show on screen border when off-screen
				if (m_bScaleCoef != 0)
					FrameSlot.SetSize(m_Widget, m_bScaleCoef, m_bScaleCoef);
					
				return ShowOffScreen(pos, screenW, screenH, posCenter);
			}
			else
			{
				//--- On-screen only
				if (pos[2] > 0)
				{					
					if (m_bScaleCoef != 0)
					{
						CameraBase camera = m_CameraManager.CurrentCamera();
						if (camera)
						{
							float size = (m_bScaleCoef * (70 / camera.GetVerticalFOV())) / vector.Distance(posWorld, camera.GetWorldTransformAxis(3));
							FrameSlot.SetSize(m_Widget, size, size);
						}
					}
					
					FrameSlot.SetPos(m_Widget, pos[0], pos[1]);
					m_Widget.SetZOrder(vector.Distance(posWorld, posCam)); //--- Don't use DistanceSq, can overflow int
					m_Widget.SetVisible(true);
					return pos;
				}
			}
		}

		m_Widget.SetVisible(false);
		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	override void InitSlot(SCR_EditableEntityComponent entity)
	{
		super.InitSlot(entity);
		
		m_CameraManager = GetGame().GetCameraManager();
		
		m_Owner = m_Entity.GetOwner();
		m_iBoneIndex = m_Owner.GetAnimation().GetBoneIndex(m_sBoneName);
		
		FrameSlot.SetAlignment(m_Widget, 0.5, 0.5);
	}
}
