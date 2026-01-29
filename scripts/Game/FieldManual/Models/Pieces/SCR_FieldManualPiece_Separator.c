[BaseContainerProps(insertable: false), SCR_BaseContainerCustomTitleResourceName("m_ImagePath", true, "Image: %1")]
class SCR_FieldManualPiece_Separator : SCR_FieldManualPiece
{
	[Attribute(defvalue: "20", uiwidget: UIWidgets.Slider, params: "0 100 10")]
	protected int m_iPaddingTop;

	[Attribute(defvalue: "75", uiwidget: UIWidgets.Slider, params: "0 100 5")]
	protected int m_iWidthPercentage;

	[Attribute(defvalue: "2", uiwidget: UIWidgets.Slider, params: "0 10 1")]
	protected int m_iThickness;

	[Attribute(defvalue: "20", uiwidget: UIWidgets.Slider, params: "0 100 10")]
	protected int m_iPaddingBottom;

	[Attribute(defvalue: "{B998E3D32F62BE60}UI/layouts/Menus/FieldManual/Pieces/FieldManual_Piece_Separator.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	protected ResourceName m_Layout;

	//------------------------------------------------------------------------------------------------
	override void CreateWidget(notnull Widget parent)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();

		Widget createdWidget = workspace.CreateWidgets(m_Layout, parent);
		if (!createdWidget)
		{
			Print("could not create image widget | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return;
		}

		LayoutSlot.SetPadding(createdWidget, 0, m_iPaddingTop, 0, m_iPaddingBottom);

		ImageWidget separator = ImageWidget.Cast(createdWidget.FindAnyWidget("Separator"));
		if (!separator)
		{
			return;
		}

		if (m_iWidthPercentage < 1)
		{
			m_iThickness = 0;
		}
		separator.SetSize(32, m_iThickness);

		FrameWidget spaceLeft = FrameWidget.Cast(createdWidget.FindAnyWidget("SpaceLeft"));
		FrameWidget spaceRight = FrameWidget.Cast(createdWidget.FindAnyWidget("SpaceRight"));
		if (spaceLeft != null && spaceRight != null)
		{
			float remainder = (100.0 - m_iWidthPercentage) / 2;
			LayoutSlot.SetFillWeight(spaceLeft, remainder);
			LayoutSlot.SetFillWeight(separator, m_iWidthPercentage);
			LayoutSlot.SetFillWeight(spaceRight, remainder);
		}
	}
}
