class SCR_Credits: ChimeraMenuBase
{
	protected const int NUMBER_OF_TEXTWIDGETS = 6;
	protected const int NAME_TIMEOUT = 5000; //How long will names appear on screen 5000
	protected const int NAME_CHANGE_DELAY = 3000; //Delay between names 3000
	protected const float NAME_ANIMATION_TIME = 0.5; //How long will fadeout animation 
	protected const float NAME_ANIMATION_CREDITS = 0.6;
	protected const int BACKGROUND_CHANGE_INTERVAL = 10000;
	protected static const ref array<int> PAGE_NAME_SIZE = { 14, 12, 10, 9, 8, 7, 7 };
	
	protected ResourceName m_sNamesLists = "{3D0DE92D54AEA7D9}Configs/Credits/credits.conf";
	protected ResourceName m_sMusicFile = "{27FAA69A3BD0473C}Sounds/Music/Ingame/Samples/MU_MilitaryBasePositive_BEYOND THE VEIL v2.wav";
	private ref array<ref SCR_CreditsDepartmentHeader> m_aCreditsNameList = {};
	private ref array<ResourceName> m_aCreditsBackgrounds = {};
	private ResourceName m_CurrentBackground;
	protected Widget m_wFooter;
	protected Widget m_wCreditsMain;
	protected ImageWidget m_wBackgroundImage;
	protected ImageWidget m_wSideVignette;
	protected TextWidget m_wCreditsText;
	protected ref array<ref RichTextWidget> m_aDepartments = {};
	protected ref array<ref RichTextWidget> m_aNames = {};
	protected bool m_bLastCycle;
	protected RichTextWidget m_wLastDepartment;
	protected RichTextWidget m_wLastText;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		m_wFooter = GetRootWidget().FindAnyWidget("Footer");
		m_wBackgroundImage = ImageWidget.Cast(GetRootWidget().FindAnyWidget("BackgroundImage"));
		m_wSideVignette = ImageWidget.Cast(GetRootWidget().FindAnyWidget("BackgroundVignette"));
		m_wCreditsText = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Credits"));
		m_wCreditsMain = GetRootWidget().FindAnyWidget("VerticalLayoutOP");
		RichTextWidget dept = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("DeptName"));
		RichTextWidget name = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Names"));
		Widget holder = GetRootWidget().FindAnyWidget("lastFrame");
		m_wLastDepartment = RichTextWidget.Cast(holder.FindAnyWidget("DeptName"));
		m_wLastText = RichTextWidget.Cast(holder.FindAnyWidget("Names"));
		
		m_bLastCycle = false;
		m_aDepartments.Insert(dept);
		m_aNames.Insert(name);
		
		//Set up text widgets for later use
		
		for(int i = 0; i < NUMBER_OF_TEXTWIDGETS; i++)
		{
			dept = RichTextWidget.Cast(name.GetParent().GetSibling().FindAnyWidget("DeptName"));
			name = RichTextWidget.Cast(name.GetParent().GetSibling().FindAnyWidget("Names"));
			m_aDepartments.Insert(dept);
			m_aNames.Insert(name);
		}
		
		//Make them invisible TODO: Change in Widget
		
		for(int i, count = m_aDepartments.Count(); i<count; i++)
		{
			m_aDepartments[i].GetParent().SetVisible(false);
		}
		
		super.OnMenuOpen();
		// Subscribe to buttons
		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, m_wFooter);
		if (back)
			back.m_OnActivated.Insert(EndCredits);
		
		SCR_InputButtonComponent licenses = SCR_InputButtonComponent.GetInputButtonComponent("Licenses", m_wFooter);
		if (licenses)
			licenses.m_OnActivated.Insert(OnLicenses);
		
		Resource container = BaseContainerTools.LoadContainer(m_sNamesLists);
		if (container && container.IsValid())
		{
			SCR_CreditsHeader list = SCR_CreditsHeader.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer()));
			if(list)
			{
				list.GetCreditsDepartmentList(m_aCreditsNameList);
				list.GetBackgrounds(m_aCreditsBackgrounds);
			}
		}
		StartCredits();
	}
	
	//------------------------------------------------------------------------------------------------
	void StartCredits()
	{
		ChangeBackground();
		
		AnimateWidget.Opacity(m_wBackgroundImage, 1, NAME_ANIMATION_TIME);
		AnimateWidget.Opacity(m_wSideVignette, 1, NAME_ANIMATION_CREDITS);
		AnimateWidget.Opacity(m_wCreditsText, 1, NAME_ANIMATION_CREDITS);
		GetGame().GetCallqueue().CallLater(ShowNames, 3000,false,0,0);
		GetGame().GetCallqueue().CallLater(ChangeBackground, BACKGROUND_CHANGE_INTERVAL, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeBackground()
	{
		if (!m_CurrentBackground)
			m_CurrentBackground = m_aCreditsBackgrounds.GetRandomElement();
		else
		{
			int backgroundIndex = m_aCreditsBackgrounds.Find(m_CurrentBackground);
			backgroundIndex++;
			if (m_aCreditsBackgrounds.Count() == backgroundIndex)
				backgroundIndex = 0;
			
			m_CurrentBackground = m_aCreditsBackgrounds.Get(backgroundIndex);
		}
		
		m_wBackgroundImage.LoadImageTexture(0, m_CurrentBackground);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowNames(int departments, int names)
	{
		//Initial variables and an array
		
		for(int i, count = m_aDepartments.Count(); i<count; i++)
		{
			m_aDepartments[i].GetParent().SetVisible(false);
		}
		
		string deptName;
		int deptCount = 0;
		int nameCount = 0;
		int cycleCount = 0;
		int counter = 0;
		string nameGlobal;
		ref array<ref SCR_CreditsPersonName> CreditsNames = {};
		
		if (departments == m_aCreditsNameList.Count())
		{
			EndCredits();
			return;
		}
		
		//Master FOR cycle for going throught departments. This BREAKS when there is no more free space on screen
		for (int x = departments, count = m_aCreditsNameList.Count(); x < count; x++)
		{	
			//Get the department name and set it, also get number of names
			deptName = m_aCreditsNameList[x].GetDeptName();
			CreditsNames.Clear();
			m_aCreditsNameList[x].GetCreditsNamesList(CreditsNames);
			nameCount = nameCount + CreditsNames.Count();
			
			counter = 0;
			
			//If there are more names from the previous screen to be shown now, subtract the number of names from previous screen to be shown
			
			if (names > 0)
			{
				counter = CreditsNames.Count() - names;
				nameCount -= counter;
				names = 0;
			}
			
			//If there are more names in this department than there is screen space, save the number of names to be shown on the next screen
			
			if (nameCount > PAGE_NAME_SIZE[cycleCount])
				names = nameCount - PAGE_NAME_SIZE[cycleCount];

			if (!m_bLastCycle)
			{
				m_aDepartments[cycleCount].GetParent().SetVisible(true);
				m_aDepartments[cycleCount].SetText(deptName);
			}
			else
			{
				m_wLastDepartment.GetParent().SetVisible(true);
				m_wLastDepartment.SetText(deptName);
			}
			
			//for cycle for getting all names from department to one string
			nameGlobal = "";
			
			for (int i = counter, countx = CreditsNames.Count(); i < countx - names; i++)
			{
				if (i<0)
					i = 0;
				
				nameGlobal =  nameGlobal + CreditsNames[i].GetPersonName() + "<br/>";	
			}
			
			//if there are no names to be shown, dont make the dept. name visible
			
			if (nameGlobal == "")
				m_aDepartments[cycleCount].GetParent().SetVisible(false);
			
			if (m_bLastCycle)
			{
				m_wLastText.SetText(nameGlobal)
			}
			else
				m_aNames[cycleCount].SetText(nameGlobal);
			
			//if there are names to be shown, there is no more space on screen. Break loop and change screens.
			
			deptCount = x;
			if (names > 0)
				break;
			//If there are no names, show next department
			deptCount++;

			//If there are exactly the number of names to be shown, next screen
			if (nameCount == PAGE_NAME_SIZE[cycleCount])
				break;
			
			//If the loop is over, end it
			
			if (departments >= m_aCreditsNameList.Count())
			{
				EndCredits();
			}

			cycleCount++;
			
			if (departments >= m_aCreditsNameList.Count() - 2)
			{
				m_bLastCycle = true;
				break;
			}
			
			if(!m_bLastCycle)
			{	
				deptName = m_aCreditsNameList[deptCount].GetDeptName();
			
				if (deptName.Contains("EnfLead") || deptName.Contains("Associate") || deptName.Contains("Community") || deptName.Contains("Platform") || deptName.Contains("LeadProducer"))
					continue;
				else
					if (deptName.Contains("Lead") || deptName.Contains("Composer") || deptName.Contains("MarketingProduction") || deptName.Contains("Consultants") || deptName.Contains("Manager") || deptName.Contains("Photographer") || deptName.Contains("CEO") || deptName.Contains("AnimationOutsource") || deptName.Contains("ReforgerBabies") || deptName.Contains("LegalNotices"))
						break;
			}
		}
		//Switch to next slide with animation
		AnimateWidget.Opacity(m_wCreditsMain, 1, NAME_ANIMATION_TIME);
		GetGame().GetCallqueue().CallLater(HideNames,NAME_TIMEOUT,false,deptCount,names);
	}
	
	//------------------------------------------------------------------------------------------------
	void HideNames(int departments, int names)
	{
		AnimateWidget.Opacity(m_wCreditsMain, 0, NAME_ANIMATION_TIME);
		GetGame().GetCallqueue().CallLater(ShowNames,NAME_CHANGE_DELAY,false,departments,names);
	}
	
	//------------------------------------------------------------------------------------------------
	void EndCredits()
	{
		GetGame().GetCallqueue().Remove(ShowNames);
		GetGame().GetCallqueue().Remove(HideNames);
		GetGame().GetCallqueue().Remove(ChangeBackground);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CreditsMenu)
	}
	
	void OnLicenses()
	{
		//GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CreditsMenu);
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CreditsLicensesMenu);
		
	}
};
