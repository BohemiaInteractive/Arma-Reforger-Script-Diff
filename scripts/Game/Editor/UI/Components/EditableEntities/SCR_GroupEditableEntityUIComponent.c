class SCR_GroupEditableEntityUIComponent : SCR_BaseEditableEntityUIComponent
{
	[Attribute()]
	protected ref array<ref SCR_GroupEditableEntityTextureUIComponent> m_Textures;
	
	[Attribute()]
	protected string m_sIconWidgetName;
	
	protected SCR_MilitarySymbolUIComponent m_SymbolUI;
	
	//------------------------------------------------------------------------------------------------
	protected bool RefreshSymbol(SCR_EditableEntityComponent entity)
	{
		SCR_GroupIdentityComponent identity = SCR_GroupIdentityComponent.Cast(entity.GetOwner().FindComponent(SCR_GroupIdentityComponent));
		if (identity)
			return RefreshSymbol(identity.GetMilitarySymbol());
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool RefreshSymbol(SCR_UIInfo info)
	{
		SCR_EditableGroupUIInfo groupInfo = SCR_EditableGroupUIInfo.Cast(info);
		if (groupInfo)
			return RefreshSymbol(groupInfo.GetMilitarySymbol());
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool RefreshSymbol(SCR_MilitarySymbol symbol)
	{
		if (!symbol)
			return false;
		
		if (m_SymbolUI)
			m_SymbolUI.Update(symbol);
		
		if (!m_Textures.IsEmpty())
		{
			ImageWidget iconWidget = ImageWidget.Cast(GetWidget().FindAnyWidget(m_sIconWidgetName));
			if (iconWidget)
			{
				bool hasTexture;
				foreach (SCR_GroupEditableEntityTextureUIComponent texture: m_Textures)
				{
					if (texture.m_Identity == symbol.GetIdentity() && texture.m_Dimension == symbol.GetDimension())
					{
						string ext;
						FilePath.StripExtension(texture.m_Texture, ext);
						if (ext == "imageset")
							iconWidget.LoadImageFromSet(0, texture.m_Texture, texture.m_ImageName);
						else
							iconWidget.LoadImageTexture(0, texture.m_Texture);
						
						iconWidget.SetVisible(true);
						hasTexture = true;
						break;
					}
				}
				iconWidget.SetVisible(hasTexture);
			}
		}
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnRefresh(SCR_EditableEntityBaseSlotUIComponent slot)
	{
		RefreshSymbol(m_Entity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_EditableEntityComponent entity, SCR_UIInfo info, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		super.OnInit(entity, info, slot);
		
		m_SymbolUI = SCR_MilitarySymbolUIComponent.Cast(GetWidget().FindHandler(SCR_MilitarySymbolUIComponent));
		
		if (entity && RefreshSymbol(entity))
			return;
		
		RefreshSymbol(info);
	}
}

[BaseContainerProps(), SCR_BaseContainerMilitaryLayout()]
class SCR_GroupEditableEntityTextureUIComponent
{
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolIdentity))]
	EMilitarySymbolIdentity m_Identity;
	
	[Attribute("1", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolDimension))]
	EMilitarySymbolDimension m_Dimension;
	
	[Attribute(uiwidget:UIWidgets.ResourcePickerThumbnail, params: "edds imageset")]
	ResourceName m_Texture;
	
	[Attribute()]
	string m_ImageName;
}
