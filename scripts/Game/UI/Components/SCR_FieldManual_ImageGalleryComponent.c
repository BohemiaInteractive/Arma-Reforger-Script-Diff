enum SCR_EFieldManual_ImageGalleryType
{
	ICONS_VERTICAL,
	ICONS_LIST,
	GALLERY_HORIZONTAL,
	GALLERY_VERTICAL,
}

class SCR_FieldManual_ImageGalleryComponent : ScriptedWidgetComponent
{
	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_EFieldManual_ImageGalleryType.GALLERY_HORIZONTAL), uiwidget: UIWidgets.ComboBox, enums: SCR_Enum.GetList(SCR_EFieldManual_ImageGalleryType))]
	protected SCR_EFieldManual_ImageGalleryType m_eType;

	[Attribute()]
	protected ref array<ref SCR_FieldManual_ImageGalleryLayoutInfo> m_aEnumLayoutPairs;

	[Attribute()]
	protected ref array<ref SCR_FieldManual_ImageData> m_aImagesData;

	[Attribute()]
	protected string m_sText;

	protected Widget m_wRoot;
	protected Widget m_wGallery;
	protected static const string S_IMAGES_PARENT_WIDGET_NAME = "Images";
	protected static const string S_IMAGE_WIDGET_NAME = "Image";
	protected static const string S_CAPTION_WIDGET_NAME = "Caption";
	protected static const string S_TEXT_WIDGET_NAME = "Text";

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_FieldManual_ImageGalleryComponent()
	{
		if (!m_aEnumLayoutPairs)
			m_aEnumLayoutPairs = {};

		AddMissingEnumLayoutPairs();

		if (!m_aImagesData)
			m_aImagesData = {};
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wRoot = w;
		Rebuild(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] type
	//! \param[in] text
	//! \param[in] imagesData
	void Init(SCR_EFieldManual_ImageGalleryType type, string text, array<ref SCR_FieldManual_ImageData> imagesData)
	{
		m_eType = type;
		m_sText = text;
		if (imagesData)
			m_aImagesData = imagesData;

		Rebuild(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}

	//------------------------------------------------------------------------------------------------
	//!
	void ClearImages()
	{
		m_aImagesData.Clear();
		Rebuild(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] imagesData
	void SetImages(array<ref SCR_FieldManual_ImageData> imagesData)
	{
		if (!imagesData || imagesData.IsEmpty())
			return;

		m_aImagesData = imagesData;
		Rebuild(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] text
	void SetText(string text)
	{
		m_sText = text;

		if (!m_wGallery)
			return;

		TextWidget textWidget = TextWidget.Cast(m_wGallery.FindAnyWidget(S_TEXT_WIDGET_NAME));
		if (textWidget)
			textWidget.SetText(m_sText);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] type
	void SetType(SCR_EFieldManual_ImageGalleryType type)
	{
		m_eType = type;
		Rebuild(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}

	//------------------------------------------------------------------------------------------------
	//! Delete and recreate all child widgets
	//! Used e.g on input changes (keyboard/controller)
	void Rebuild(bool isGamepad)
	{
		if (!m_wRoot || !m_aEnumLayoutPairs)
			return;

		SCR_FieldManual_ImageGalleryLayoutInfo galleryInfo;
		foreach (SCR_FieldManual_ImageGalleryLayoutInfo layoutInfo : m_aEnumLayoutPairs)
		{
			if (layoutInfo && layoutInfo.m_eType == m_eType)
			{
				galleryInfo = layoutInfo;
				break;
			}
		}

		if (!galleryInfo || galleryInfo.m_sGalleryLayout.IsEmpty() || galleryInfo.m_sImageLayout.IsEmpty())
			return;

		SCR_WidgetHelper.RemoveAllChildren(m_wRoot);

		m_wGallery = GetGame().GetWorkspace().CreateWidgets(galleryInfo.m_sGalleryLayout, m_wRoot);
		if (!m_wGallery)
			return;

		TextWidget textWidget = TextWidget.Cast(m_wGallery.FindAnyWidget(S_TEXT_WIDGET_NAME));
		if (textWidget)
			textWidget.SetText(m_sText);

		Widget imagesParent = m_wGallery.FindAnyWidget(S_IMAGES_PARENT_WIDGET_NAME);
		if (!imagesParent)
			return;

		array<SCR_EInputTypeCondition> conditions = { SCR_EInputTypeCondition.ALL_INPUTS };
		if (isGamepad)
			conditions.Insert(SCR_EInputTypeCondition.GAMEPAD_ONLY);
		else
			conditions.Insert(SCR_EInputTypeCondition.KEYBOARD_ONLY);

		Widget image;
		foreach (SCR_FieldManual_ImageData imageData : m_aImagesData)
		{
			if (!conditions.Contains(imageData.m_eInputDisplayCondition))
				continue;

			image = GetGame().GetWorkspace().CreateWidgets(galleryInfo.m_sImageLayout, imagesParent);
			if (!image)
				continue;

			SetImageData(image, imageData);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetImageData(Widget widget, SCR_FieldManual_ImageData imageData)
	{
		ImageWidget imageWidget = ImageWidget.Cast(SCR_WidgetHelper.GetWidgetOrChild(widget, S_IMAGE_WIDGET_NAME));
		if (imageWidget && !imageData.m_Image.IsEmpty())
		{
			if (imageData.m_sImageName.IsEmpty())
			{
				imageWidget.LoadImageTexture(0, imageData.m_Image);
			}
			else
			{
				imageWidget.LoadImageFromSet(0, imageData.m_Image, imageData.m_sImageName);
			}
			SCR_WidgetHelper.ResizeToImage(imageWidget);
		}

		TextWidget captionWidget = TextWidget.Cast(widget.FindAnyWidget(S_CAPTION_WIDGET_NAME));
		if (captionWidget)
			captionWidget.SetText(imageData.m_sCaption);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddMissingEnumLayoutPairs()
	{
		set<SCR_EFieldManual_ImageGalleryType> foundEnums = new set<SCR_EFieldManual_ImageGalleryType>();
		foreach (SCR_FieldManual_ImageGalleryLayoutInfo pair : m_aEnumLayoutPairs)
		{
			foundEnums.Insert(pair.m_eType);
		}

		if (!foundEnums.Contains(SCR_EFieldManual_ImageGalleryType.ICONS_VERTICAL))
		{
			SCR_FieldManual_ImageGalleryLayoutInfo info = new SCR_FieldManual_ImageGalleryLayoutInfo();
			info.m_eType = SCR_EFieldManual_ImageGalleryType.ICONS_VERTICAL;
			info.m_sGalleryLayout = "{912B608D2650F8B9}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/ICONS_VERTICAL_ImageGallery.layout";
			info.m_sImageLayout = "{BF790BE149822E0F}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/ICONS_VERTICAL_Image.layout";
			m_aEnumLayoutPairs.Insert(info);
		}

		if (!foundEnums.Contains(SCR_EFieldManual_ImageGalleryType.ICONS_LIST))
		{
			SCR_FieldManual_ImageGalleryLayoutInfo info = new SCR_FieldManual_ImageGalleryLayoutInfo();
			info.m_eType = SCR_EFieldManual_ImageGalleryType.ICONS_LIST;
			info.m_sGalleryLayout = "{B4DF69D7D553CFBC}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/ICONS_LIST_ImageGallery.layout";
			info.m_sImageLayout = "{3194AFE59F1B877E}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/ICONS_LIST_Image.layout";
			m_aEnumLayoutPairs.Insert(info);
		}

		if (!foundEnums.Contains(SCR_EFieldManual_ImageGalleryType.GALLERY_HORIZONTAL))
		{
			SCR_FieldManual_ImageGalleryLayoutInfo info = new SCR_FieldManual_ImageGalleryLayoutInfo();
			info.m_eType = SCR_EFieldManual_ImageGalleryType.GALLERY_HORIZONTAL;
			info.m_sGalleryLayout = "{D8E4A2905F167C53}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/GALLERY_HORIZONTAL_ImageGallery.layout";
			info.m_sImageLayout = "{D08B8D5E3209D733}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/GALLERY_HORIZONTAL_Image.layout";
			m_aEnumLayoutPairs.Insert(info);
		}

		if (!foundEnums.Contains(SCR_EFieldManual_ImageGalleryType.GALLERY_VERTICAL))
		{
			SCR_FieldManual_ImageGalleryLayoutInfo info = new SCR_FieldManual_ImageGalleryLayoutInfo();
			info.m_eType = SCR_EFieldManual_ImageGalleryType.GALLERY_VERTICAL;
			info.m_sGalleryLayout = "{856A53946CA73978}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/GALLERY_VERTICAL_ImageGallery.layout";
			info.m_sImageLayout = "{D077D7EA510D2FC5}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/GALLERY_VERTICAL_Image.layout";
			m_aEnumLayoutPairs.Insert(info);
		}
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(enumType: SCR_EFieldManual_ImageGalleryType, propertyName: "m_eType", format: "Image Gallery %1")]
class SCR_FieldManual_ImageGalleryLayoutInfo
{
	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_EFieldManual_ImageGalleryType.GALLERY_HORIZONTAL), uiwidget: UIWidgets.ComboBox, enums: SCR_Enum.GetList(SCR_EFieldManual_ImageGalleryType))]
	SCR_EFieldManual_ImageGalleryType m_eType;

	[Attribute()]
	ResourceName m_sGalleryLayout;

	[Attribute()]
	ResourceName m_sImageLayout;
}
