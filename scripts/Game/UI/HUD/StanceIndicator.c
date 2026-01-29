class StanceIndicator : SCR_InfoDisplay
{
	// Stances
	const string STANCE_STAND 	= "{9DF327726AD6B515}UI/Textures/StanceIndicator/stance_stand.edds";
	const string STANCE_CROUCH 	= "{90ED76A233697F90}UI/Textures/StanceIndicator/stance_crouch.edds";
	const string STANCE_PRONE 	= "{A7117D97ED663EC4}UI/Textures/StanceIndicator/stance_prone.edds";
	
	private ImageWidget m_wStance = null;
	private ECharacterStance m_eCurrentStance = ECharacterStance.PRONE;
	private SCR_PlayerController m_PlayerController;
	
	protected static StanceIndicator s_Instance = null;
	sealed static StanceIndicator GetInstance() { return s_Instance; }
	
	//------------------------------------------------------------------------------------------------
	override void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		if (m_wRoot)
		{
			m_wStance = ImageWidget.Cast(m_wRoot.FindAnyWidget("Image0"));
			if (m_wStance)
			{
				m_wStance.LoadImageTexture((int)ECharacterStance.PRONE, STANCE_PRONE);
				m_wStance.LoadImageTexture((int)ECharacterStance.CROUCH, STANCE_CROUCH);
				m_wStance.LoadImageTexture((int)ECharacterStance.STAND, STANCE_STAND);
			}
			m_wRoot.SetOpacity(0);
		}
		
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		m_PlayerController = SCR_PlayerController.Cast(game.GetPlayerController());
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateValues(IEntity owner, float timeSlice)
	{
		if (!m_wRoot || !m_wStance || !m_PlayerController)
			return;
		

/*		CharacterStance stance = CharacterStance.STANCE_ERECT;
		CharacterEntity character = CharacterEntity.Cast(owner);
		if (character)
		{
			stance = character.GetCurrentStance();
		}
		
		if (m_eCurrentStance != stance)
		{
			m_eCurrentStance = stance;
			m_wStance.SetImage(stance);

			// Update widget size
			int x;
			int y;
			m_wStance.GetImageSize(stance,x,y);
			float fx = (float)x * 0.3;
			float fy = (float)y * 0.3;
			m_wStance.SetSize(fx,fy);
			
			if (stance == ECharacterStance.STAND)
			{
				if (m_wRoot.GetOpacity() > 0 || m_wRoot.IsVisible())
					AnimateWidget.Opacity(m_wRoot, 0, UIConstants.FADE_RATE_SLOW);
			}
			else
			{
				if (m_wRoot.GetOpacity() < 1 || !m_wRoot.IsVisible())
					AnimateWidget.Opacity(m_wRoot, 1, UIConstants.FADE_RATE_DEFAULT);
			}
		}*/
	}
};