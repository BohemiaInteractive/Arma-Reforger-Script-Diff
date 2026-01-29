[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("m_sText", "Header: %1")]
class SCR_FieldManualPiece_Header : SCR_FieldManualPiece
{
	protected static const int IMAGE_INDEX = 0;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "edds imageset")]
	protected ResourceName m_ImagePath;

	[Attribute()]
	protected string m_sImageSetName;

	[Attribute(uiwidget: UIWidgets.EditBoxWithButton)]
	string m_sText;

	[Attribute(defvalue: "{B6E3D069C489E6EF}UI/layouts/Menus/FieldManual/Pieces/FieldManual_Piece_Header.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	ResourceName m_Layout;

	//------------------------------------------------------------------------------------------------
	override void CreateWidget(notnull Widget parent)
	{
		Widget createdWidget = GetGame().GetWorkspace().CreateWidgets(m_Layout, parent);
		if (!createdWidget)
		{
			Print(string.Format("created widget (%2:%1) was null | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, m_Layout, m_sText), LogLevel.DEBUG);
			return;
		}

		TextWidget textWidget = TextWidget.Cast(createdWidget);
		if (!textWidget)
		{
			textWidget = TextWidget.Cast(SCR_WidgetHelper.GetWidgetOrChild(createdWidget, "Text"));
		}

		if (!textWidget)
		{
			Print("created widget could not be set text | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.DEBUG);
			return;
		}

		textWidget.SetText(m_sText);

		ImageWidget imageWidget = ImageWidget.Cast(createdWidget.FindAnyWidget("Image"));
		if (imageWidget != null)
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
			}
			else
			{
				imageWidget.GetParent().SetVisible(false); //--- ToDo: Cleaner hierarchy
			}
		}

	}
}
