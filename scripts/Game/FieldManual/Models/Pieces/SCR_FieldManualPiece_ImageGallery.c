[BaseContainerProps(), SCR_BaseContainerStaticTitleField(customTitle: "Images Gallery")]
class SCR_FieldManualPiece_ImageGallery : SCR_FieldManualPiece
{
	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_EFieldManual_ImageGalleryType.GALLERY_HORIZONTAL), uiwidget: UIWidgets.ComboBox, enums: SCR_Enum.GetList(SCR_EFieldManual_ImageGalleryType))]
	protected SCR_EFieldManual_ImageGalleryType m_eType;

	[Attribute()]
	protected string m_sText;

	[Attribute()]
	protected ref array<ref SCR_FieldManual_ImageData> m_aImages;

	[Attribute(defvalue: "{68553AA626CD1A40}UI/layouts/Menus/FieldManual/Pieces/FieldManual_Piece_ImageGallery.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	protected ResourceName m_Layout;

	protected SCR_FieldManual_ImageGalleryComponent m_ImageGalleryComponent;

	//------------------------------------------------------------------------------------------------
	override void CreateWidget(notnull Widget parent)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();

		Widget createdWidget = workspace.CreateWidgets(m_Layout, parent);
		if (!createdWidget)
		{
			Print("could not create Image Gallery widget | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return;
		}

		m_ImageGalleryComponent = SCR_FieldManual_ImageGalleryComponent.Cast(createdWidget.FindHandler(SCR_FieldManual_ImageGalleryComponent));
		if (!m_ImageGalleryComponent)
			return;

		m_ImageGalleryComponent.Init(m_eType, m_sText, m_aImages);
		if (GetGame().OnInputDeviceIsGamepadInvoker())
			GetGame().OnInputDeviceIsGamepadInvoker().Insert(m_ImageGalleryComponent.Rebuild);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_FieldManualPiece_ImageGallery()
	{
		if (!m_aImages) // can be config-provided
			m_aImages = {};
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_FieldManualPiece_ImageGallery()
	{
		if (m_ImageGalleryComponent && GetGame().OnInputDeviceIsGamepadInvoker())
			GetGame().OnInputDeviceIsGamepadInvoker().Remove(m_ImageGalleryComponent.Rebuild);
	}
}
