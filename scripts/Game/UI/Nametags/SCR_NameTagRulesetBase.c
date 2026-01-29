//------------------------------------------------------------------------------------------------
//! Base class for nametag visiblity ruleset - single tag displayed continuously
[BaseContainerProps()]
class SCR_NameTagRulesetBase : Managed
{
	[Attribute("0", UIWidgets.CheckBox, desc: "seconds \n how long it takes to de/focus on target in order to collapse/expand nametag")]
	protected float m_fTagFocusDelay;
		
	[Attribute("0", UIWidgets.CheckBox, desc: "seconds \n how long it takes for nametag to fade in/out when switching visibility", "0 10")]
	float m_fTagFadeTime;
	
	[Attribute("true", UIWidgets.CheckBox, "This will cancel fade out transitions if new primary tag is acquired")]
	protected bool m_bCancelFadeOnNewFocus;
	
	[Attribute("1", UIWidgets.CheckBox, desc: "seconds \n how long does it take to delete tags from dead entities")]
	float m_fDeadEntitiesCleanup;
	
	[Attribute(SCR_Enum.GetDefault(SCR_ELOSTraceMode.ALL_PLAYERS), desc: "Filters which entities within the zone should be traced for obstructed visibility", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ELOSTraceMode))];
	protected SCR_ELOSTraceMode m_eTraceForLOSMode;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "seconds \n how long it takes for the tag to start fading if LOS is obstructed")]
	protected float m_fLOSFadeDelay;
	
	[Attribute("120", UIWidgets.CheckBox, desc: "degrees \n base angle from center of the screen to entity required to display nametag, automatically scales with distance")]
	protected int m_iMaxAngle;						// degrees, no tags may be visible outside of this angle to screen center
	
	// Consts
	const int DISABLE_FLAGS = ENameTagFlags.UPDATE_DISABLE | ENameTagFlags.DISABLED; // flags which mark the nametag for cleanup process
	const int SECONDARY_FLAGS = ENameTagEntityState.VON; // tags with these entity states are eligible to be secondary tags
	const int NEAR_TAG_ANGLE = 3;					// degrees, when multiple tags are within this value, the one which is closer will be prioritized as for primary tag to avoid nametag overlap 
	const int DISABLED_CLEANUP = 5.0;				// seconds, time it takes to cleanup disabled tag
	const int POSITION_UPDATE = 0.1;				// seconds, how often position is updated
	const float POS_TRANSITION_MULT = 2;			// multiplier of time it takes to transition between different tag positions (HEAD -> BODY), default is 1 second
	const float VERT_ANGLE_ADJUST = 0.3; 			// multiplies vertical angle weight when determining whether tag should be displayed, lower number means higher vertical tolerance -> easier to focus tag
	const vector HEAD_LOS_OFFSET = "0 0.05 0";		// ajdustment of head pos for LOS tracing
	protected vector m_CameraPosition;
	const float VON_STOP_TIME = 1;					// time (seconds) it takes for the VON state to end since the last received VON
	protected const string ZONE_SCOPED = "Scoped";

	protected float m_fTimeElapsed = 0;				// used to calculate elapsed time for nametag expand/collapse delay	
	protected float m_fTimeFade = 0;				// time for the tag to fade when no longer visible
	protected float m_fFocusPrioAngle;				// highest priority candidate for tag 	
	protected int m_iIndexLOS = 0;					// nametag index for periodic LOS testing
	protected int m_iRefResolutionX;				// cached reference resolution, not to be confused with reference screen size
	protected int m_iRefResolutionY;				// =||=
	
	SCR_NameTagData m_CurrentPlayerTag;				// nametag data of current player	
	protected SCR_NameTagData m_ClosestAngleTag;	// candidate for primary tag
	protected SCR_NameTagData m_PrimaryTag;			// primary visible tag which needs to pass all of the visibility rules
	protected SCR_NameTagData m_ExpandedTag;		// currently expanded tag, by default primary tag after expand conditions are passed
	protected SCR_2DPIPSightsComponent m_PIPSightsComp; // this is expensive to acquire for each tag individually, so its cached here and reset every frame 
	
	protected BaseWorld m_World;
	protected WorkspaceWidget m_Workspace;
	protected SCR_NameTagDisplay m_NTDisplay;
	protected SCR_NameTagConfig m_ZoneCfg;

	protected ref array<ref SCR_NameTagData> m_aNameTags = {};		// basic nametag array from display
	protected ref array<ref SCR_NameTagData> m_aCandidateTags = {};	// nametags who passed initial visibility rules

	//------------------------------------------------------------------------------------------------
	//! Determine whether the tag passes basic conditions for visibility
	//! \param data is the subject nametag
	//! \param timeSlice is frame time slice
	//! \return Returns true if tag passed conditions for visibility
	protected bool TestVisibility(SCR_NameTagData data, float timeSlice)
	{
		// current players tag, update tag specific vars and return
		if (data == m_CurrentPlayerTag)
		{
			data.UpdateTagPos();
			return false;
		}

		// out of zone range
		data.m_fTimeSliceUpdate += timeSlice;
		
		if (data.m_fTimeSliceUpdate >= POSITION_UPDATE) // avoid updating every frame
		{
			data.m_fTimeSliceUpdate = 0;
			
			data.m_fDistance = vector.DistanceSq(m_CurrentPlayerTag.m_vEntWorldPos, data.m_Entity.GetOrigin()); // checked through origin here to avoid more expensive pos check bottom
		}
		
		bool foundZone;
		int distMin;
		int distMax;
		foreach (SCR_NameTagZone zone : m_ZoneCfg.m_aZones)
		{
			if (zone.GetZoneName() == ZONE_SCOPED)
			{
				if ((!SCR_2DPIPSightsComponent.IsPIPActive() && !SCR_BinocularsComponent.IsZoomedView() && !SCR_2DSightsComponent.IsUsingSights()) || m_CurrentPlayerTag.m_CharController.IsFreeLookEnabled())
					continue;
			}
			
			distMin = zone.m_iZoneStartSq;
			distMax = zone.m_iZoneEndSq;
			
			if (data.m_fDistance < distMax && data.m_fDistance > distMin)
			{
				data.m_fTimeSliceCleanup = 0;
				foundZone = true;
				break;
			}
		}
		
		if (!foundZone)
		{
			// cleanup tags which are more than distance + 10m (10^2) away from base zone query 
			if (data.m_fDistance >= m_ZoneCfg.m_fFarthestZoneRangePow2 + 100)
				data.m_fTimeSliceCleanup += timeSlice;			
			
			return false;
		}
		
		// vehicle visibility 
		if (data.m_Flags & ENameTagFlags.VEHICLE)	// in vehicle
		{
			if (!data.m_VehicleParent.m_bIsControlledPresent)	// dont show vehicle crew tags until inside the same vehicle
				return false;
			else if (m_CurrentPlayerTag.m_CharController.IsInThirdPersonView())	// dont show vehicle crew tags if in third person
				return false;
		}
		
		if (data.m_eType == ENameTagEntityType.VEHICLE)	// is vehicle
		{	
			if (data.m_Flags & ENameTagFlags.VEHICLE_DISABLE || SCR_VehicleTagData.Cast(data).m_bIsControlledPresent)
				return false;
		}
								
		// update tag position
		if (data.m_eAttachedTo == data.m_eAttachedToLast)
			data.UpdateTagPos();
		else
		{
			data.m_fTimeSlicePosChange += timeSlice * POS_TRANSITION_MULT;
			if ( data.m_fTimeSlicePosChange >= 1.0 )
			{
				data.UpdateTagPos();
				data.m_eAttachedToLast = data.m_eAttachedTo;
			}
			else 
			{
				data.UpdateTagPos();
				data.m_vTagWorldPos = vector.Lerp( data.m_vTagWorldPosLast, data.m_vTagWorldPos, data.m_fTimeSlicePosChange );
			}
		}
					
		CalculateScreenPos(data);
		if (m_iRefResolutionY * 1.1 < data.m_vTagScreenPos[1] || m_iRefResolutionX * 1.1 < data.m_vTagScreenPos[0] || 0 > data.m_vTagScreenPos[1] || 0 > data.m_vTagScreenPos[0])
			return false;	
			
		return true;	
	} 
	
	//------------------------------------------------------------------------------------------------
	//! Determine secondary visibility rules from the filtered tags
	//! \param data is the subject nametag
	//! \param timeSlice is frame time slice
	//! \return Returns true if tag passed conditions for visibility
	protected bool TestVisibilityFiltered(SCR_NameTagData data, float timeSlice)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update for visible tags
	//! \param data is the subject nametag
	//! \param timeSlice is the OnFrame timeslice
	protected void UpdateVisibleTag(SCR_NameTagData data, float timeSlice)
	{				
		if (data.m_eEntityStateFlags & ENameTagEntityState.HIDDEN)	// fade in tag			
			data.DeactivateEntityState(ENameTagEntityState.HIDDEN);
									
		// Zone selection
		int currentZone = -1;
		for (int i = 0; i < m_ZoneCfg.m_iZoneCount; i++) 	// get lowest nametag zone and use it to display the tag, for perf improvement, only nearest zone is searched for when deciding where to draw
		{
			if ( data.m_fDistance <=  m_ZoneCfg.m_aZones[i].m_iZoneEndSq )
			{
				currentZone = i;
				break; 		// zones are ordered from lowest, if we find a match leave
			}
		}
		
		if (data.m_iZoneID != currentZone)
		{	
			if (data.m_iZoneID != -1)
				m_ZoneCfg.m_aZones[data.m_iZoneID].HideElements(data);		
				
			m_ZoneCfg.m_aZones[currentZone].OnZoneChanged(data); // mark for update if we moved between zones	
		}
																					
		data.m_iZoneID = currentZone;
		m_ZoneCfg.m_aZones[data.m_iZoneID].DrawElements(data);
									
		// Set opacity and ZOrder
		if ( ~data.m_eEntityStateFlags & ENameTagEntityState.FOCUSED )
		{
			data.m_fOpacityFade = Math.InverseLerp(m_ZoneCfg.m_fFarthestZoneRangePow2, 0, data.m_fDistance);	// fade opacity with distance
			data.m_NameTagWidget.SetOpacity(data.m_fVisibleOpacity * data.m_fOpacityFade);
			
			int zOrder = Math.Round((data.m_fOpacityFade - 1.0) * 100);	// reuse lerp above, the further away target, the higher -zOrder applied //  1.0 is max opacity // 100 is decimal correction 
			data.m_NameTagWidget.SetZOrder(zOrder);
		}
		
		// Set tag 2D pos
		FrameSlot.SetPos(data.m_NameTagWidget, data.m_vTagScreenPos[0], data.m_vTagScreenPos[1]);
						
		// VON indication
		if (data.m_eEntityStateFlags & ENameTagEntityState.VON)
		{
			if (data.m_fTimeSliceVON >= VON_STOP_TIME)
				data.DeactivateEntityState(ENameTagEntityState.VON);
			else
				data.m_fTimeSliceVON += timeSlice;
		}
		
		// Tag positioning for vehicle
		if ( data.m_Flags & ENameTagFlags.VEHICLE )
		{
			if (data.m_eAttachedTo != ENameTagPosition.BODY)
				data.SetTagPosition(ENameTagPosition.BODY, false);
		}
		else if (data.m_eAttachedTo == ENameTagPosition.BODY)
			data.SetTagPosition(ENameTagPosition.HEAD, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change opacity based on line of sight
	protected void CheckLOS()
	{								
		int count = m_aNameTags.Count(); // use base array here because cnadidate tags are being actively filtered out
		if (m_iIndexLOS >= count)
			m_iIndexLOS = 0;
		
		vector cameraMat[4];
		m_World.GetCurrentCamera(cameraMat);
		m_CameraPosition = cameraMat[3];
		
		int id;
		
		while (m_iIndexLOS < count)
		{	
			SCR_NameTagData data = m_aNameTags.Get(m_iIndexLOS);
			m_iIndexLOS++;
			
			if (data == m_CurrentPlayerTag)
				continue;
			
			id = data.m_iZoneID;
			if (id == -1)
				id = 0;
			
			if (m_eTraceForLOSMode == SCR_ELOSTraceMode.NON_GROUP_MEMBERS && data.m_iGroupID == m_CurrentPlayerTag.m_iGroupID)
				continue;
			
			// trace
			if ( TraceLOS(data) )
			{
				data.m_Flags &= ~ENameTagFlags.OBSTRUCTED;
				data.m_fTimeSliceFade = 0;
			}
			else if (data.m_fTimeSliceFade >= m_fLOSFadeDelay)
			{
				data.m_fTimeSliceFade = 0;
				data.m_Flags &= ~ENameTagFlags.FADE_TIMER;
				data.m_Flags |= ENameTagFlags.OBSTRUCTED;
			}
			else 
				data.m_Flags |= ENameTagFlags.FADE_TIMER;
			
			return;	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if target is visible to the player
	//! \param data is the subject nametag
	//! \return Returns true if the tested target is visible/not obstructed in current players line of sight
	protected bool TraceLOS(SCR_NameTagData data)
	{						
		TraceParam param = new TraceParam;
		param.Start = m_CameraPosition;
		param.End = data.m_vEntHeadPos + HEAD_LOS_OFFSET;
		param.Flags = TraceFlags.ANY_CONTACT | TraceFlags.WORLD | TraceFlags.ENTS; 
		IEntity targetEntity = data.m_Entity;
		
		if (data.GetVehicleCompartment())
			targetEntity = data.GetVehicleCompartment().GetVehicle();
		
		array<IEntity> ExcludeArray = {};
		ExcludeArray.Insert(targetEntity);
		ExcludeArray.Insert(m_CurrentPlayerTag.m_Entity);
		param.ExcludeArray = ExcludeArray;
		
		param.LayerMask = TRACE_LAYER_CAMERA;
		float percent = GetGame().GetWorld().TraceMove(param, null);
		if (percent == 1)
			return true;
		
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		percent = GetGame().GetWorld().TraceMove(param, null);
		if (percent == 1)
			return true;
		
		return false;
	}
			
	//------------------------------------------------------------------------------------------------
	//! Evaluates if player is focusing a nametag entity or not, expands/collapses nametag after a set time
	//! \param timeSlice is the OnFrame timeslice
	protected void SelectTagToExpand(float timeSlice)
	{
		// If player is not focusing anyone
		if (!m_PrimaryTag)
		{
			m_fTimeElapsed = 0;
		}
		// Player focusing on a target
		else
		{
			// No expanded tag, start the expand timer
			if (!m_ExpandedTag && m_PrimaryTag.m_Flags & ENameTagFlags.VISIBLE) // can be called in the same frame with disable so check the flag here
			{	
				if (m_fTagFocusDelay == 0)
					ExpandTag(m_PrimaryTag);
				else if (m_fTimeElapsed > m_fTagFocusDelay)
					ExpandTag(m_PrimaryTag);
				
				m_fTimeElapsed += timeSlice;
				
				return;
			}
			// Expanded tag
			else 
			{				
				// If focusing the expanded ent
				if ( m_PrimaryTag == m_ExpandedTag )
				{
					m_fTimeElapsed = 0;
					return;
				}
			}
		}
	}
				
	//------------------------------------------------------------------------------------------------
	//! Logic for showing additional info when nametag becomes expanded
	//! \param data is the subject nametag
	protected void ExpandTag(SCR_NameTagData data)
	{
		m_fTimeElapsed = 0;
		m_ExpandedTag = data;
		
		data.m_NameTagWidget.SetZOrder(0);
		data.ActivateEntityState(ENameTagEntityState.FOCUSED);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Logic for collapsing the nametag back to its basic format
	//! \param timeSlice is the frame slice
	//! \param data is the subject nametag
	protected void CollapseTag(float timeSlice, SCR_NameTagData data)
	{
		m_fTimeElapsed = 0;
		m_ExpandedTag = null;
		
		data.DeactivateEntityState(ENameTagEntityState.FOCUSED);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fade out tag and disable it when it no longer passes rules for visibility
	//! \param data is the subject nametag
	//! \return True if tag was completely cleaned up
	protected bool DisableTag(SCR_NameTagData data, float timeSlice)
	{
		//It will take at least 3 DisableTag calls before the tag is cleaned up -> 1) Initial call which sets UPDATE_DISABLE 2) DISABLED flag when anims are finished  3) clean up
		if (data.m_Flags & ENameTagFlags.DISABLED)
		{						
			if (data.m_fTimeSliceCleanup >= DISABLED_CLEANUP || data.m_Flags & ENameTagFlags.VEHICLE_DISABLE)
			{
				data.Cleanup();
				return true;
			}
			
			return false;
		}
		
		// update screen pos while fading so it doesnt get stuck on screen side mid fade
		data.UpdateTagPos();
		data.m_fAngleToScreenCenter = GetCameraToEntityAngle(data.m_vEntWorldPos, VERT_ANGLE_ADJUST);
		if (data.m_fAngleToScreenCenter > m_iMaxAngle) // player quickly turning 180 deg returns tag to screen space visibility if it has fade, this check removes it before that point
			data.m_fTimeSliceVisibility = 0;
		
		CalculateScreenPos(data);
		FrameSlot.SetPos(data.m_NameTagWidget, data.m_vTagScreenPos[0], data.m_vTagScreenPos[1]);	// uses unscaled vals
			
		// Fade out tag
		if (data.m_Flags & ENameTagFlags.VISIBLE)
		{
			if (m_ExpandedTag == data)
				CollapseTag(timeSlice, data);
						
			data.ActivateEntityState(ENameTagEntityState.HIDDEN);
			
			data.m_Flags &= ~ENameTagFlags.VISIBLE;
			data.m_Flags &= ~ENameTagFlags.VISIBLE_PASS;
			data.m_Flags |= ENameTagFlags.UPDATE_DISABLE | ENameTagFlags.NAME_UPDATE | ENameTagFlags.ENT_TYPE_UPDATE;
			
			data.m_fTimeSliceVisibility = m_fTagFadeTime;
			return false;
		}	
		// when done, disable for perf
		else if ( data.m_fTimeSliceVisibility <= 0 )
		{
			data.m_NameTagWidget.SetVisible(false);
			data.m_Flags &= ~ENameTagFlags.UPDATE_DISABLE;
			data.m_Flags |= ENameTagFlags.DISABLED;

			// Hide zone elements
			if (m_ZoneCfg.m_aZones.IsIndexValid(data.m_iZoneID))
				m_ZoneCfg.m_aZones[data.m_iZoneID].HideElements(data);

			data.m_iZoneID = -1; // zone id reset, needed for proper update
			
			if (m_PrimaryTag == data)
				m_PrimaryTag = null;
		}
		
		data.m_fTimeSliceVisibility -= timeSlice;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calculates angle between screen center and target entity
	//! \param targetOrigin is the targets world position
	//! \return Returns angle in degrees
	protected float GetCameraToEntityAngle(vector targetOrigin, float verticalWeight)
	{
		vector cameraMat[4];
		m_World.GetCurrentCamera(cameraMat);
						
		vector camPosFix = cameraMat[3];
		vector camDirFix = cameraMat[2];
		
		if (verticalWeight != 1)
		{
			camPosFix[1] = camPosFix[1] * verticalWeight;
			camDirFix[1] = camDirFix[1] * verticalWeight;
			targetOrigin[1] = targetOrigin[1] * verticalWeight;
		}
			
		vector direction = vector.Direction(camPosFix, targetOrigin);							// vector from camera to target									
		float dotProduct = vector.Dot(camDirFix, direction);									// dot product of target dir & and screen center dir
		float vectorMagnitude = direction.Length() * camDirFix.Length();	
		if (vectorMagnitude == 0)
			vectorMagnitude = 1;
								
		float vectorsAngle = Math.Acos( dotProduct / vectorMagnitude );	// angle between two vectors (radians)
		float angle = vectorsAngle * Math.RAD2DEG;																
		return angle;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calculate tag 2d screen pos
	//! \param data is the subject nametag
	protected void CalculateScreenPos(SCR_NameTagData data)
	{
		if (!SCR_2DPIPSightsComponent.IsPIPActive())	// PIP support
		{
			data.m_vTagScreenPos = m_Workspace.ProjWorldToScreen( data.m_vTagWorldPos, m_World );	// no PIP
		}
		else
		{		
			if (!m_PIPSightsComp)
			{
				m_PIPSightsComp = ArmaReforgerScripted.GetCurrentPIPSights();
				if (!m_PIPSightsComp)	// IsPIPActive() will return active for an extra frame while this cant be fetched anymore
					return;
			}
			
			if (m_CurrentPlayerTag.m_CharController.IsFreeLookEnabled())
			{
				data.m_vTagScreenPos = m_Workspace.ProjWorldToScreen( data.m_vTagWorldPos, m_World );	// if PIP free look, draw tags standard way
				if (m_PIPSightsComp.IsScreenPositionInSights(data.m_vTagScreenPos))
					data.m_vTagScreenPos = {0,0};														// hide tags within scope area
			}
			else 
			{
				int camID = m_PIPSightsComp.GetPIPCameraIndex();
				
				vector aimRotationModification = m_CurrentPlayerTag.m_CharController.GetAimingComponent().GetAimingRotationModification();
				
				vector camTM[4];
				m_PIPSightsComp.GetPIPCamera().GetWorldTransform(camTM);
				
				vector nametagTM[4];
				Math3D.MatrixIdentity4(nametagTM);
				nametagTM[3] = data.m_vTagWorldPos;
				
				float zoomCorrection = m_PIPSightsComp.GetFOV() / m_PIPSightsComp.GetMainCameraFOV(); // m_PIPSightsComp.GetFOV() * 0.04386 or m_PIPSightsComp.GetFOV() / 22.8 more accurate but these values seem random
				SCR_Math3D.RotateAround(nametagTM, camTM[3], camTM[1], aimRotationModification[0] * zoomCorrection, nametagTM);
				SCR_Math3D.RotateAround(nametagTM, camTM[3], camTM[0], -aimRotationModification[1] * zoomCorrection, nametagTM);
				
				data.m_vTagWorldPos = nametagTM[3];
				
				data.m_vTagScreenPos = m_Workspace.ProjWorldToScreen(data.m_vTagWorldPos, m_World, camID);

				float screenWidth = m_Workspace.GetWidth();
				float screenHeight = m_Workspace.GetHeight();
				float screenAspectRatio = screenWidth / screenHeight;
				float screenInverseAspectRatio = screenHeight / screenWidth;
				
				// GetSightsRadiusScreen doesnt work properly if you resize your screen :c
				//float radiusOfScope = m_PIPSightsComp.GetSightsRadiusScreen();
				float radiusOfScope = screenHeight * 0.5;
				float widthCorrection = (screenWidth / (radiusOfScope * 2.0)) * screenInverseAspectRatio; // PIP scope uses a square render target but m_Workspace.ProjWorldToScreen() assumes the result will be used in a rectangular render target 
				float heightCorrection = screenHeight / (radiusOfScope * 2.0);
				
				data.m_vTagScreenPos[0] = data.m_vTagScreenPos[0] / widthCorrection;
				data.m_vTagScreenPos[1] = data.m_vTagScreenPos[1] / heightCorrection;
				vector positionCorrection = { screenWidth * 0.5 - radiusOfScope * screenAspectRatio, // multiplied by aspect ratio for the same reason as above comment
											 screenHeight * 0.5 - radiusOfScope,
											 0.0 };
				data.m_vTagScreenPos += positionCorrection;
				
				if (!m_PIPSightsComp.IsScreenPositionInSights(data.m_vTagScreenPos))
					data.m_vTagScreenPos = {0,0};		// hide tags outside of scope area
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Uses provided nametag data to determine which tags pass the visibility conditions and filters them into a new array
	//! \param timeSlice is frame time slice
	protected void DetermineVisibility(float timeSlice)
	{			
		int count = m_aNameTags.Count();
		
		// Primary set of visibility conditions
		for ( int i = 0; i < count; i++ )  
		{					
			if (!m_aNameTags.IsIndexValid(i))
				continue;
			
			SCR_NameTagData data = m_aNameTags.Get(i);
			if (!data)
				continue;
				
			// If no ent is found, data is cleaned up		
			if (!data.m_Entity)
			{	
				data.Cleanup();
				i--;
				count--;
				continue;
			}
					
			// Determine tag visibility
			if ( TestVisibility(data, timeSlice) )
				m_aCandidateTags.Insert(data);
			// If no longer passing conditions or in process of being disabled
			else if ( (data.m_Flags & ENameTagFlags.VISIBLE) || (data.m_Flags & DISABLE_FLAGS) )
			{
				if ( DisableTag(data, timeSlice) )	// if tag gets cleaned up, we need to adjust index
				{
					i--;
					count--;
				}
			}
		}
		
		// Secondary set of visibility conditions
		m_ClosestAngleTag = null;
		m_fFocusPrioAngle = 0;
		
		count = m_aCandidateTags.Count();
		for (int i = 0; i < count; i++)
		{
			SCR_NameTagData data = m_aCandidateTags.Get(i);
			
			// fails are disabled, passes move on to final selection
			if ( !TestVisibilityFiltered(data, timeSlice) )
			{
				if ( (data.m_Flags & ENameTagFlags.VISIBLE) || (data.m_Flags & ENameTagFlags.UPDATE_DISABLE) )
					DisableTag(data, timeSlice);
				
				m_aCandidateTags.Remove(i);
				count--;
				i--;
			}
		}
		
		// Line of sight checks
		if (m_eTraceForLOSMode != SCR_ELOSTraceMode.NO_TRACE)
			CheckLOS();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Settings changed event
	protected void OnUserSettingChanged()
	{
		m_Workspace = g_Game.GetWorkspace();
		m_iRefResolutionX = m_Workspace.DPIUnscale(m_Workspace.GetWidth());
		m_iRefResolutionY = m_Workspace.DPIUnscale(m_Workspace.GetHeight());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup
	protected void Cleanup()
	{
		m_aNameTags = null;		
		m_aCandidateTags = null;	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Debug
	protected void UpdateDebug()
	{
		DbgUI.Begin("NameTag debug");
		const string dbg = "queried tags: %1 | candidate tags: %2";
		DbgUI.Text( string.Format( dbg, m_aNameTags.Count(), m_aCandidateTags.Count() ) );
		const string dbg2 = "primary: %1";
		DbgUI.Text( string.Format( dbg2, m_PrimaryTag ) );
		const string dbg3 = "focusAngle: %1 | sq distance: %2";
		float distance = 0;
		if (m_PrimaryTag) distance = m_PrimaryTag.m_fDistance;
		DbgUI.Text( string.Format( dbg3, m_fFocusPrioAngle, distance ) );
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize ruleset
	//! \return Returns true is successful
	bool Init(SCR_NameTagDisplay display, SCR_NameTagConfig config)
	{		
		GenericEntity player = GenericEntity.Cast( SCR_PlayerController.GetLocalControlledEntity() );
		if (!player)
			return false;
		
		m_World = GetGame().GetWorld(); 
		m_NTDisplay = display;
		m_ZoneCfg = config;
		
		GetGame().OnUserSettingsChangedInvoker().Insert(OnUserSettingChanged);
		OnUserSettingChanged();	// cached reso init
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice, array<ref SCR_NameTagData> nameTags, SCR_NameTagData nameTagPlayer)
	{
		m_aNameTags = nameTags;
		if (m_aNameTags.Count() <= 1) // 1 tag is player
			return;
				
		m_CurrentPlayerTag = nameTagPlayer;
		m_PIPSightsComp = null;
		
		// Filter tags using visibility rules
		m_aCandidateTags.Clear();
		DetermineVisibility(timeSlice);
		
		// Tag expand/collapse logic
		SelectTagToExpand(timeSlice);
		
		#ifdef NAMETAG_DEBUG
			UpdateDebug();
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_NameTagRulesetBase()
	{
		Cleanup();
	}
};

enum SCR_ELOSTraceMode
{
	NO_TRACE,			// Line of sight tracing turned off
	NON_GROUP_MEMBERS,	// Trace only players outside your group
	ALL_PLAYERS			// Line of sight tracing of everyone
}
