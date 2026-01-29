[BaseContainerProps(insertable: false), SCR_BaseContainerCustomTitleResourceName("m_ImagePath", true, "Image: %1")]
class SCR_FieldManualPiece_Image : SCR_FieldManualPiece
{
	protected static const int IMAGE_INDEX = 0;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "edds imageset")]
	protected ResourceName m_ImagePath;

	[Attribute()]
	protected string m_sImageSetName;

	[Attribute(defvalue: "1 1 1 1")]
	ref Color m_ImageColor;

	[Attribute()]
	protected string m_sCaption;

	[Attribute(defvalue: "{68553AA626CD1A41}UI/layouts/Menus/FieldManual/Pieces/FieldManual_Piece_Image.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	protected ResourceName m_Layout;
	
	[Attribute(defvalue: PlatformKind.PSN.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Platform selected here will not show this element", enums: ParamEnumArray.FromEnum(PlatformKind))]
	protected PlatformKind m_eExcludedPlatform;

	//------------------------------------------------------------------------------------------------
	override void CreateWidget(notnull Widget parent)
	{
		if (m_ImagePath.IsEmpty() && m_sCaption.IsEmpty())
		{
			Print("Empty image entry, skipping widget creation | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.DEBUG);
			return;
		}

		WorkspaceWidget workspace = GetGame().GetWorkspace();

		Widget createdWidget = workspace.CreateWidgets(m_Layout, parent);
		if (!createdWidget)
		{
			Print("could not create image widget | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return;
		}

		ImageWidget imageWidget = ImageWidget.Cast(createdWidget.FindAnyWidget("Image"));
		if (imageWidget)
		{
			if (!m_ImagePath.IsEmpty())
			{
				if (m_ImagePath.EndsWith("imageset") && !m_sImageSetName.IsEmpty())
				{
					imageWidget.LoadImageFromSet(IMAGE_INDEX, m_ImagePath, m_sImageSetName);
				}
				else
				{
					imageWidget.LoadImageTexture(IMAGE_INDEX, m_ImagePath);
				}
				SCR_WidgetHelper.ResizeToImage(imageWidget, IMAGE_INDEX);
				if (m_ImageColor)
					imageWidget.SetColor(m_ImageColor);
			}
		}

		TextWidget textWidget = TextWidget.Cast(createdWidget.FindAnyWidget("Caption"));
		if (textWidget)
			textWidget.SetText(m_sCaption);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetImagePath()
	{
		return m_ImagePath;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetSetName()
	{
		return m_sImageSetName;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	Color GetImageColor()
	{
		return Color.FromInt(m_ImageColor.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetCaption()
	{
		return m_sCaption;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanCreateWidget()
	{
		PlatformKind localPlatform = GetGame().GetPlatformService().GetLocalPlatformKind();
		
		if (localPlatform == m_eExcludedPlatform && localPlatform != PlatformKind.NONE)
			return false;
		
		return super.CanCreateWidget();
	}
}
