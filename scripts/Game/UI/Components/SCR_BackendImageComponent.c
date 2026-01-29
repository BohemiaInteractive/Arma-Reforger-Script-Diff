// When enabled, the component will visualize debug information
//#define BACKEND_IMAGE_DEBUG

//! This component implements basic image and loading overlay handling
//! m_sImageWidgetName must be an Image Widget
//! m_sLoadingOverlayName must have an SCR_LoadingOverlay component attached
class SCR_BackendImageComponent : SCR_ScriptedWidgetComponent
{
	// Attributes
	[Attribute("{04EB797EBF59CDEF}UI/Textures/Workshop/AddonThumbnails/workshop_defaultFallback_UI.edds", UIWidgets.ResourceNamePicker, desc: "Image used as fallback for addon image", params: "edds")]
	protected ResourceName m_sFallbackImage;
	
	[Attribute()]
	protected string m_sImageWidgetName;
	
	[Attribute("", UIWidgets.EditBox, "Widget from which we will be reading size of image widget. It can be detached from image widget, since its reported size can be different when wrapped into a scale widget.")]
	protected string m_sImageSizeWidgetName;
	
	[Attribute("1")]
	protected bool m_bShowLoadingImage;
	
	[Attribute("")]
	protected string m_sLoadingOverlayName;
	
	[Attribute("")]
	protected string m_sDebugTextName;
	
	// Widgets
	protected ImageWidget m_wImage;
	protected Widget m_wImageSize;
	protected SCR_LoadingOverlay m_LoadingOverlay;
	protected TextWidget m_wDebugText;
	
	// Other
	protected BackendImage m_BackendImage;
	protected ref SCR_WorkshopItemCallback_DownloadImage m_DownloadImageCallback;

	protected bool m_bIsWaitingForWidgetInit;
	protected int m_iPreferredWidth;	
	
	// Debugging
	protected int m_iScreenWidth;
	
	protected ref ScriptInvokerVoid m_OnImageSelected;
	
	//----------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		if (!m_sImageWidgetName.IsEmpty())
			m_wImage = ImageWidget.Cast(w.FindAnyWidget(m_sImageWidgetName));
		
		if (!m_sImageSizeWidgetName.IsEmpty())
			m_wImageSize = w.FindAnyWidget(m_sImageSizeWidgetName);
		
		if (!m_sLoadingOverlayName.IsEmpty())
		{
			Widget wLoading = w.FindAnyWidget(m_sLoadingOverlayName);
			if (wLoading)
				m_LoadingOverlay = SCR_LoadingOverlay.Cast(wLoading.FindHandler(SCR_LoadingOverlay));
		}
		
		if (!m_sDebugTextName.IsEmpty())
		{
			m_wDebugText = TextWidget.Cast(w.FindAnyWidget(m_sDebugTextName));
			
			if (m_wDebugText)
			{
				m_wDebugText.SetVisible(false);
				
				// Debug
				#ifdef BACKEND_IMAGE_DEBUG
				m_wDebugText.SetVisible(true);
				ShowDebugText("SCR_BackendImageComponent\nInitialized");
				#endif
			}
		}	
	}
	
	//----------------------------------------------------------------------------------
	protected void CheckWidgetInitialized()
	{
		if (!m_wImageSize)
			return;
			
		// Get widget size instantly, related variables might be needed for debug text
		float fWidth, fHeight;
		m_wImageSize.GetScreenSize(fWidth, fHeight);
		
		m_iScreenWidth = Math.Round(fWidth); // Store for debugging
		
		// Wait untill the parent widget has been initialized
		int preferredWidth = Math.Round(fWidth);
		if (preferredWidth == 0)
		{
			if (!m_bIsWaitingForWidgetInit)
			{				
				ShowLoadingImage();
				GetGame().GetCallqueue().CallLater(CheckWidgetInitialized, 0, true);
				
				m_bIsWaitingForWidgetInit = true;
			}
		
			#ifdef BACKEND_IMAGE_WIDGET_DEBUG
			Debug.Error(string.Format("SCR_BackendImageComponent - IsWidgetReady() - widget has no size: %1 | %2", m_wRoot, m_wRoot.GetName()));
			#endif
				
			return;
		}
		
		GetGame().GetCallqueue().Remove(CheckWidgetInitialized);
		
		m_iPreferredWidth = preferredWidth;
		
		m_bIsWaitingForWidgetInit = false;
		
		TryDownloadImage();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TryDownloadImage()
	{
		ImageScale imageScale = m_BackendImage.GetScale(m_iPreferredWidth);
		if (!imageScale)
		{
			TryLocalImage();
			return;
		}
			
		// Setup callback
		m_DownloadImageCallback = new SCR_WorkshopItemCallback_DownloadImage();
		m_DownloadImageCallback.SetOnSuccess(Callback_DownloadImage_OnSuccess);
		m_DownloadImageCallback.SetOnError(Callback_DownloadImage_OnTimeoutError);
		
		m_DownloadImageCallback.m_Scale = imageScale;

		// Dwonload
		if (imageScale.Download(m_DownloadImageCallback))
		{
			// While we are downloading, try to get a smaller local scale and show the laoding overlay
			ImageScale localImageScale = m_BackendImage.GetLocalScale(m_iPreferredWidth);
			if (localImageScale)
				ShowLoadingImage(localImageScale.Path());
			else
				ShowLoadingImage();
			
			// Debug
			#ifdef BACKEND_IMAGE_DEBUG
			ShowDebugText("Waiting");
			#endif
			
			return;
		}

		// Most likely we have already downloaded this
		string imagePath = imageScale.Path();
		if (!TryShowImage(imagePath))
			TryLocalImage();
		
		// Debug
		#ifdef BACKEND_IMAGE_DEBUG
		string dbgtxt = string.Format("Local Image: %1x%2", imageScale.Width(), imageScale.Height());
		ShowDebugText(dbgtxt);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_DownloadImage_OnTimeoutError(SCR_WorkshopItemCallback_DownloadImage callback)
	{		
		TryLocalImage();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_DownloadImage_OnSuccess(SCR_WorkshopItemCallback_DownloadImage callback)
	{		
		if (!callback.m_Scale)
		{
			ShowFallbackImage();
			return;
		}

		string imgPath = callback.m_Scale.Path();
		TryShowImage(imgPath);
		
		// Debug
		#ifdef BACKEND_IMAGE_DEBUG
		string dbgtxt = string.Format("Received %1x%2", callback.m_Scale.Width(), callback.m_Scale.Height());
		ShowDebugText(dbgtxt);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TryLocalImage()
	{
		ImageScale localImageScale = m_BackendImage.GetLocalScale(m_iPreferredWidth);
		if (localImageScale)
			TryShowImage(localImageScale.Path());
		else
			ShowFallbackImage();
	}
	
	//----------------------------------------------------------------------------------
	protected bool TryShowImage(ResourceName imagePath)
	{
		if (imagePath.IsEmpty())
		{
			ShowFallbackImage();
			return false;
		}

		bool loaded = TryLoadImageTexture(imagePath);
		
		if (!loaded)
			ShowFallbackImage();
		else
			m_wImage.SetVisible(true);
		
		return loaded;
	}
	
	//----------------------------------------------------------------------------------
	protected void ShowLoadingImage(ResourceName loadingImage = string.Empty)
	{
		if (!m_wImage)
			return;
		
		if (loadingImage.IsEmpty())
			m_wImage.SetVisible(false);
		else if (m_bShowLoadingImage)
			TryShowImage(loadingImage);
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(true);
	}
	
	//----------------------------------------------------------------------------------
	//! If no fallback is set, the image widget will be hidden instead
	protected void ShowFallbackImage()
	{
		if (!m_wImage)
			return;
		
		if (m_sFallbackImage.IsEmpty())
		{
			m_wImage.SetVisible(false);
		
			if (m_LoadingOverlay)
				m_LoadingOverlay.SetShown(false);
			
			if (m_OnImageSelected)
				m_OnImageSelected.Invoke();
			
			return;
		}
		
		m_wImage.SetVisible(TryLoadImageTexture(m_sFallbackImage));
	}
	
	//----------------------------------------------------------------------------------
	protected bool TryLoadImageTexture(ResourceName imagePath)
	{
		if (!m_wImage)
			return false;
		
		bool loaded = m_wImage.LoadImageTexture(0, imagePath, false, true);
		
		int sx, sy;
		m_wImage.GetImageSize(0, sx, sy);
		m_wImage.SetSize(sx, sy);
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(false);
		
		if (m_OnImageSelected)
			m_OnImageSelected.Invoke();
		
		return loaded; 
	}
	
	// --- PUBLIC ---
	//----------------------------------------------------------------------------------
	//! image can be null
	void SetImage(BackendImage image)
	{
		if (!image)
		{
			ShowFallbackImage();
			
			#ifdef BACKEND_IMAGE_DEBUG
			ShowDebugText("Image unavailable\nShowing default image");
			#endif
			
			return;
		}
		
		m_BackendImage = image;
			
		CheckWidgetInitialized();
	}
	
	//----------------------------------------------------------------------------------
	void SetImageSaturation(float saturation)
	{
		if (m_wImage)
			m_wImage.SetSaturation(saturation);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnImageSelected()
	{
		if (!m_OnImageSelected)
			m_OnImageSelected = new ScriptInvokerVoid();

		return m_OnImageSelected;
	}
	
	// --- DEBUG ---
	//----------------------------------------------------------------------------------
	protected void ShowDebugText(string txt)
	{
		if (!m_wDebugText)
			return;
		
		string commotDebugData;
		
		if (m_iPreferredWidth != 0)
			commotDebugData = string.Format("Widget Screen Width: %1\nPrefered Width: %2\nScales in Backend:\n%3",
				m_iScreenWidth, m_iPreferredWidth, FormatAvailableScales());
		
		m_wDebugText.SetText(commotDebugData + "\n" + txt);
	}
	
	//----------------------------------------------------------------------------------
	protected string FormatAvailableScales()
	{
		if (!m_BackendImage)
			return string.Empty;
		
		array<ImageScale> scales = {};
		m_BackendImage.GetScales(scales);
		
		string str;
		string s;
		int i;
		foreach (ImageScale scale : scales)
		{			
			if (scale)
				s = string.Format("%1x%2 ", scale.Width(), scale.Height());
			else
				s = "null ";
			
			i++;
			if (i == 3)
			{
				s = s + "\n";
				i = 0;
			}
			
			str = str + s;
		}
		
		return str;
	}
}

// --- BACKEND IMAGE COMPONENT variants ---
//! Same as SCR_BackendImageComponent, but implements default image based on tag of Workshop Item
class SCR_WorkshopItemBackendImageComponent : SCR_BackendImageComponent
{
	protected ref SCR_WorkshopItem m_Item;

	//------------------------------------------------------------------------------------------------
	//! Sets reference to workshop item. Call this, then call SetImage as usual
	void SetWorkshopItemAndImage(SCR_WorkshopItem item, BackendImage image)
	{
		m_Item = item;

		if (m_Item)
			m_sFallbackImage = SCR_WorkshopUiCommon.GetDefaultAddonThumbnail(m_Item);
		else
			m_sFallbackImage = SCR_WorkshopUiCommon.ADDON_DEFAULT_THUMBNAIL;

		SetImage(image);
	}

	//------------------------------------------------------------------------------------------------
	override void ShowLoadingImage(ResourceName loadingImage = string.Empty)
	{
		if (loadingImage.IsEmpty() && m_Item)
			loadingImage = SCR_WorkshopUiCommon.GetDefaultAddonThumbnail(m_Item);

		if (loadingImage.IsEmpty())
			loadingImage = SCR_WorkshopUiCommon.ADDON_DEFAULT_THUMBNAIL_LOADING;

		super.ShowLoadingImage(loadingImage);
	}
}

//! Same as SCR_BackendImageComponent, but implements default image based on scenario
class SCR_ScenarioBackendImageComponent : SCR_BackendImageComponent
{
	//------------------------------------------------------------------------------------------------
	override void ShowLoadingImage(ResourceName loadingImage = string.Empty)
	{
		if (loadingImage.IsEmpty())
			loadingImage = SCR_WorkshopUiCommon.SCENARIO_SP_DEFAULT_THUMBNAIL;

		super.ShowLoadingImage(loadingImage);
	}
}
