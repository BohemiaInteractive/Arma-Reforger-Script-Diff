/*!
//~~~~~~~~~~~ Guide how to set up the emat to use this animation script. ~~~~~~~~~~~\\
- First thing to note is that the emat you want to animate needs to be on the mesh of the SCR_CustomMaterialAnimatorEntity entity. It cannot be on a child entity (that child entity needs to be it's own SCR_CustomMaterialAnimatorEntity).

//~ Step by Step emat setup
	0. This entity has a Mesh, here you will need to ad the emat you want to animate. Each of this entity has their own local var and multiple instances can exist without a problem.
	1. Set the emat you want to animate on the mesh within the SCR_CustomMaterialAnimatorEntity
	2. Locate the .emat on your pc (not in workbench)
	3. Open the file with notepad (or similar programs to edit the code within)
	4. You will see in the file something on the line of: 

MatCommon {
 [Some variables]
};

	5. Within the { } you will add the following to link the emat with this script and specificly the m_iMaterialAnimationFrame:
 
 Refs {
  "AlbedoMap" "SCR_CustomMaterialAnimatorEntity.m_iMaterialAnimationFrame"
 }

	6. Save the material and you are done.
	7. Here is a fully functioning example emat that is set up for the animator:

MatCommon {
 Diffuse 0 0 0 1
 Ambient 0 0 0 1
 Emissive 0.281 0.216 1 1
 EmissiveLV 8
 EmissiveAbsoluteLV 0
 ApplyAlbedoToEmissive 1
 Sort translucent
 CastShadow 0
 ReceiveShadow 0
 Specular 0 0 0 1
 AlphaTest 0.2
 BlendMode Additive
 CrossFade 0
 AlbedoMap "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{0539259902F64090}Assets/Editor/Lightning/Data/LgStrike_01_MASK.edds" "{2F1D023AF41826BF}Assets/Editor/Lightning/Data/LgStrike_02_MASK.edds" "{F7514002C11BE92B}Assets/Editor/Lightning/Data/LgStrike_03_MASK.edds" "{7B554D7D19C4EAE1}Assets/Editor/Lightning/Data/LgStrike_04_MASK.edds" "{A3190F452CC72575}Assets/Editor/Lightning/Data/LgStrike_05_MASK.edds" "{893D28E6DA29435A}Assets/Editor/Lightning/Data/LgStrike_06_MASK.edds" "{51716ADEEF2A8CCE}Assets/Editor/Lightning/Data/LgStrike_07_MASK.edds" "{D3C5D3F2C27D725D}Assets/Editor/Lightning/Data/LgStrike_08_MASK.edds" "{0B8991CAF77EBDC9}Assets/Editor/Lightning/Data/LgStrike_09_MASK.edds" "{49930C342F0012B7}Assets/Editor/Lightning/Data/LgStrike_10_MASK.edds" "{91DF4E0C1A03DD23}Assets/Editor/Lightning/Data/LgStrike_11_MASK.edds" "{BBFB69AFECEDBB0C}Assets/Editor/Lightning/Data/LgStrike_12_MASK.edds" "{63B72B97D9EE7498}Assets/Editor/Lightning/Data/LgStrike_13_MASK.edds" "{EFB326E801317752}Assets/Editor/Lightning/Data/LgStrike_14_MASK.edds" "{37FF64D03432B8C6}Assets/Editor/Lightning/Data/LgStrike_15_MASK.edds" "{1DDB4373C2DCDEE9}Assets/Editor/Lightning/Data/LgStrike_16_MASK.edds" "{C597014BF7DF117D}Assets/Editor/Lightning/Data/LgStrike_17_MASK.edds" "{4723B867DA88EFEE}Assets/Editor/Lightning/Data/LgStrike_18_MASK.edds" "{9F6FFA5FEF8B207A}Assets/Editor/Lightning/Data/LgStrike_19_MASK.edds" "{B6495160AFF482F1}Assets/Editor/Lightning/Data/LgStrike_20_MASK.edds" "{6E0513589AF74D65}Assets/Editor/Lightning/Data/LgStrike_21_MASK.edds" "{442134FB6C192B4A}Assets/Editor/Lightning/Data/LgStrike_22_MASK.edds" "{9C6D76C3591AE4DE}Assets/Editor/Lightning/Data/LgStrike_23_MASK.edds" "{10697BBC81C5E714}Assets/Editor/Lightning/Data/LgStrike_24_MASK.edds" "{C8253984B4C62880}Assets/Editor/Lightning/Data/LgStrike_25_MASK.edds" "{E2011E2742284EAF}Assets/Editor/Lightning/Data/LgStrike_26_MASK.edds" "{3A4D5C1F772B813B}Assets/Editor/Lightning/Data/LgStrike_27_MASK.edds" "{B8F9E5335A7C7FA8}Assets/Editor/Lightning/Data/LgStrike_28_MASK.edds" "{60B5A70B6F7FB03C}Assets/Editor/Lightning/Data/LgStrike_29_MASK.edds" "{22AF3AF5B7011F42}Assets/Editor/Lightning/Data/LgStrike_30_MASK.edds" "{FAE378CD8202D0D6}Assets/Editor/Lightning/Data/LgStrike_31_MASK.edds" "{D0C75F6E74ECB6F9}Assets/Editor/Lightning/Data/LgStrike_32_MASK.edds" "{088B1D5641EF796D}Assets/Editor/Lightning/Data/LgStrike_33_MASK.edds" "{848F102999307AA7}Assets/Editor/Lightning/Data/LgStrike_34_MASK.edds" "{5CC35211AC33B533}Assets/Editor/Lightning/Data/LgStrike_35_MASK.edds" "{76E775B25ADDD31C}Assets/Editor/Lightning/Data/LgStrike_36_MASK.edds" "{AEAB378A6FDE1C88}Assets/Editor/Lightning/Data/LgStrike_37_MASK.edds" "{22AF3AF5B7011F42}Assets/Editor/Lightning/Data/LgStrike_30_MASK.edds" "{22AF3AF5B7011F42}Assets/Editor/Lightning/Data/LgStrike_30_MASK.edds" "{22AF3AF5B7011F42}Assets/Editor/Lightning/Data/LgStrike_30_MASK.edds" "{22AF3AF5B7011F42}Assets/Editor/Lightning/Data/LgStrike_30_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{22AF3AF5B7011F42}Assets/Editor/Lightning/Data/LgStrike_30_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{22AF3AF5B7011F42}Assets/Editor/Lightning/Data/LgStrike_30_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{22AF3AF5B7011F42}Assets/Editor/Lightning/Data/LgStrike_30_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" "{DD7567A137F58F04}Assets/Editor/Lightning/Data/LgStrike_00_MASK.edds" clampu clampv anim( 64 )
 Refs {
  "AlbedoMap" "SCR_CustomMaterialAnimatorEntity.m_iMaterialAnimationFrame"
 }
};

*/

[EntityEditorProps(category: "GameScripted/MaterialAnimation", description: "This entity plays .emat animations consistently from the given starting frame to the given end frame. After the animation is done it can: Loop (default), hide the entity, delete the entity or delete the entity and children")]
class SCR_CustomMaterialAnimatorEntityClass: GenericEntityClass
{
};
class SCR_CustomMaterialAnimatorEntity: GenericEntity
{
	[Attribute("1", params: "1 99999999", category: "Material Animation", desc: "Starting frame (texture in emat Albedo Map) of the animation. If m_bDeleteWhenDone is false then it will also restart on this frame again, Use in combination with m_iEndFrameIndex to play specific sections of the material animation. The first frame is 1 (so it is not an index)")]
	protected int m_iStartingFrame;
	
	[Attribute("-1", params: "-1 99999999", category: "Material Animation", desc: "At which frame (texture in the Albedo Map) should the animation stop playing (loop again or destroy entity). This should never be higher then Albedo Map texture count else there might be unintentional consequences.")]
	protected int m_iEndFrame;
	
	[Attribute("60", params: "0.001 99999999", category: "Material Animation", desc: "Use this to change the speed of the animation. Higher value means faster animation, if you have 60 frames and this value is 60 then the animation will be done in 1 second")]
	protected float m_fAnimationSpeed;
	
	[Attribute(defvalue: "0", category: "Material Animation", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EActionWhenDone), desc: "What will happen when the animation is done? By default it will loop but can also set to: Hide entity (sets visiblity flag), delete entity and delete entity & children")]
	protected EActionWhenDone m_iActionWhenDone;
	
	//~ This value is the actual varriable the emat will read. See the top of this script how to set up the emat.
	protected int m_iMaterialAnimationFrame = 0;
	
	//~ This value is used to calculate the next m_iMaterialAnimationFrame
	protected float m_fMaterialAnimationFrameCalculator = 0;
	
	
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{
		//~ Increase the frame calculater
		m_fMaterialAnimationFrameCalculator += m_fAnimationSpeed * timeSlice;
		
		//~ Set frame to converted int of calculator. The m_iMaterialAnimationFrame is the actual value the emat will read
		m_iMaterialAnimationFrame = m_fMaterialAnimationFrameCalculator;
		
		//End of animation reached
		if (m_iMaterialAnimationFrame >= m_iEndFrame)
		{
			//~ Animation done, Loop again to m_iStartingFrame
			if (m_iActionWhenDone == EActionWhenDone.LOOP)
			{
				m_fMaterialAnimationFrameCalculator = m_iStartingFrame;
				m_iMaterialAnimationFrame = m_iStartingFrame;
			}
			//~ Animation done, remove on Frame and hide the entity
			else if (m_iActionWhenDone == EActionWhenDone.HIDE)
			{
				ClearEventMask(EntityEvent.FRAME);
				ClearFlags(EntityFlags.VISIBLE, true);
			}
			//~ Animation done delete self
			else if (m_iActionWhenDone == EActionWhenDone.DELETE_SELF)
			{
				delete this;
			}
			//~ Animation done delete self and children
			else if (m_iActionWhenDone == EActionWhenDone.DELETE_SELF_AND_CHILDREN)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(this);
			}
		}
	}

	void SCR_CustomMaterialAnimatorEntity(IEntitySource src, IEntity parent)
	{
		if (m_iEndFrame < 1)
			Print("'SCR_CustomMaterialAnimatorEntity': 'm_iEndFrame' is not set correctly! Check the textures in the Albedo Map of the emat to find out what to set the 'm_iEndFrame' value to", LogLevel.WARNING);
		else if (m_iStartingFrame > m_iEndFrame)
			Print("'SCR_CustomMaterialAnimatorEntity': 'm_iStartingFrame' is higher then 'm_iEndFrame', make sure 'm_iStartingFrame' is lower then 'm_iEndFrame'!", LogLevel.WARNING);
		
		
		//~ System works with indexes so while varriables follow the same logic as the albinoMap starting at frame 1 and ending at count. The actual logic starts at index 0 and ends at count -1.
		m_iStartingFrame -= 1;
		m_iEndFrame -= 1;
		
		m_fMaterialAnimationFrameCalculator = m_iStartingFrame;
		m_iMaterialAnimationFrame = m_iStartingFrame;

		SetEventMask(EntityEvent.FRAME);
	}
};

enum EActionWhenDone
{
	LOOP = 0,
	HIDE = 1,
	DELETE_SELF = 2,
	DELETE_SELF_AND_CHILDREN = 3,
};









