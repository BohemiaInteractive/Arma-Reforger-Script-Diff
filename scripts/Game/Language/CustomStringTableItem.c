//! Custom string table item for the game
class CustomStringTableItem: ScriptStringTableItem
{
	static override string GetTargetPrefix() { return "Target_"; }
	
	[Attribute(category:"Default", desc: "English (United States)\nServes as a source for translated texts.\nIf proofreading or translation is in progress, use 'Target_en_us_edited' instead.", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_en_us;
	[Attribute(category:"Default", desc: "English (United States)\nUse this when the string is being processed by proofreaders or translators.\nOnce it's done, the Localization Manager will move this text to 'Target_en_us'.", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_en_us_edited;
	[Attribute(category:"Translated Texts", desc: "French (France)", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_fr_fr;
	[Attribute(category:"Translated Texts", desc: "Italian (Italy)", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_it_it;
	[Attribute(category:"Translated Texts", desc: "German (Germany)", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_de_de;
	[Attribute(category:"Translated Texts", desc: "Spanish (Spain)", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_es_es;
	[Attribute(category:"Translated Texts", desc: "Czech (Czech Republic)", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_cs_cz;
	[Attribute(category:"Translated Texts", desc: "Polish (Poland)", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_pl_pl;
	[Attribute(category:"Translated Texts", desc: "Russian (Russia)", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_ru_ru;
	[Attribute(category:"Translated Texts", desc: "Japanese (Japan)", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_ja_jp;
	[Attribute(category:"Translated Texts", desc: "Korean (South Korea)", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_ko_kr;
	[Attribute(category:"Translated Texts", desc: "Portuguese (Brazil)", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_pt_br;
	[Attribute(category:"Translated Texts", desc: "Chinese (China)", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_zh_cn;
	[Attribute(category:"Translated Texts", desc: "Ukrainian (Ukraine)", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_uk_ua;
	
	[Attribute(category:"Custom", uiwidget: UIWidgets.EditBoxWithButton, desc: "Comment for translators.")]
	string Comment;
	
	[Attribute(category:"Custom", desc: "Comment only for developers, not translators.", uiwidget: UIWidgets.EditBoxWithButton)]
	string DevNote;
	
	[Attribute(category:"Custom")]
	string Repro;
	
	[Attribute(category:"Custom", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EStringTableActorGender))]
	EStringTableActorGender ActorGender;
	
	[Attribute(category:"Custom", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EStringTableTerminology))]
	EStringTableTerminology Terminology;
	
	[Attribute(category:"Default", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EStringTableStatus), defvalue: "0", desc: "Status of the string.\n\nDEVELOPMENT_PENDING - developers are working on the text\nDEVELOPMENT_DONE - developers finished the text and it's ready to be proofread\nPROOFREADING_PENDING - proofreaders are working on the text\nPROOFREADING_DONE - proofreaders processed the text and it's ready to be translated\nTRANSLATION_PENDING - translators are working on the text\nTRANSLATION_DONE - text is fully translated and ready for release")]
	EStringTableStatus Status;
	
	[Attribute(category:"Custom", uiwidget: UIWidgets.CheckBox)]
	bool NonTranslatable;
	
	[Attribute(category:"Custom", desc: "Provide a number of how many W characters fit before we have a cut-off or overlap of this string, not a number of regular characters.")]
	int MaxLength;
	
	[Attribute(category:"Meta", uiwidget: UIWidgets.Date)]
	int Modified;
	
	[Attribute(category:"Meta", desc: "Who create the item")]
	string Author;
	
	[Attribute(category:"Meta", desc: "Who last edited the item")]
	string LastChanged;
	
	[Attribute(category:"Custom", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EStringTableLabels))]
	ref array<int> Labels;
	
	[Attribute(category:"Custom", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "File this string originates from.")]
	ResourceName SourceFile;
	
	[Attribute(category:"Custom", desc: "Online link to image showing context of the text.")]
	string ImageLink;
	
	[Attribute(category:"Custom", defvalue: "false", desc: "Hidden items will not be present in public releases")]
	bool Hidden;
};