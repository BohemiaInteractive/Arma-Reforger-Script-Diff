// Class for one entry of the magazine configuration
// It contains images for mag outline, progress bar and alpha mask.
[BaseContainerProps(configRoot: true)]
class SCR_MagazineIndicatorConfiguration 
{
	// ---- Layout, can be used with the textures below or without, allows for more cutomizable mag indicators
	[Attribute("{8A98061335A620E1}UI/layouts/HUD/WeaponInfo/SingleMagazineIndicator.layout", UIWidgets.ResourceNamePicker, "Custom Layout for magazine indicator. If null, default will be used.")]
	ResourceName m_sMagazineLayout;
	
	// ---- Imageset with magazine standard textures
	[Attribute("{CDA6C73DFD789999}UI/Imagesets/WeaponInfo/WeaponInfo.imageset", UIWidgets.ResourceNamePicker, "Imageset with foreground textures", "imageset")]
	ResourceName m_sImagesetIcons;
	
	// ---- Imageset with magazine glow textures
	[Attribute("{C6D15495F684E841}UI/Imagesets/WeaponInfo/WeaponInfo_Glow.imageset", UIWidgets.ResourceNamePicker, "Imageset with glow textures", "imageset")]
	ResourceName m_sImagesetGlows;
	
	// ---- Magazine outline image ----
	[Attribute("magazine-default-outline", UIWidgets.EditBox, "Magazine icon/outline image")]
	string m_sOutline;
	
	[Attribute("true", UIWidgets.EditBox, "Use progress bar. If false, only outline image will be used.", params: "imageset")]
	bool m_bProgressBar;	
	
	[Attribute("magazine-default-background", UIWidgets.EditBox, "Magazine background image")]
	string m_sBackground;	
		
	// ---- Magazine progress image ----
	[Attribute("magazine-default-fill", UIWidgets.EditBox, "Magazine progress bar image")]
	string m_sProgress;	
	
	// ---- Magazine alpha mask image ----
	[Attribute("magazine-default-alpha", UIWidgets.EditBox, "Magazine progress bar alpha mask")]
	string m_sProgressAlphaMask;
	
	// ---- Empty magazine indicator ----
	[Attribute("firemode-safety", UIWidgets.EditBox, "Icon to display when magazine is empty (used for rocket pods)")]
	string m_sEmptyMagazine;
};