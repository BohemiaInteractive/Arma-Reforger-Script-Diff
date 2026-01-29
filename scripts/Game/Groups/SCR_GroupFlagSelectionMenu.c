class SCR_GroupFlagSelectionMenu : DialogUI
{		
	protected const int MAX_COLUMNS = 6;
	protected const float ICONS_SCALE = 0.75; 
	
	protected const ResourceName BUTTON_IMAGE = "{6BBBE8F27E385D3B}UI/layouts/WidgetLibrary/Groups/WLib_GroupFlagButtonImage.layout";	
		
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		InitGroupFlagSelectionMenu(MAX_COLUMNS, BUTTON_IMAGE);			
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		GetGame().GetInputManager().ActivateContext("InteractableDialogContext");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		// This is handled by the tiles themselves
	}
	
	//------------------------------------------------------------------------------------------------
	void CloseDialog()
	{			
		CloseAnimated();
	}	
	
	//------------------------------------------------------------------------------------------------
	void InitGroupFlagSelectionMenu(int maxColumns, ResourceName widget)
	{	
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();		
		if(!groupManager)
			return;
			
		int row = 1;
		int col = 1;	
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		SCR_Faction playerFaction = SCR_Faction.Cast(factionManager.GetLocalPlayerFaction());
		if (!playerFaction)
			return;	
				
		OverlayWidget con = OverlayWidget.Cast(GetRootWidget().FindAnyWidget("Content"));
		
		GridLayoutWidget content = GridLayoutWidget.Cast(con.FindAnyWidget("ContentGrid"));				
		if (!content)
			return;		
		
		ResourceName imageSet = playerFaction.GetGroupFlagImageSet();
		array<ResourceName> textures = {};
		playerFaction.GetGroupFlagTextures(textures);
						
		if (!textures.IsEmpty())
		{
			for(int i = 0, count = textures.Count(); i < count; i++)
			{	
				Widget contentButton = GetGame().GetWorkspace().CreateWidgets(widget, content);		
				if (!contentButton)
					continue;
		
				SCR_GroupFlagImageComponent imageButton = SCR_GroupFlagImageComponent.Cast(contentButton.FindHandler(SCR_GroupFlagImageComponent));	
				if (!imageButton)
					continue;
				
				if (col > maxColumns)
				{
					row++;
					col = 1;
				}
			
				GridSlot.SetRow(contentButton, row);
				GridSlot.SetColumn(contentButton, col);
			
				ResourceName resource = textures[i];	
				if (resource.IsEmpty())
					continue;						
						
				imageButton.GetImageWidget().LoadImageTexture(0, textures[i]);	
				imageButton.Resize(ICONS_SCALE);
				imageButton.m_OnClicked.Insert(SetGroupFlag);
				imageButton.SetImageID(i);
				imageButton.SetIsFromImageset(false);
				
				col++;
			}			
		}	
		
		if (!imageSet.IsEmpty())
		{
			array<string> flagNames = {};
			playerFaction.GetFlagNames(flagNames);
		
			for(int i = 0; i < flagNames.Count(); i++)
			{		
				Widget contentButton = GetGame().GetWorkspace().CreateWidgets(widget, content);		
				if (!contentButton)
					continue;
		
				SCR_GroupFlagImageComponent imageButton = SCR_GroupFlagImageComponent.Cast(contentButton.FindHandler(SCR_GroupFlagImageComponent));	
				if (!imageButton)
					continue;
						
				if (col > maxColumns)
				{
					row++;
					col = 1;
				}
			
				GridSlot.SetRow(contentButton, row);
				GridSlot.SetColumn(contentButton, col);
			
				ResourceName resource = playerFaction.GetGroupFlagImageSet();	
				if (resource.IsEmpty())
					continue;						
						
				imageButton.GetImageWidget().LoadImageFromSet(0, resource, flagNames[i]);
				imageButton.Resize(ICONS_SCALE);
				imageButton.m_OnClicked.Insert(SetGroupFlag);
				imageButton.SetImageID(i);
				imageButton.SetImageSet(flagNames[i]);
				imageButton.SetIsFromImageset(true);
				col++;
			}
		}
					
		
		Widget cancelButton = GetRootWidget().FindAnyWidget("Cancel");		
		
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.Cast(cancelButton.FindHandler(SCR_InputButtonComponent));		
		cancel.m_OnClicked.Insert(CloseDialog);		
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupFlag(SCR_ButtonBaseComponent button)
	{		
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();		
		if(!groupManager)
			return;
			
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();	
		if (!playerGroupController)
			return;
			
		SCR_AIGroup group =  groupManager.FindGroup(playerGroupController.GetGroupID());		
		if (!group)
			return;
				
		SCR_GroupFlagImageComponent imageButton = SCR_GroupFlagImageComponent.Cast(button.GetRootWidget().FindHandler(SCR_GroupFlagImageComponent));	
		if (!imageButton)
			return;
		
		bool isFromImageset = imageButton.GetIsFromImageset();
		playerGroupController.RequestSetGroupFlag(group.GetGroupID(), imageButton.GetImageID(), isFromImageset);			
				
		CloseAnimated();				
	}
}