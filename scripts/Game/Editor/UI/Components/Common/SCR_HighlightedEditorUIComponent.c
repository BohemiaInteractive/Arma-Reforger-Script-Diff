//! Is an highlighted icon which, if animated pulsates

class SCR_HighlightedEditorUIComponent : SCR_BaseEditableEntityUIComponent
{
	[Attribute("Icon")]
	protected string m_ImageWidgetName;

	[Attribute("IconText")]
	protected string m_TextWidgetName;

	[Attribute("Animation")]
	protected string m_AnimWidgetName;

	[Attribute(desc: "If Highlight has no specific icon")]
	protected ResourceName m_DefaultIcon;

	[Attribute(desc: "If multiple highlights")]
	protected ResourceName m_MultiHighlightIcon;

	[Attribute(desc: "Array of highlights and icons")]
	protected ref array<ref SCR_HighlightVisual> m_aHighlightVisuals;
	
	[Attribute("1")]
	protected bool m_bAnimate;
	
	//Ref
	protected SCR_HighlightedEditorComponent m_HighlightedEditorComponent;
	protected ImageWidget m_ImageWidget;
	protected TextWidget m_TextWidget;

	//-------------------------- On Highlight update --------------------------\\

	//------------------------------------------------------------------------------------------------
	protected void OnUpdateHighlight(SCR_EditableEntityComponent entity, EEditableEntityHighlight highlight, int highlightCount)
	{
		if (entity != m_Entity)
			return;
		
		if (highlightCount > 1)
		{	
			m_ImageWidget.LoadImageTexture(0, m_MultiHighlightIcon);
			m_ImageWidget.SetImage(0);
			m_TextWidget.SetText(highlightCount.ToString());
			m_TextWidget.SetVisible(true);
		}
		else 
		{
			m_TextWidget.SetVisible(false);
			foreach (SCR_HighlightVisual visual: m_aHighlightVisuals)
			{
				if (visual.GetHighlightType() == highlight)
				{
					m_ImageWidget.LoadImageTexture(0, visual.GetIcon());
					m_ImageWidget.SetImage(0);
					return;
				}
			}
			
			//No specific icon found
			m_ImageWidget.LoadImageTexture(0, m_DefaultIcon);
			m_ImageWidget.SetImage(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelectedChange(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		if (entitiesInsert && entitiesInsert.Contains(m_Entity))
			StopAnimations(GetWidget());
	}

	//------------------------------------------------------------------------------------------------
	protected void StopAnimations(Widget w)
	{
		if (m_bAnimate)
		{
			Widget animWidget = w.FindAnyWidget(m_AnimWidgetName);
			if (animWidget)
			{
				AnimateWidget.StopAllAnimations(animWidget);
				animWidget.SetVisible(false);
			}
			
			SCR_BaseEditableEntityFilter selectedFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
			if (selectedFilter)
				selectedFilter.GetOnChanged().Remove(OnSelectedChange);
		}
	}
	
	//-------------------------- Init --------------------------\\

	//---- REFACTOR NOTE START: Hardcoded values for size
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_EditableEntityComponent entity, SCR_UIInfo info, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		super.OnInit(entity, info, slot);
		
		m_HighlightedEditorComponent = SCR_HighlightedEditorComponent.Cast(SCR_HighlightedEditorComponent.GetInstance(SCR_HighlightedEditorComponent, true));
		if (!m_HighlightedEditorComponent) 
			return;
		
		Widget w = GetWidget();
		m_ImageWidget = ImageWidget.Cast(w.FindAnyWidget(m_ImageWidgetName));
		m_TextWidget = TextWidget.Cast(w.FindAnyWidget(m_TextWidgetName));
		
		EEditableEntityHighlight highlight;
		int highlightCount;
	
		m_HighlightedEditorComponent.GetEntityHighlightedState(m_Entity, highlight, highlightCount);
		m_HighlightedEditorComponent.GetOnUpdateHighlight().Insert(OnUpdateHighlight);
		OnUpdateHighlight(m_Entity, highlight, highlightCount);
		
		if (m_bAnimate)
		{
			Widget animWidget = w.FindAnyWidget(m_AnimWidgetName);

			float size[2] = {70, 70};
			WidgetAnimationBase anim = AnimateWidget.Opacity(animWidget, 0, 1);
			if (anim)
				anim.SetRepeat(true);
			
			anim = AnimateWidget.Size(animWidget, size, 1);
			if (anim)
				anim.SetRepeat(true);
			
			SCR_BaseEditableEntityFilter selectedFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
			if (selectedFilter)
				selectedFilter.GetOnChanged().Insert(OnSelectedChange);
		}
	}
	
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		StopAnimations(w);
		return super.OnClick(w, x, y, button);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_HighlightedEditorComponent && m_Entity)
			m_HighlightedEditorComponent.GetOnUpdateHighlight().Remove(OnUpdateHighlight);
		
		StopAnimations(w);
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityHighlight, "m_HighlightType")]
class SCR_HighlightVisual
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityHighlight))]
	protected EEditableEntityHighlight m_HighlightType;

	[Attribute()]
	protected ResourceName m_Icon;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EEditableEntityHighlight GetHighlightType()
	{
		return m_HighlightType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetIcon()
	{
		return m_Icon;
	}
}
