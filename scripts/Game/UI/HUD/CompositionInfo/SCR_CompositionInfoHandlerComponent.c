//#define COMPOSITION_INFO_EVERYWHERE

class SCR_CompositionInfoHandlerComponentClass : ScriptComponentClass
{

}

class SCR_CompositionInfoHandlerComponent : ScriptComponent
{
	[Attribute("ToggleCompositionReporting")]
	protected string m_sToggleInfoAction;
	
	[Attribute("#AR-EditorToggle_LoadingText_Text")]
	protected string m_sAuthorNameLoading;
	
	[Attribute("#AR-CoreMenus_Tooltips_Unavailable")]
	protected string m_sAuthorNameFailed;
	
	[Attribute("ReportUser")]
	protected string m_sReportUserAction;
	
	[Attribute("CompositionInfoContext")]
	protected string m_sActionContext;
	
	[Attribute("20")]
	protected float m_fTraceDistance;
	
	protected bool m_ReportingActive;
	protected bool m_IsInit;
	protected bool m_bNameUpdateRequested;
	
	protected ref SCR_CompositionInfoDisplay m_CompositionInfoDisplay;
	protected ref SCR_CompositionDetails m_CurrentDetails;
	
	//------------------------------------------------------------------------------------------------	
	SCR_CompositionDetails TraceEditableEntity()
	{
		CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
		vector transform[4];
		camera.GetTransform(transform);
		
		vector position = camera.GetOrigin();
		vector direction = transform[2];
		direction.Normalize();
		TraceParam trace = MakeTraceParam(position, position + (direction * m_fTraceDistance), TraceFlags.ENTS);
		array<IEntity> exludedEntities = {};
		
		IEntity excEnt = GetOwner();
		if (excEnt)
			exludedEntities.Insert(excEnt);
		
		excEnt = SCR_PlayerController.Cast(excEnt).GetControlledEntity();
		if (excEnt)
			exludedEntities.Insert(excEnt);
		
		trace.ExcludeArray = exludedEntities;
		
		GetGame().GetWorld().TraceMove(trace, null);
		
		IEntity traceEntity = trace.TraceEnt;
		if (!traceEntity)
		{
			m_CurrentDetails = null;
			return null;
		}
		
		traceEntity = SCR_EntityHelper.GetMainParent(traceEntity, true);
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(traceEntity.FindComponent(SCR_EditableEntityComponent));
		if (!editableEntity)
		{
			m_CurrentDetails = null;
			return null;
		}
		
		vector line[2];
		line[0] = position;
		line[1] = position + (transform[3] * m_fTraceDistance);
		Shape.CreateLines(Color.RED, ShapeFlags.NOZBUFFER, line, 1);
		
		SCR_EditableEntityAuthor author = editableEntity.GetAuthor();
		if (!author || author.m_sAuthorUID.IsEmpty())
		{
			m_CurrentDetails = null;
			return null;
		}
		
		string oldName;
		if (m_CurrentDetails)
			oldName = m_CurrentDetails.m_sAuthorName;
		
		m_CurrentDetails = new SCR_CompositionDetails(oldName, author.m_iAuthorID, author.m_sAuthorUID, author.m_ePlatform, editableEntity.GetDisplayName(), editableEntity.GetAuthorLastUpdated(), editableEntity.GetOwner().GetOrigin());
		
		if (!m_bNameUpdateRequested && m_CurrentDetails.m_sAuthorName.IsEmpty())
		{
			Print("CompositionInfoHandler - Name Cache Update Requested!", LogLevel.VERBOSE);
			
			m_bNameUpdateRequested = true;
			m_CompositionInfoDisplay.UpdateName(m_sAuthorNameLoading);
			
			PlayerManager.s_OnPlayerNameCacheUpdateInvoker.Insert(OnNameCacheUpdated);
			PlayerManager.RequestPlayerNameCacheUpdate({author.m_sAuthorUID});
		}
		
		return m_CurrentDetails;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void OnNameCacheUpdated(bool success)
	{
		if (success)
		{
			Print("CompositionInfoHandler - Name Cache Updated", LogLevel.VERBOSE);
		}
		else
		{
			Print("CompositionInfoHandler - Name Cache Update failed!", LogLevel.VERBOSE);
		}

		m_bNameUpdateRequested = false;
		PlayerManager.s_OnPlayerNameCacheUpdateInvoker.Remove(OnNameCacheUpdated);

		if (!m_CurrentDetails)
			return;
		
		PlayerManager pm = GetGame().GetPlayerManager();
		
		if (!m_CurrentDetails.m_sAuthorUID.IsEmpty())
		{
			// Even if the request failed, we should try to do lookup by IdentityId first
			m_CurrentDetails.m_sAuthorName = pm.GetPlayerNameByIdentity(m_CurrentDetails.m_sAuthorUID);
		}
		else
		{
			m_CurrentDetails.m_sAuthorName = pm.GetPlayerName(m_CurrentDetails.m_iAuthorID);
		}
		
		if (m_CurrentDetails.m_sAuthorName.IsEmpty())
		{
			m_CurrentDetails.m_sAuthorName = m_sAuthorNameFailed;
		}
		
		m_CompositionInfoDisplay.UpdateName(m_CurrentDetails.m_sAuthorName);
	}
	
	//--- EVENTS ---//
	//------------------------------------------------------------------------------------------------	
	protected void OnReportingToggle()
	{
		if (!m_IsInit)
			return;
		
		m_ReportingActive = !m_ReportingActive;
		
		// Disabled Reporting
		//InputManager im = GetGame().GetInputManager();
		//if (im && m_ReportingActive)
		//	im.AddActionListener(m_sReportUserAction, EActionTrigger.PRESSED, OnReportUser);
		//else if (im)
		//	im.RemoveActionListener(m_sReportUserAction, EActionTrigger.PRESSED, OnReportUser);
		
		if (m_CompositionInfoDisplay)
			return;
		
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)
			return;
		
		SCR_InfoDisplay displayCandidate = hudManager.FindInfoDisplay(SCR_CompositionInfoDisplay);
		if (!displayCandidate)
			return;
		
		m_CompositionInfoDisplay = SCR_CompositionInfoDisplay.Cast(displayCandidate);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void OnReportUser()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		
		SocialComponent sc = SocialComponent.Cast(pc.FindComponent(SocialComponent));
		if (!sc)
			return;
		
		if (!m_CurrentDetails.m_sAuthorName.IsEmpty() && !m_CurrentDetails.m_iAuthorID > -1)
		{
			sc.ReportPlayer(m_CurrentDetails.m_iAuthorID, SCR_EReportReason.NEGATIVE_ATTITUDE);
			PrintFormat("Player: %1, with UID: %2. Was Reported", m_CurrentDetails.m_sAuthorName, m_CurrentDetails.m_iAuthorID, level: LogLevel.NORMAL);
			m_CompositionInfoDisplay.ShowReported();
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_IsInit)
			return;
		
		InputManager im = GetGame().GetInputManager();
		if (!im)
			return;
		
		im.ActivateContext(m_sActionContext);
		
		if (!m_CompositionInfoDisplay)
			return;
		
		m_CompositionInfoDisplay.Show(m_ReportingActive);
		
		if (!m_ReportingActive)
			return;
		
		TraceEditableEntity();
		m_CompositionInfoDisplay.UpdateCompositionDetails(m_CurrentDetails);
	}
	
	//------------------------------------------------------------------------------------------------	
	override protected void EOnInit(IEntity owner)
	{
#ifndef COMPOSITION_INFO_EVERYWHERE
		PlatformService ps = GetGame().GetPlatformService();
		if (!ps || ps.GetLocalPlatformKind() != PlatformKind.PSN)
			return;
#endif
		
		InputManager im = GetGame().GetInputManager();
		if (!im)
			return;
		
		im.AddActionListener(m_sToggleInfoAction, EActionTrigger.DOWN, OnReportingToggle);
		m_IsInit = true;
	}
	
	//------------------------------------------------------------------------------------------------	
	override protected void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------	
	override protected void OnDelete(IEntity owner)
	{
		if (!m_IsInit)
			return;
		
		InputManager im = GetGame().GetInputManager();
		if (!im)
			return;
		
		im.RemoveActionListener(m_sToggleInfoAction, EActionTrigger.DOWN, OnReportingToggle);
	}
}

class SCR_CompositionDetails
{
	string m_sAuthorName;
	int m_iAuthorID;
	string m_sAuthorUID;
	PlatformKind m_ePlatform;
	string m_sEntityName;
	int m_iLastUpdated;
	vector m_vPosition;
	
	//------------------------------------------------------------------------------------------------	
	void SCR_CompositionDetails(string authorName, int authorID, string authorUID, PlatformKind platform, string entityName, int lastUpdated, vector position = vector.Zero)
	{
		m_sAuthorName = authorName;
		m_iAuthorID = authorID;
		m_sAuthorUID = authorUID;
		m_ePlatform = platform;
		m_sEntityName = entityName;
		m_iLastUpdated = lastUpdated;
		m_vPosition = position;
	}
}
