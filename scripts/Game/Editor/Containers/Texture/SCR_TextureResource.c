[BaseContainerProps()]
class SCR_TextureResource : SCR_BaseTextureResource
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "edds")]
	protected ResourceName m_Texture;

	//------------------------------------------------------------------------------------------------
	override void ApplyTo(ImageWidget widget)
	{
		if (m_Texture.IsEmpty())
		{
			Print("Texture not defined!", LogLevel.ERROR);
			return;
		}

		if (widget) widget.LoadImageTexture(0, m_Texture);
	}
}
