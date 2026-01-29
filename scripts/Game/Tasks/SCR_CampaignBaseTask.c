//------------------------------------------------------------------------------------------------
//class SCR_CampaignBaseTaskClass: SCR_TaskClass
//{
//};

//------------------------------------------------------------------------------------------------
//! This class is meant to be inherited.
//! Don't use this as your custom task!
//class SCR_CampaignBaseTask : SCR_Task
//{
//	
//	const int MIN_OFFSET = 3;
//	const int MAX_OFFSET = 80;
//	//**************************//
//	//PROTECTED MEMBER VARIABLES//
//	//**************************//
//	
//	protected SCR_CampaignMilitaryBaseComponent m_TargetBase = null;
//	protected int m_iTargetBaseId = -1;
//	
//	//*********************//
//	//PUBLIC MEMBER METHODS//
//	//*********************//
//	
//	//------------------------------------------------------------------------------------------------
//	int GetTargetBaseId()
//	{
//		return m_iTargetBaseId;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Sets the base that is marked as the target of this task.
//	//! For capture, target base should be the base to be captured.
//	//! For defend, target base should be the base to be defended.
//	//! For recon, target base should be the base around which recon should be done.
//	void SetTargetBase(notnull SCR_CampaignMilitaryBaseComponent targetBase)
//	{
//		if (SCR_MapEntity.GetMapInstance())
//		{
//			SCR_MapEntity.GetOnMapZoom().Insert(OnMapZoom);
//			SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpenCallback);
//		}
//			
//		m_TargetBase = targetBase;
//
//		SetOrigin(m_TargetBase.GetOwner().GetOrigin());
//		
//		if (RplSession.Mode() != RplMode.Dedicated)
//		{
//			CreateMapUIIcon();
//			UpdateMapInfo();
//			SetHUDIcon();
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnMapOpenCallback(MapConfiguration config)
//	{
//		SetOffset();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnMapZoom(float pixelPerUnit)
//	{
//		SetOffset();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	// Temporary method to set offset of task marker from base marker. Once we have a proper solution for the map, this can be deleted.
//	void SetOffset()
//	{
//		vector newPos = m_TargetBase.GetOwner().GetOrigin();
//		newPos[0] = newPos[0] + CalculateOffset() / 2.5;
//		newPos[2] = newPos[2] - CalculateOffset() / 5;
//		SetOrigin(newPos);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	// Temporary method to set offset of task marker from base marker. Once we have a proper solution for the map, this can be deleted.
//	float CalculateOffset()
//	{
//		float pixelPerUnit = SCR_MapEntity.GetMapInstance().GetCurrentZoom();
//		int max_offset = MAX_OFFSET;
//		int min_offset = MIN_OFFSET;
//		float min_zoom = SCR_MapConstants.MAX_PIX_PER_METER;
//		float max_zoom = SCR_MapEntity.GetMapInstance().GetMinZoom();
//		
//		float a = max_offset * min_offset * (max_zoom - min_zoom) / (min_offset - max_offset);
//		float b = min_zoom - a / min_offset;
//		
//		return b + a / pixelPerUnit;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	string GetBaseNameWithCallsign()
//	{
//		if (m_TargetBase.GetType() == SCR_ECampaignBaseType.RELAY)
//			return m_TargetBase.GetBaseNameUpperCase();
//		return string.Format("%1 (%2)", m_TargetBase.GetBaseNameUpperCase(), m_TargetBase.GetCallsignDisplayNameOnlyUC());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns the base that is marked as target of this task.
//	//! For capture, target base should be the base to be captured.
//	//! For defend, target base should be the base to be defended.
//	//! For recon, target base should be the base around which recon should be done.
//	SCR_CampaignMilitaryBaseComponent GetTargetBase()
//	{
//		return m_TargetBase;
//	}
//	
//	//******************************//
//	//PUBLIC OVERRIDE MEMBER METHODS//
//	//******************************//
//	
//	//------------------------------------------------------------------------------------------------
//	override void Deserialize(ScriptBitReader reader)
//	{
//		super.Deserialize(reader);
//		
//		int baseID;
//		reader.ReadInt(baseID);
//		
//		SCR_CampaignMilitaryBaseComponent base = SCR_GameModeCampaign.GetInstance().GetBaseManager().FindBaseByCallsign(baseID);
//		
//		if (!base)
//		{
//			m_iTargetBaseId = baseID;
//			return;
//		}
//		
//		if (base.GetFaction() == GetTargetFaction())
//			return;
//		
//		SetTargetBase(base);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void Serialize(ScriptBitWriter writer)
//	{
//		super.Serialize(writer);
//		
//		int baseID = -1;
//		SCR_CampaignMilitaryBaseComponent base = GetTargetBase();
//		if (base)
//			baseID = base.GetCallsign();
//		
//		writer.WriteInt(baseID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected override void UpdateMapInfo()
//	{
//		//Insert task icon into base's map UI
//		//m_TargetBase cannot be null here
//	}
//};
