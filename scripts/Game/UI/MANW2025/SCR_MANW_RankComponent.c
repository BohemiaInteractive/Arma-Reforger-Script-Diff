class SCR_MANW_RankComponent : SCR_ScriptedWidgetComponent
{
	protected const string IMAGESET_AWARD 	= "{402BB92E44897D60}UI/Textures/MANW2025/CupRanks.imageset";
	
	protected ref SCR_MANW_RankLabelWidgets m_Widgets = new SCR_MANW_RankLabelWidgets();

	[Attribute("")]
	protected ref array<ref SCR_MANW_RankCategoryData> m_aAwardCategories;
	protected ref map<string, string> m_MapAwardCategories;

	[Attribute("")]
	protected ref array<ref SCR_MANW_RankIconData> m_aAwardIcons;
	
	[Attribute("")]
	protected ref array<ref SCR_MANW_RankTagCategoryData> m_aTagCategoryData;
	protected ref map<string, string> m_MapTagCategoryData;
	
	[Attribute("")]
	protected ref array<ref SCR_MANW_RankTagRankData> m_aTagRankData;
	protected ref map<string, int> m_MapTagRankData;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (!GetGame().InPlayMode())
			return;

		m_Widgets.Init(m_wRoot);
		m_wRoot.SetVisible(false);

		m_MapAwardCategories = new map<string, string>();
		foreach (SCR_MANW_RankCategoryData rankCategory : m_aAwardCategories)
		{
			m_MapAwardCategories.Insert(rankCategory.m_sIdentifier, rankCategory.m_sName);
		}
		
		m_MapTagCategoryData = new map<string, string>();
		foreach (SCR_MANW_RankTagCategoryData tagCategoryData : m_aTagCategoryData)
		{
			m_MapTagCategoryData.Insert(tagCategoryData.m_sTagName, tagCategoryData.m_sCategoryName);
		}
		
		m_MapTagRankData = new map<string, int>();
		foreach (SCR_MANW_RankTagRankData tagRankData : m_aTagRankData)
		{
			m_MapTagRankData.Insert(tagRankData.m_sTagName, tagRankData.m_sRankId);
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdateAwards(notnull SCR_WorkshopItem workshopItem)
	{
		string category;
		int place;
		if (!GetAwardData(workshopItem, category, place))
		{
			m_wRoot.SetVisible(false);
		}
		else
		{
			m_wRoot.SetVisible(true);
	
			m_Widgets.m_wRankIcon.LoadImageFromSet(0, IMAGESET_AWARD, m_aAwardIcons[place - 1].m_sName);
			m_Widgets.m_wRankIcon.SetColor(m_aAwardIcons[place - 1].m_Color);
	
			m_Widgets.m_wCategoryText.SetText(m_MapAwardCategories.Get(category));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetAwardData(notnull SCR_WorkshopItem workshopItem, out string category, out int place)
	{
		bool hasCategory, hasRank;
		string categoryOutput;
		int placeOutput;
		array<WorkshopTag> items = {};
		workshopItem.GetWorkshopItem().GetTags(items);
		foreach (WorkshopTag item : items)
		{			
			if (!hasCategory)
			{
				if (m_MapTagCategoryData.Find(item.Name(), categoryOutput))
				{
					category = categoryOutput;
					hasCategory = true;
				}
			}
			
			if (!hasRank)
			{
				if (m_MapTagRankData.Find(item.Name(), placeOutput))
				{
					place = placeOutput;
					hasRank = true;
				}
			}
			
			if (hasCategory && hasRank)
				return true;
		}

		return false;
	}
}

[BaseContainerProps(configRoot : true)]
class SCR_MANW_RankCategoryData
{
	[Attribute("")]
	string m_sIdentifier;

	[Attribute("")]
	string m_sName;
}

[BaseContainerProps(configRoot : true)]
class SCR_MANW_RankIconData
{
	[Attribute("")]
	string m_sName;

	[Attribute("")]
	ref Color m_Color;
}

[BaseContainerProps(configRoot : true)]
class SCR_MANW_RankTagCategoryData
{
	[Attribute("")]
	string m_sTagName;
	
	[Attribute("")]
	string m_sCategoryName;
}

[BaseContainerProps(configRoot : true)]
class SCR_MANW_RankTagRankData
{
	[Attribute("")]
	string m_sTagName;
	
	[Attribute("")]
	int m_sRankId;
}