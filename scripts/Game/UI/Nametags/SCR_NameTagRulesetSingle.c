//------------------------------------------------------------------------------------------------
// Ruleset for displaying a single tag only
[BaseContainerProps()]
class SCR_NameTagRulesetSingle : SCR_NameTagRulesetBase
{
	protected ref array<ref SCR_NameTagData> m_aSecondaryTags = {};	// nametags who passed secondary visibility rules
	
	//------------------------------------------------------------------------------------------------
	override protected bool TestVisibilityFiltered(SCR_NameTagData data, float timeSlice)
	{
		if (data.m_Flags & ENameTagFlags.FADE_TIMER)
			data.m_fTimeSliceFade += timeSlice;
		
		if (data.m_Flags & ENameTagFlags.OBSTRUCTED)	// obstructed, this is checked here in order for the LOS checks to have run for the smaller subset of tag data 
			return false;
		
		float distLerp = Math.InverseLerp(m_ZoneCfg.m_fFarthestZoneRangePow2, 0, data.m_fDistance);	// Reduce the angle required to show with distance -> the further is the entity, angle required to focus it gets smaller
		distLerp *= m_iMaxAngle;
		
		data.m_fAngleToScreenCenter = GetCameraToEntityAngle(data.m_vEntWorldPos, VERT_ANGLE_ADJUST);
		if ( data.m_fAngleToScreenCenter > distLerp)	// within displayable angle
			return false;
							
		if ( data.m_fAngleToScreenCenter - NEAR_TAG_ANGLE < m_fFocusPrioAngle || m_fFocusPrioAngle == 0 )	// if closer than the current focus priority 
		{
			if ( !m_ClosestAngleTag || (m_fFocusPrioAngle - data.m_fAngleToScreenCenter) > NEAR_TAG_ANGLE || m_ClosestAngleTag.m_fDistance > data.m_fDistance ) // if angle difference is small && previous tag is closer, keep the (previous) closer target
			{
				m_ClosestAngleTag = data;
				m_fFocusPrioAngle = data.m_fAngleToScreenCenter;
			}
		}
		
		if (data.m_eEntityStateFlags & SECONDARY_FLAGS)
		{
			if (~data.m_Flags & ENameTagFlags.VISIBLE_PASS)
			{
				data.m_Flags |= ENameTagFlags.VISIBLE_PASS;
				m_aSecondaryTags.Insert(data);
			}
		}
		else 
			data.m_Flags &= ~ENameTagFlags.VISIBLE_PASS;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void DetermineVisibility(float timeSlice)
	{
		super.DetermineVisibility(timeSlice);
		
		int count = m_aSecondaryTags.Count();
		for (int i = count - 1; i > -1; i--)
		{		
			SCR_NameTagData data = m_aSecondaryTags.Get(i);
			if (data == m_ClosestAngleTag)
				continue;
			
			if (~data.m_Flags & ENameTagFlags.VISIBLE_PASS)
			{
				m_aSecondaryTags.Remove(i);
				DisableTag(data, timeSlice);	// this is here because the above iteration wont be able to fail it since the tag could still be passing the primary conditions
				continue;
			}
			
			UpdateVisibleTag(data, timeSlice);
		}
		
		if (m_ClosestAngleTag)	// primary tag
		{
			if (!m_PrimaryTag || m_ClosestAngleTag == m_PrimaryTag)	// no tag visible yet or update
			{
				m_PrimaryTag = m_ClosestAngleTag;
				UpdateVisibleTag(m_PrimaryTag, timeSlice);
				return;
			}
			else if (m_bCancelFadeOnNewFocus && m_PrimaryTag)
				m_PrimaryTag.m_fTimeSliceVisibility = 0; // kind of a hack for fade out disabling when we find a new tag in order to instantly acquire it
		}
		
		if (m_PrimaryTag)	// disable the last primary tag
		{
			if (m_PrimaryTag.m_Flags & ENameTagFlags.DELETED) // if this tag no longer exists skip
			{
				m_PrimaryTag = null;
				m_ExpandedTag = null;
				return;
			}
			
			if (m_PrimaryTag.m_Flags & ENameTagFlags.VISIBLE_PASS)
			{
				CollapseTag(timeSlice, m_PrimaryTag);
				m_PrimaryTag = null;
			}
			else 
				DisableTag(m_PrimaryTag, timeSlice);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void Cleanup()
	{
		super.Cleanup();
			
		m_aSecondaryTags = null;	
	}

};
