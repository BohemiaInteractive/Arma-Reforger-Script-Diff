enum SCR_EFractalDestructionForceModel
{
	FORCE_NONE = -1,
	FORCE_UNFRACTURED,
	FORCE_FRACTURED
}

[ComponentEditorProps(category: "GameScripted/Destruction", description: "Fractal destruction component. For objects that should shatter/splinter etc")]
class SCR_DestructionFractalComponentClass: SCR_DestructionDamageManagerComponentClass
{
	[Attribute(ResourceName.Empty, UIWidgets.ResourceNamePicker, "Particle effect to play when a fragment is destroyed", "ptc", category: "Destruction Fractal")]
	ResourceName m_ParticleDestroyFragment;

	[Attribute("", UIWidgets.Object, "Debris settings to use when a fragment is destroyed and debris is spawned", category: "Destruction Fractal")]
	ref SCR_FragmentDebris m_DebrisDestroyFragment;

	[Attribute("10", UIWidgets.Slider, "Health value of each fragment", "0.01 100000 0.01", category: "Destruction Fractal")]
	float m_fFragmentHealth;

	[Attribute("1", UIWidgets.Slider, "Whether to destroy the fragment at the impact point when damage leads to fracturing of the object", category: "Destruction Fractal")]
	bool m_bDestroyFragmentOnFracture;

	[Attribute("0", UIWidgets.CheckBox, "If true, the object will be deleted after the last fragment has been destroyed", category: "Destruction Fractal")]
	bool m_bDeleteAfterFinalFragment;

	[Attribute("1", UIWidgets.Slider, "Whether structural integrity is enabled (fragments that have no anchor fragments to hold on to fall as well when a nearby fragment is destroyed)", category: "Destruction Fractal")]
	bool m_bEnableStructuralIntegrity;

	[Attribute("", UIWidgets.Object, "List of fractal setup variations (chosen using position as seed)", category: "Destruction Fractal")]
	ref array<ref SCR_FractalVariation> m_FractalVariants;
}

//! Fractal destruction component. For objects that should shatter/splinter etc
class SCR_DestructionFractalComponent: SCR_DestructionDamageManagerComponent
{
	#ifdef ENABLE_DESTRUCTION
		protected SCR_FractalVariation m_UsedFractalData;
		protected static ref array<SCR_FragmentEntity> s_FragmentList = {};
		protected static ref array<SCR_FragmentEntity> s_FragmentToCheckList = {};
		protected static ref array<SCR_FragmentEntity> s_FragmentCheckedList = {};

		protected static ref s_RandomGenerator = new RandomGenerator();
	
		#ifdef WORKBENCH
			[Attribute("0", UIWidgets.CheckBox, "Check to generate fragment hierarchies for each fractal variant", category: "EDITOR: Destruction Fractal")]
			protected bool GenerateFragmentHierarchies;

			[Attribute("0", UIWidgets.CheckBox, "Check to toggle display of visualizers in the World Editor", category: "EDITOR: Destruction Fractal")]
			protected bool ToggleVisualizers;

			static bool s_WBDisplayVisualizers = false;
			static GenericEntity s_WBVisualizeEntity = null;
			static ref array<SCR_PreviewEntity> s_WBFragmentList = {};

			//------------------------------------------------------------------------------------------------
			protected static void ClearVisualizers()
			{
				s_WBVisualizeEntity = null;
				SCR_PreviewEntity fragVis;
				foreach (SCR_PreviewEntity preview : s_WBFragmentList)
				{
					fragVis = s_WBFragmentList[i];
					delete fragVis;
				}
				s_WBFragmentList.Clear();
			}

			//------------------------------------------------------------------------------------------------
			override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
			{
				GenericEntity genOwner = GenericEntity.Cast(owner);
				if (!genOwner)
					return true;

				WorldEditorAPI api = genOwner._WB_GetEditorAPI();
				if (!api || api.UndoOrRedoIsRestoring())
					return false;

				switch (key)
				{
					case "GenerateFragmentHierarchies":
					{
						src.ClearVariable("GenerateFragmentHierarchies");

						Print("SCR_DestructionFractalComponent: Generating fragment hierarchies...", LogLevel.NORMAL);

						BaseContainerList srcFractalVariantList = src.GetObjectArray("m_FractalVariants");
						int numVariants = m_FractalVariants.Count();
						for (int v = 0; v < numVariants; v++)
						{
							BaseContainer srcFractalVariant = srcFractalVariantList.Get(v);
							SCR_FractalVariation fractalVariant = m_FractalVariants[v];
							fractalVariant.m_Hierarchy = new SCR_FragmentHierarchy;
							fractalVariant.m_Hierarchy.GenerateHierarchy(fractalVariant);
							srcFractalVariant.Set("m_Hierarchy", fractalVariant.m_Hierarchy);
						}

						BaseContainerTools.WriteToInstance(this, src);
						genOwner._WB_GetEditorAPI().UpdateSelectionGui();
						break;
					}

					case "ToggleVisualizers":
					{
						src.ClearVariable("ToggleVisualizers");
						BaseContainerTools.WriteToInstance(this, src);

						s_WBDisplayVisualizers = !s_WBDisplayVisualizers;
						if (!s_WBDisplayVisualizers)
							ClearVisualizers();

						genOwner._WB_GetEditorAPI().UpdateSelectionGui();

						break;
					}

					// The code below stores the newly set model to the MeshObject component on the entity
//					case "m_ModelNormal":
//					{
//						// Model has changed in a variation, check if it's the one we are using
//						int currentIndex = m_FractalVariants.Find(m_UsedFractalData);
//						if (currentIndex == -1)
//							break;
//
//						// Get the fractal destruction component and mesh object component from parent entity (if present)
//						BaseContainer fractalComp = null;
//						BaseContainer meshComp = null;
//						BaseContainer ownerEnt = null;
//						int contCount = ownerContainers.Count();
//						if (contCount > 0)
//							fractalComp = ownerContainers.Get(contCount - 1);
//						if (contCount > 1)
//							ownerEnt = ownerContainers.Get(contCount - 2);
//						if (ownerEnt)
//						{
//							BaseContainerList ownerEntCompList = ownerEnt.GetObjectArray("components");
//							if (ownerEntCompList)
//							{
//								for (int i = 0, num = ownerEntCompList.Count(); i < num; i++)
//								{
//									BaseContainer cont = ownerEntCompList.Get(i);
//									if (cont.GetClassName() == "MeshObject")
//									{
//										meshComp = cont;
//										break;
//									}
//								}
//							}
//						}
//
//						if (!meshComp || !fractalComp)
//							break;
//
//						// Get the index of the fractal variant in the variants container
//						BaseContainerList fractalVariantsList = fractalComp.GetObjectArray("m_FractalVariants");
//						if (!fractalVariantsList)
//							break;
//
//						int currentSrcIndex = -1;
//						for (int i = 0, num = fractalVariantsList.Count(); i < num; i++)
//						{
//							BaseContainer variant = fractalVariantsList.Get(i);
//							if (variant != src)
//								continue;
//							currentSrcIndex = i;
//							break;
//						}
//
//						if (currentSrcIndex != currentIndex)
//							break;
//
//						// Get the new model
//						ResourceName newModel = ResourceName.Empty;
//						if (!src.Get(key, newModel))
//							break;
//
//						// Set the new model and store it to the instance
//						meshComp.Set("Object", newModel);
//						BaseContainerTools.WriteToInstance(this, meshComp);
//						UpdateModel(SCR_EFractalDestructionForceModel.FORCE_UNFRACTURED);
//
//						break;
//					}

					default:
					{
						// Get our fractal component source
						BaseContainer srcFractalComp = null;
						if (src.GetClassName() == "SCR_DestructionFractalComponent")
						{
							srcFractalComp = src;
						}
						else
						{
							for (int c = 0, srcCount = ownerContainers.Count(); c < srcCount; c++)
							{
								BaseContainer compSrc = ownerContainers.Get(c);
								if (compSrc.GetClassName() != "SCR_DestructionFractalComponent")
									continue;

								srcFractalComp = compSrc;
								break;
							}
						}

						// Now go through each fractal variant and validate hierarchies
						if (srcFractalComp)
						{
							BaseContainerList srcFVariantList = srcFractalComp.GetObjectArray("m_FractalVariants");
							int numVariants = m_FractalVariants.Count();
							bool needResave = false;
							for (int v = 0; v < numVariants; v++)
							{
								BaseContainer srcFVariant = srcFVariantList.Get(v);
								if (!srcFVariant)
									continue;

								BaseContainer srcHier = srcFVariant.GetObject("m_Hierarchy");
								if (!srcHier)
									continue;

								SCR_FractalVariation fracVariant = m_FractalVariants[v];
								if (fracVariant.m_Hierarchy && fracVariant.m_Hierarchy.ValidateHierarchy(fracVariant, srcHier, v))
									needResave = true;
							}

							if (needResave)
							{
								BaseContainerTools.WriteToInstance(this, srcFractalComp);
								genOwner._WB_GetEditorAPI().UpdateSelectionGui();
							}
						}

						ClearVisualizers();
						break;
					}
				}

				return true;
			}

			//------------------------------------------------------------------------------------------------
			//! Creates the fragment visualiser
			//! \param[in] index
			//! \return
			SCR_PreviewEntity CreateFragmentVisualizer(int index)
			{
				bool isAnchor = false;
				SCR_PreviewEntity fragVis = SCR_PreviewEntity.Cast(GetGame().SpawnEntity(SCR_PreviewEntity));
				Resource resource = Resource.Load(m_UsedFractalData.GetFragmentModel(index, isAnchor));
				VObject asset = resource.GetResource().ToVObject();
				string materials[256];
				int numMats = asset.GetMaterials(materials);
				string remap = "";
				for (int m = 0; m < numMats; m++)
				{
					remap += "$remap '" + materials[m] + "' '{639855E4E1F52285}Assets/Editor/PlacingPreview/Preview_Scriptable.emat';";
				}

				const int intensity = 200;
				const int intensityOther = 16;
				int mod = index % 6;
				if (mod == 0)
					fragVis.m_Color = ARGB(255, intensityOther, intensityOther, intensity);
				else
				if (mod == 1)
					fragVis.m_Color = ARGB(255, intensityOther, intensity, intensityOther);
				else
				if (mod == 2)
					fragVis.m_Color = ARGB(255, intensity, intensityOther, intensityOther);
				else
				if (mod == 3)
					fragVis.m_Color = ARGB(255, intensity, intensityOther, intensity);
				else
				if (mod == 4)
					fragVis.m_Color = ARGB(255, intensityOther, intensity, intensity);
				else
				if (mod == 5)
					fragVis.m_Color = ARGB(255, intensity, intensity, intensityOther);

				fragVis.SetObject(asset, remap);
				fragVis.SetFlags(EntityFlags.ACTIVE);
				m_Owner.AddChild(fragVis, -1);
				fragVis.Update();

				return fragVis;
			}

			//------------------------------------------------------------------------------------------------
			override int _WB_GetAfterWorldUpdateSpecs(IEntity owner, IEntitySource src)
			{
				return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
			}

			//------------------------------------------------------------------------------------------------
			override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
			{
				GenericEntity gEntity = GenericEntity.Cast(owner);
				if (!s_WBDisplayVisualizers)
				{
					if (s_WBVisualizeEntity)
						ClearVisualizers();

					return;
				}

				// Are we selected as the main entity?
				if (!gEntity._WB_GetEditorAPI().IsEntitySelectedAsMain(gEntity))
				{
					if (s_WBVisualizeEntity == gEntity)
						ClearVisualizers();

					return;
				}

				vector textMat[4], camMat[4];
				gEntity.GetWorld().GetCurrentCamera(camMat);
				gEntity.GetWorld().GetCurrentCamera(textMat);
				vector camDir = camMat[2];
				vector camPos = camMat[3];
				vector entPos = gEntity.GetOrigin();
				vector entCenter = SCR_Global.GetEntityCenterWorld(gEntity);
				vector entMins, entMaxs;
				gEntity.GetWorldBounds(entMins, entMaxs);
				vector entTop = Vector((entMaxs[0] + entMins[0]) * 0.5, entMaxs[1], (entMaxs[2] + entMins[2]) * 0.5);
				float distScale = Math.Clamp(vector.Distance(camPos, entCenter) * 0.15, 0.1, 3);

				// A different entity was selected before, so clear visualizers and make our own
				if (s_WBVisualizeEntity != gEntity)
				{
					ClearVisualizers();
					s_WBVisualizeEntity = gEntity;

					if (!m_UsedFractalData)
						InitDestruction();

					if (!m_UsedFractalData)
						return;

					int numFrags = m_UsedFractalData.CountFragments();
					for (int i = 0; i < numFrags; i++)
					{
						s_WBFragmentList.Insert(CreateFragmentVisualizer(i));
					}
				}

				// No fractal data, so display text indicating so
				if (!m_UsedFractalData)
				{
					textMat[3] = entTop + vector.Up * 0.25;
					CreateSimpleText("NO FRACTAL VARIANT", textMat, 0.17 * distScale, ARGB(255, 255, 0, 0), ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, null, 0.7, true, ARGB(128, 0, 0, 0));

					return;
				}

				vector highlightDir = SCR_Global.ProjWorldEditorMouseScreenToWorld(gEntity);
				int highlighted = -1;
				int numFrags = m_UsedFractalData.CountFragments();
				for (int i = 0; i < numFrags; i++)
				{
					SCR_PreviewEntity fragVis = s_WBFragmentList[i];
					bool isAnchor = m_UsedFractalData.GetFragmentIndexIsAnchor(i);
					vector fragCenter = SCR_Global.GetEntityCenterWorld(fragVis);
					vector fragToCamDir = (fragCenter - camPos).Normalized();
					textMat[3] = fragCenter;

					int textColor = ARGB(255, 255, 255, 255);
					int bgColor = ARGB(128, 0, 0, 0);
					float textSize = 0.12;
					if (highlighted == -1 && vector.Dot(fragToCamDir, highlightDir) > 0.9997)
					{
						highlighted = i;
						textColor = ARGB(255, 0, 255, 0);
						bgColor = ARGB(200, 0, 32, 0);
						textSize = 0.2;
						fragVis.ClearFlags(EntityFlags.VISIBLE, false);

						if (m_UsedFractalData.m_Hierarchy)
						{
							SCR_FragmentLinkage link = m_UsedFractalData.m_Hierarchy.GetFragmentLinkage(i);
							if (link)
							{
								vector from = fragCenter + vector.Up * distScale * -0.2;
								int numLinked = link.m_aOtherIndexes.Count();
								for (int n = 0; n < numLinked; n++)
								{
									int otherIndex = link.m_aOtherIndexes[n];
									if (otherIndex < 0 || otherIndex >= numFrags || otherIndex == i)
										continue;

									SCR_PreviewEntity otherFragVis = s_WBFragmentList[otherIndex];
									bool isOtherAnchor = m_UsedFractalData.GetFragmentIndexIsAnchor(otherIndex);
									vector fragOtherCenter = SCR_Global.GetEntityCenterWorld(otherFragVis);

									int arrowsColor = ARGB(255, 255, 0, 0);
									if (isOtherAnchor || isAnchor)
										arrowsColor = ARGB(255, 0, 128, 255);

									int numArrows = Math.Ceil(vector.Distance(fragCenter, fragOtherCenter) * 10 / distScale);
									if (numArrows == 0)
										numArrows = 1;

									Shape.Create(ShapeType.LINE, arrowsColor, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, from, fragOtherCenter);
								}

								CreateCircle(from, camDir, 0.02 * distScale, ARGB(255, 128, 128, 128), 12, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER);
							}
						}
					}
					else
					{
						fragVis.SetFlags(EntityFlags.VISIBLE, false);
					}

					textMat[3] = textMat[3] + camMat[1] * textSize * distScale * -0.5;
					CreateSimpleText(i.ToString(), textMat, textSize * distScale, textColor, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, null, 1, true, bgColor);
					if (isAnchor)
					{
						textMat[3] = textMat[3] - vector.Up * distScale * textSize * 1.6;
						CreateSimpleText("ANCHOR", textMat, textSize * distScale, ARGB(255, 255, 64, 64), ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, null, 1, true, bgColor);
					}
				}
			}
		#endif // WORKBENCH

		//------------------------------------------------------------------------------------------------
		//! \return whether or not the object has been fractured
		bool GetFractured()
		{
			return GetDestroyed();
		}

		//------------------------------------------------------------------------------------------------
		// \return how many fragments are left
		int CountFragments()
		{
			int numFragments = 0;

			IEntity child = m_Owner.GetChildren();
			while (child)
			{
				SCR_FragmentEntity fragment = SCR_FragmentEntity.Cast(child);
				child = child.GetSibling();
				if (fragment)
					numFragments++;
			}

			return numFragments;
		}

		//------------------------------------------------------------------------------------------------
		// Fills the input list with fragments and returns the maximum amount
		//! \param[out] fragmentList
		//! \return
		int FillCompleteOrderedFragmentList(notnull array<SCR_FragmentEntity> fragmentList)
		{
			fragmentList.Clear();
			int maxFragments = m_UsedFractalData.CountFragments();
			for (int i = 0; i < maxFragments; i++)
			{
				fragmentList.Insert(null);
			}

			IEntity child = m_Owner.GetChildren();
			while (child)
			{
				SCR_FragmentEntity fragment = SCR_FragmentEntity.Cast(child);
				child = child.GetSibling();
				if (fragment)
					fragmentList[fragment.GetIndex()] = fragment;
			}

			return fragmentList.Count();
		}

		//------------------------------------------------------------------------------------------------
		// Fills the input list with fragments and returns the amount
		//! \param[in] fragmentList
		//! \return
		int FillFragmentList(notnull array<SCR_FragmentEntity> fragmentList)
		{
			fragmentList.Clear();

			IEntity child = m_Owner.GetChildren();
			while (child)
			{
				SCR_FragmentEntity fragment = SCR_FragmentEntity.Cast(child);
				child = child.GetSibling();
				if (fragment)
					fragmentList.Insert(fragment);
			}

			return fragmentList.Count();
		}

		//------------------------------------------------------------------------------------------------
		// Fills the input list with fragments' indexes and returns the amount
		//! \param[in] fragmentIndexList
		//! \return
		int FillFragmentIndexList(array<int> fragmentIndexList)
		{
			fragmentIndexList.Clear();

			IEntity child = m_Owner.GetChildren();
			while (child)
			{
				SCR_FragmentEntity fragment = SCR_FragmentEntity.Cast(child);
				child = child.GetSibling();
				if (fragment)
					fragmentIndexList.Insert(fragment.GetIndex());
			}

			return fragmentIndexList.Count();
		}

		//------------------------------------------------------------------------------------------------
		//! \return the fractal variant for this entity
		SCR_FractalVariation GetCurrentFractalVariant()
		{
			return m_UsedFractalData;
		}

		//------------------------------------------------------------------------------------------------
		//! \return a random fractal variant based on this entity's position
		SCR_FractalVariation GetRandomFractalVariant()
		{
			int numVariants = m_FractalVariants.Count();
			if (numVariants == 0)
				return null;

			int seed = 0;
			vector pos;
			IEntity parent = SCR_Global.GetMainParent(m_Owner);
			if (parent) // We are parented, so use main parent position plus child count (as we will always be first child) for the seed
			{
				pos = parent.GetOrigin();
				seed = SCR_Global.CountChildren(m_Owner.GetParent()) * 11111;
			}
			else // Just use our position
			{
				pos = m_Owner.GetOrigin();
			}

			int x = Math.Floor(pos[0] * 1000);
			int z = Math.Floor(pos[2] * 1000);
			x = x % 10000;
			z = z % 10000;
			seed += x + z;
			s_RandomGenerator.Randomize(seed); // Set randomizer to seed based on position

			int randomVariant = s_RandomGenerator.RandomInt(0, numVariants);
			SCR_FractalVariation variant = m_FractalVariants.Get(randomVariant);

			return variant;
		}

		//------------------------------------------------------------------------------------------------
		//! Deletes any fragments
		void DeleteFragments()
		{
			if (!m_Owner)
				return;

			IEntity child = m_Owner.GetChildren();
			while (child)
			{
				SCR_FragmentEntity fragment = SCR_FragmentEntity.Cast(child);
				child = child.GetSibling();
				if (fragment)
					delete fragment;
			}
		}

		//------------------------------------------------------------------------------------------------
		//! Finds the fragment with input index
		//! \param[in]
		//! \return the found fragment or null if the index is not found
		SCR_FragmentEntity FindFragment(int index)
		{
			IEntity child = m_Owner.GetChildren();
			while (child)
			{
				SCR_FragmentEntity fragment = SCR_FragmentEntity.Cast(child);
				child = child.GetSibling();
				if (fragment.GetIndex() == index)
					return fragment;
			}

			return null;
		}

		//------------------------------------------------------------------------------------------------
		//! Creates a specific fragment
		//! \param[in] index
		//! \return
		protected SCR_FragmentEntity CreateFragment(int index)
		{
			bool isAnchor = false;
			ResourceName assetPath = m_UsedFractalData.GetFragmentModel(index, isAnchor);
			if (assetPath == ResourceName.Empty)
				return null;

			SCR_FragmentEntity fragment = SCR_FragmentEntity.Cast(GetGame().SpawnEntity(SCR_FragmentEntity));
			fragment.Initialize(this, index, m_fFragmentHealth, assetPath);

			return fragment;
		}

		//------------------------------------------------------------------------------------------------
		//! Creates all fragments for the object
		//! \param[in] addToTraceIgnoreList
		protected void CreateFragments(bool addToTraceIgnoreList = false)
		{
			if (!m_UsedFractalData)
				return;

			if (addToTraceIgnoreList)
				SCR_Global.g_TraceFilterList.Clear();

			int num = m_UsedFractalData.CountFragments();
			for (int i = 0; i < num; i++)
			{
				SCR_FragmentEntity fragment = CreateFragment(i);
				if (addToTraceIgnoreList && fragment)
					SCR_Global.g_TraceFilterList.Insert(fragment);
			}
		}

		//------------------------------------------------------------------------------------------------
		//! Updates and ensures structural integrity for fragments (uses lists s_FragmentList, s_FragmentToCheckList, s_FragmentCheckedList)
		//! \param[in] fromFragment
		//! \param[in] damageType
		//! \param[in] damage
		//! \param[in] hitPosition
		//! \param[in] hitDirection
		protected void UpdateStructuralIntegrity(SCR_FragmentEntity fromFragment, EDamageType damageType, float damage, vector hitPosition, vector hitDirection)
		{
			// First get ordered list of existing fragments
			int numFragments = FillCompleteOrderedFragmentList(s_FragmentList);

			// Exclude the 'from' fragment from the list
			s_FragmentList[fromFragment.GetIndex()] = null;

			// If we don't have a hierarchy, ignore
			if (!m_UsedFractalData.m_Hierarchy)
				return;

			// Get the linkage node for the 'from' fragment
			SCR_FragmentLinkage fromLinkage = m_UsedFractalData.m_Hierarchy.GetFragmentLinkage(fromFragment.GetIndex());
			if (!fromLinkage)
				return;

			// Add all directly linked fragments to the 'to check' list
			s_FragmentToCheckList.Clear();
			for (int i = fromLinkage.m_aOtherIndexes.Count() - 1; i >= 0; i--)
			{
				SCR_FragmentEntity neighborFragment = s_FragmentList[fromLinkage.m_aOtherIndexes[i]];
				if (!neighborFragment)
					continue;

				s_FragmentToCheckList.Insert(neighborFragment);
			}

			// Now go through each fragment in the 'to check' list and find whether it is connected to an anchor
			while (!s_FragmentToCheckList.IsEmpty())
			{
				SCR_FragmentEntity checkFragment = s_FragmentToCheckList[0];
				s_FragmentToCheckList.RemoveItem(checkFragment);

				// Check if anchored, if not remove from lists, add any connected fragments to the check list and queue its deletion
				s_FragmentCheckedList.Clear();
				if (!CheckFragmentAnchored(checkFragment, numFragments))
				{
					s_FragmentList[checkFragment.GetIndex()] = null;

					SCR_FragmentLinkage linkage = m_UsedFractalData.m_Hierarchy.GetFragmentLinkage(checkFragment.GetIndex());
					if (linkage)
					{
						int numOther = linkage.m_aOtherIndexes.Count();
						for (int i = 0; i < numOther; i++)
						{
							int otherIndex = linkage.m_aOtherIndexes[i];
							if (otherIndex < 0 || otherIndex >= numFragments)
								continue;

							SCR_FragmentEntity neighborFragment = s_FragmentList[otherIndex];
							if (!neighborFragment)
								continue;

							if (s_FragmentToCheckList.Find(neighborFragment) == -1)
								s_FragmentToCheckList.Insert(neighborFragment);
						}
					}

					checkFragment.QueueDestroy(damageType, damage, hitPosition, hitDirection, false);
				}
			}
		}

		//------------------------------------------------------------------------------------------------
		//! Performs a structural integrity step check on the input fragment and returns true if the fragment is connected to an anchor (uses lists s_FragmentList, s_FragmentCheckedList)
		//! \param[in] fragment
		//! \param[in] numFragments
		private bool CheckFragmentAnchored(SCR_FragmentEntity fragment, int numFragments)
		{
			if (s_FragmentCheckedList.Find(fragment) >= 0)
				return false;

			s_FragmentCheckedList.Insert(fragment);

			SCR_FragmentLinkage linkage = m_UsedFractalData.m_Hierarchy.GetFragmentLinkage(fragment.GetIndex());
			if (!linkage)
				return false;

			if (linkage.m_bIsAnchor)
				return true;

			int numOther = linkage.m_aOtherIndexes.Count();
			for (int i = 0; i < numOther; i++)
			{
				int otherIndex = linkage.m_aOtherIndexes[i];
				if (otherIndex < 0 || otherIndex >= numFragments)
					continue;

				SCR_FragmentEntity neighborFragment = s_FragmentList[otherIndex];
				if (!neighborFragment)
					continue;

				if (CheckFragmentAnchored(neighborFragment, numFragments))
					return true;
			}

			return false;
		}

		//------------------------------------------------------------------------------------------------
		//! Called when one of the pieces is destroyed
		//! \param[in] fragment
		//! \param[in] damageType
		//! \param[in] damage
		//! \param[in] hitPosition
		//! \param[in] hitDirection
		void OnFragmentDestroyed(SCR_FragmentEntity fragment, EDamageType damageType, float damage, vector hitPosition, vector hitDirection)
		{
			if (m_bDeleteAfterFinalFragment)
				EnableOnFrame(true); // Enable frame, will check if all fragments gone

			SCR_MPDestructionManager manager = SCR_MPDestructionManager.GetInstance();
			if (!manager)
				return;

			//TODO Send hit data with our own rpc
			//manager.SendHitData(this, fragment.GetIndex(), damageType, damage, hitPosition, hitDirection);

			if (m_bEnableStructuralIntegrity)
				UpdateStructuralIntegrity(fragment, damageType, damage, hitPosition, hitDirection);
		}

		//------------------------------------------------------------------------------------------------
		//! Receive encoded hit data from server
		//! \param[in] hitIndex
		//! \param[in] damageType
		//! \param[in] damage
		//! \param[in] hitPosition
		//! \param[in] hitDirection
		override void NetReceiveHitData(int hitIndex, EDamageType damageType, float damage, vector hitPosition, vector hitDirection)
		{
			SCR_FragmentEntity fragment = FindFragment(hitIndex);
			if (!fragment)
				return;

			if (m_bEnableStructuralIntegrity)
				UpdateStructuralIntegrity(fragment, damageType, damage, hitPosition, hitDirection);

			fragment.QueueDestroy(damageType, damage, hitPosition, hitDirection);
		}

		//------------------------------------------------------------------------------------------------
		override void OnFrame(IEntity owner, float timeSlice)
		{
			// If we are meant to delete after the final fragment, do so
			if (m_bDeleteAfterFinalFragment && !m_DestructionHitInfo)
			{
				if (CountFragments() <= 0)
					DeleteSelf();
				else
					EnableOnFrame(false);
			}
			else
			{
				super.OnFrame(owner, timeSlice);
		}
		}

		//------------------------------------------------------------------------------------------------
		//! Updates the model to reflect damaged/destroyed status
		//! \param[in] forceState
		protected void UpdateModel(SCR_EFractalDestructionForceModel forceState = SCR_EFractalDestructionForceModel.FORCE_NONE)
		{
			if (!m_UsedFractalData)
				return;

			bool fractured = GetFractured();
			if (forceState == SCR_EFractalDestructionForceModel.FORCE_UNFRACTURED)
				fractured = false;
			else if (forceState == SCR_EFractalDestructionForceModel.FORCE_FRACTURED)
				fractured = true;

			ResourceName assetPath;
			if (fractured)
				assetPath = m_UsedFractalData.m_ModelDestroyed;
			else
				assetPath = m_UsedFractalData.m_ModelNormal;

			// Update physics and visual model
			Physics phys = m_Owner.GetPhysics();
			if (phys)
				phys.Destroy();

			if (assetPath == ResourceName.Empty)
			{
				m_Owner.SetObject(null, string.Empty);
				m_Owner.ClearFlags(EntityFlags.VISIBLE, false);
				m_Owner.Update();
			}
			else
			{
				Resource resource = Resource.Load(assetPath);
				if (resource.IsValid())
				{
					BaseResourceObject baseResource = resource.GetResource();
					if (baseResource)
					{
						VObject asset = baseResource.ToVObject();
						if (asset)
						{
							m_Owner.SetObject(asset, string.Empty);
							m_Owner.SetFlags(EntityFlags.VISIBLE, false);
							m_Owner.Update();

							Physics.CreateStatic(m_Owner, -1);
						}
						else
							Print("FRACTAL DESTRUCTION::UpdateModel: Could not load visual object '" + assetPath + "'!!", LogLevel.WARNING);
					}
					else
						Print("FRACTAL DESTRUCTION::UpdateModel: Could not load base resource for model '" + assetPath + "'!!", LogLevel.WARNING);
				}
				else
					Print("FRACTAL DESTRUCTION::UpdateModel: Could not load model '" + assetPath + "'!!", LogLevel.WARNING);
			}

			//UpdatePhysicsInteractionLayers();
		}

		//------------------------------------------------------------------------------------------------
		//! Handle destruction
		override void HandleDestruction()
		{
			UpdateModel();

			// Only create fragments if not in total destruction
			if (!m_DestructionHitInfo.m_TotalDestruction)
			{
				CreateFragments(true);

				if (m_bDestroyFragmentOnFracture)
				{
					// Now try to trace the fragment at the position we hit
					TraceParam param = new TraceParam();
					param.Exclude = m_Owner;
					param.Start = m_DestructionHitInfo.m_HitPosition + m_DestructionHitInfo.m_HitDirection * -0.25;
					param.End = m_DestructionHitInfo.m_HitPosition + m_DestructionHitInfo.m_HitDirection * 0.25;
					param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
					param.LayerMask = -1;
					if (m_Owner.GetWorld().TraceMove(param, SCR_Global.FilterCallback_IgnoreNotInList) < 1)
					{
						IEntity child = m_Owner.GetChildren();
						while (child)
						{
							SCR_FragmentEntity fragment = SCR_FragmentEntity.Cast(child);
							child = child.GetSibling();
							if (fragment && param.TraceEnt == fragment)
							{
								//delete fragment;
								fragment.QueueDestroy(m_DestructionHitInfo.m_DamageType, m_DestructionHitInfo.m_HitDamage, m_DestructionHitInfo.m_HitPosition, m_DestructionHitInfo.m_HitDirection);
								UpdateStructuralIntegrity(fragment, m_DestructionHitInfo.m_DamageType, m_DestructionHitInfo.m_HitDamage, m_DestructionHitInfo.m_HitPosition, m_DestructionHitInfo.m_HitDirection);
								break;
							}
						}
					}
				}
			}

			delete m_DestructionHitInfo;
		}

		//------------------------------------------------------------------------------------------------
		//! Called when Item is initialized from replication stream. Carries the data from Master.
		//! \param[in] reader
		override void NetReadInit(ScriptBitReader reader)
		{
			int fractalVariantIndex;
			reader.Read(fractalVariantIndex, 32); // Read which fractal variant is used
			if (fractalVariantIndex == -1)
				return;

			DeleteFragments();

			SetHitZoneDamage(GetMaxHealth());

			m_UsedFractalData = m_FractalVariants[fractalVariantIndex];
			UpdateModel(SCR_EFractalDestructionForceModel.FORCE_FRACTURED);

			int numBitMasks;
			reader.Read(numBitMasks, 32); // Read num bitmasks
			if (numBitMasks == 0)
				return;

			// Create bitmask array with values of fragments
			SCR_BitMaskArray fragmentsBitMaskArray = new SCR_BitMaskArray(numBitMasks);
			for (int i = 0; i < numBitMasks; i++)
			{
				int bitMask;
				reader.Read(bitMask, 32); // Read fragment bitmask
				fragmentsBitMaskArray.SetBitMask(i, bitMask);
			}

			// Now create the fragments stored in the bitmask
			int numFragmentsMax = m_UsedFractalData.CountFragments();
			for (int i = 0; i < numFragmentsMax; i++)
			{
				if (fragmentsBitMaskArray.GetBit(i))
					CreateFragment(i);
			}
		}

		//------------------------------------------------------------------------------------------------
		//! Called when Item is getting replicated from Master to Slave connection.
		override void NetWriteInit(ScriptBitWriter writer)
		{
			int fractalVariantIndex = -1;
			if (m_UsedFractalData)
			{
				fractalVariantIndex = m_FractalVariants.Find(m_UsedFractalData);
				writer.Write(fractalVariantIndex, 32); // Write which fractal variant is used
			}
			else
			{
				writer.Write(-1, 32); // Write null fractal variant
				return;
			}

			array<int> fragmentIndexList = {};
			int numFragments = FillFragmentIndexList(fragmentIndexList);

			if (numFragments == 0) // All were destroyed
			{
				writer.Write(0, 32); // Write that there are no bitmasks
				return;
			}

			// Now write existing fragments into a bitmask array that we can send (for compression)
			SCR_BitMaskArray fragmentsBitMaskArray = new SCR_BitMaskArray(m_UsedFractalData.CountFragments());
			int numBitMasks = fragmentsBitMaskArray.GetNumBitMasks();
			writer.Write(numBitMasks, 32); // Write num bitmasks

			// Write fragment bits into bitmask array
			for (int i = 0; i < numFragments; i++)
			{
				fragmentsBitMaskArray.SetBit(fragmentIndexList[i], true);
			}

			// Write bitmask arrays
			for (int i = 0; i < numBitMasks; i++)
			{
				int bitMask = fragmentsBitMaskArray.GetBitMask(i);
				writer.Write(bitMask, 32); // Write fragment bitmask
			}
		}

		//------------------------------------------------------------------------------------------------
		//! Initialise destruction
		override void InitDestruction()
		{
			m_UsedFractalData = GetRandomFractalVariant();
			UpdateModel();
		}

		//------------------------------------------------------------------------------------------------
		//! Destructor
		void ~SCR_DestructionFractalComponent()
		{
			#ifdef WORKBENCH
				ClearVisualizers();
			#endif

			if (GetFractured())
				DeleteFragments();
		}
	#endif // ENABLE_DESTRUCTION
}

class SCR_FractalVariationTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		array<ResourceName> fragMdls = {};
		array<ResourceName> fragAnchorMdls = {};
		source.Get("m_aModelFragments", fragMdls);
		source.Get("m_aModelAnchorFragments", fragAnchorMdls);
		int num = 0;
		if (fragMdls)
			num = fragMdls.Count();

		if (fragAnchorMdls)
			num += fragAnchorMdls.Count();

		title = "Variation | FRAGS: " + num.ToString();
		return true;
	}
}

[BaseContainerProps(), SCR_FractalVariationTitle()]
class SCR_FractalVariation
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Model to use when the object is undamaged", "xob")]
	protected ResourceName m_ModelNormal;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Model to use when the object is damaged/destroyed", "xob")]
	protected ResourceName m_ModelDestroyed;

	[Attribute("", UIWidgets.ResourceAssignArray, "List of fragment models (excluding anchor fragments)", "xob")]
	protected ref array<ResourceName> m_aModelFragments;

	[Attribute("", UIWidgets.ResourceAssignArray, "List of anchor fragment models (these are fragments that are considered firmly attached and hold other fragments in place, if structural integrity is enabled)", "xob")]
	protected ref array<ResourceName> m_aModelAnchorFragments;

	[Attribute("", UIWidgets.ResourceAssignArray, "List of fragment debris models (excluding anchor fragments), if empty uses m_aModelFragments", "xob")]
	protected ref array<ResourceName> m_aDebrisModelFragments;

	[Attribute("", UIWidgets.ResourceAssignArray, "List of anchor fragment debris models (these are fragments that are considered firmly attached and hold other fragments in place, if structural integrity is enabled), if empty uses m_aModelAnchorFragments", "xob")]
	protected ref array<ResourceName> m_aDebrisModelAnchorFragments;

	[Attribute("", UIWidgets.Object, "Hierarchy between fragments")]
	protected ref SCR_FragmentHierarchy m_Hierarchy;

	//------------------------------------------------------------------------------------------------
	//! \return how many fragments are in the list (including anchor fragments)
	int CountFragments()
	{
		return m_aModelFragments.Count() + m_aModelAnchorFragments.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \return whether the input fragment index is an anchor
	bool GetFragmentIndexIsAnchor(int index)
	{
		int numFrags = m_aModelFragments.Count();
		int numAnchorFrags = m_aModelAnchorFragments.Count();
		if (index >= (numFrags + numAnchorFrags))
			return false;

		return index >= numFrags;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] index
	//! \param[out] isAnchor
	//! \return the model to use for the input fragment index
	ResourceName GetFragmentModel(int index, out bool isAnchor)
	{
		isAnchor = false;
		if (index < 0)
			return ResourceName.Empty;

		int numFrags = m_aModelFragments.Count();
		int numAnchorFrags = m_aModelAnchorFragments.Count();
		if (index >= (numFrags + numAnchorFrags))
			return ResourceName.Empty;

		if (index < numFrags)
			return m_aModelFragments[index];
		else
		{
			isAnchor = true;
			return m_aModelAnchorFragments[index - numFrags];
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] index
	//! \param[out] isAnchor
	//! \return the debris model to use for the input fragment index
	ResourceName GetFragmentDebrisModel(int index, out bool isAnchor)
	{
		isAnchor = false;
		if (index < 0)
			return ResourceName.Empty;

		int numFrags = m_aModelFragments.Count();
		int numAnchorFrags = m_aModelAnchorFragments.Count();
		if (index >= (numFrags + numAnchorFrags))
			return ResourceName.Empty;

		if (index < numFrags)
		{
			if (!m_aDebrisModelFragments || m_aDebrisModelFragments.Count() <= index) // No debris model defined, so use normal model
				return m_aModelFragments[index];
			else
				return m_aDebrisModelFragments[index];
		}
		else
		{
			isAnchor = true;
			index -= numFrags;
			if (! m_aDebrisModelAnchorFragments || m_aDebrisModelAnchorFragments.Count() <= index) // No anchor debris model defined, so use normal anchor model
				return m_aModelAnchorFragments[index];
			else
				return m_aDebrisModelAnchorFragments[index];
		}
	}
}

class SCR_Spawnable_FragmentDebrisTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Fragment Debris";
		return true;
	}
}

class SCR_FragmentHierarchyTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Fragment Hierarchy";
		return true;
	}
}

class SCR_FragmentLinkageTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		int index = -1;
		bool isAnchor = false;
		source.Get("m_iIndex", index);
		source.Get("m_bIsAnchor", isAnchor);
		if (index == -1)
			title = "INVALID INDEX";
		else if (isAnchor)
			title = "Anchor | Index: " + index.ToString();
		else
			title = "------ | Index: " + index.ToString();

		return true;
	}
}

[BaseContainerProps(), SCR_Spawnable_FragmentDebrisTitle()]
class SCR_FragmentDebris
{
	[Attribute("10", UIWidgets.Slider, "Mass of the debris", "0.01 1000 0.01")]
	protected float m_fMass;

	[Attribute("5", UIWidgets.Slider, "Minimum lifetime value for the debris (in s)", "0 3600 0.5")]
	protected float m_fLifetimeMin;

	[Attribute("10", UIWidgets.Slider, "Maximum lifetime value for the debris (in s)", "0 3600 0.5")]
	protected float m_fLifetimeMax;

	[Attribute("200", UIWidgets.Slider, "Maximum distance from camera above which the debris is not spawned (in m)", "0 3600 0.5")]
	protected float m_fDistanceMax;

	[Attribute("0", UIWidgets.Slider, "Higher priority overrides lower priority if at or over debris limit", "0 100 1")]
	protected int m_fPriority;

	[Attribute("0.1", UIWidgets.Slider, "Damage received to physics impulse (speed / mass) multiplier", "0 10000 0.01")]
	protected float m_fDamageToImpulse;

	[Attribute("0.5", UIWidgets.Slider, "Random linear velocity multiplier (m/s)", "0 200 0.1")]
	protected float m_fRandomVelocityLinear;

	[Attribute("180", UIWidgets.Slider, "Random angular velocity multiplier (deg/s)", "0 3600 0.1")]
	protected float m_fRandomVelocityAngular;

	//------------------------------------------------------------------------------------------------
	//! Spawns the object
	//! \param[in] fragment
	//! \param[in] parentPhysics
	//! \param[in] damage
	//! \param[in] hitDirection
	void Spawn(SCR_FragmentEntity fragment, Physics parentPhysics, float damage, vector hitDirection)
	{
		#ifndef ENABLE_DESTRUCTION
			return;
		#else
			SCR_FractalVariation fractalVariation = fragment.GetDestructibleParent().GetCurrentFractalVariant();
			if (!fractalVariation)
				return;

			int fragmentIndex = fragment.GetIndex();

			bool isAnchor;
			ResourceName modelPath = fractalVariation.GetFragmentDebrisModel(fragmentIndex, isAnchor);
			if (modelPath == ResourceName.Empty)
				return;

			vector spawnMat[4];
			fragment.GetTransform(spawnMat);

			float dmgSpeed = damage * m_fDamageToImpulse / m_fMass;

			vector linearVelocity = hitDirection * Math.RandomFloat(0, 1);
			linearVelocity += Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * m_fRandomVelocityLinear;
			linearVelocity *= dmgSpeed;
			vector angularVelocity = Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * Math.RandomFloat(0.25, 4) * m_fRandomVelocityAngular;
			angularVelocity *= dmgSpeed;

			if (parentPhysics)
			{
				linearVelocity += parentPhysics.GetVelocity();
				angularVelocity += parentPhysics.GetAngularVelocity();
			}

			SCR_DebrisSmallEntity debris = SCR_DebrisSmallEntity.SpawnDebris(fragment.GetWorld(), spawnMat, modelPath, m_fMass, Math.RandomFloat(m_fLifetimeMin, m_fLifetimeMax), m_fDistanceMax, m_fPriority, linearVelocity, angularVelocity);
		#endif // ENABLE_DESTRUCTION
	}
}

[BaseContainerProps(), SCR_FragmentHierarchyTitle()]
class SCR_FragmentHierarchy
{
	[Attribute(desc: "Hierarchical list of fragments containing which fragments they are connected to")]
	protected ref array<ref SCR_FragmentLinkage> m_aLinks;

	#ifdef ENABLE_DESTRUCTION
		//------------------------------------------------------------------------------------------------
		//! \param[in] index
		//! \return the fragment linkage object for the input index
		SCR_FragmentLinkage GetFragmentLinkage(int index)
		{
			int numLinks = m_aLinks.Count();
			foreach (SCR_FragmentLinkage link : m_aLinks)
			{
				if (link.m_iIndex == index)
				return link;
			}

			return null;
		}

		#ifdef WORKBENCH
			//------------------------------------------------------------------------------------------------
			//! Validates linkage in the hierarchy and returns true if it was changed
			//! \param[in] fractalVariant
			//! \param[in] srcHierarchy
			//! \param[in] variantIndex
			//! \return
			bool ValidateHierarchy(SCR_FractalVariation fractalVariant, BaseContainer srcHierarchy, int variantIndex)
			{
				if (!fractalVariant)
					return false;

				if (!m_aLinks)
					return false;

				bool result = false;

				BaseContainerList srcLinks = srcHierarchy.GetObjectArray("m_aLinks");
				array<int> foundIndexes = {};
				int numFrags = fractalVariant.CountFragments();
				int numLinks = m_aLinks.Count();
				for (int l = 0; l < numLinks; l++)
				{
					BaseContainer srcLink = srcLinks.Get(l);
					int srcLinkIndex = -1;
					if (!srcLink.Get("m_iIndex", srcLinkIndex))
						continue;

					bool srcLinkIsAnchor = false;
					if (!srcLink.Get("m_bIsAnchor", srcLinkIsAnchor))
						continue;

					// Found a duplicate, so change index
					if (foundIndexes.Find(srcLinkIndex) != -1 || srcLinkIndex < -1 || srcLinkIndex >= numFrags)
					{
						Print("SCR_DestructionFractalComponent: Bad index (" + srcLinkIndex.ToString() + ") detected in linkage object index " + l.ToString() + " in variant index " + variantIndex.ToString() + ", setting to -1", LogLevel.WARNING);
						srcLinkIndex = -1;
						srcLink.Set("m_iIndex", srcLinkIndex);
						BaseContainerTools.WriteToInstance(this, srcLink);
						result = true;
						continue;
					}

					foundIndexes.Insert(srcLinkIndex);

					// Mismatch in stored anchor value to true anchor value, so update
					if (srcLinkIsAnchor != fractalVariant.GetFragmentIndexIsAnchor(srcLinkIndex))
					{
						Print("SCR_DestructionFractalComponent: Updating anchor setting in linkage object index " + l + " in variant index " + variantIndex, LogLevel.NORMAL);
						srcLinkIsAnchor = !srcLinkIsAnchor;
						srcLink.Set("m_bIsAnchor", srcLinkIsAnchor);
						BaseContainerTools.WriteToInstance(this, srcLink);
						result = true;
					}

					// Check other indexes in linkage
					array<int> srcOtherLinks;
					srcLink.Get("m_aOtherIndexes", srcOtherLinks);
					bool badOtherIndex = false;
					int numOtherLinks = srcOtherLinks.Count();
					for (int o = 0; o < numOtherLinks; o++)
					{
						int srcLinkOtherIndex = srcOtherLinks[o];
						if (srcLinkOtherIndex < -1 || srcLinkOtherIndex >= numFrags)
						{
							Print("SCR_DestructionFractalComponent: Bad other index (" + srcLinkOtherIndex.ToString() + ") detected in linkage object index " + l.ToString() + " in variant index " + variantIndex.ToString() + ", setting to -1", LogLevel.WARNING);
							srcOtherLinks[o] = -1;
							badOtherIndex = true;
						}
					}

					if (badOtherIndex)
					{
						srcLink.Set("m_aOtherIndexes", srcOtherLinks);
						BaseContainerTools.WriteToInstance(this, srcLink);
						result = true;
					}
				}

				if (result)
					BaseContainerTools.WriteToInstance(this, srcHierarchy);

				return result;
			}

			//------------------------------------------------------------------------------------------------
			//! Generate the hierarchical linkage structure between all fragments
			//! \param[in] fractalVariant
			void GenerateHierarchy(SCR_FractalVariation fractalVariant)
			{
				if (m_aLinks)
				m_aLinks.Clear();
				else
					m_aLinks = {};

				if (!fractalVariant)
					return;

				// First count fragment model counts
				int numFrags = fractalVariant.m_aModelFragments.Count();
				int numAnchorFrags = fractalVariant.m_aModelAnchorFragments.Count();
				int numTotal = numFrags + numAnchorFrags;

				// Create a generic entity for getting bounds sizes and then load each model and get its bounds
				GenericEntity fragmentDummy = GenericEntity.Cast(GetGame().SpawnEntity(GenericEntity));
				array<vector> fragment_mins = {};
				array<vector> fragment_maxs = {};
				for (int f = 0; f < numFrags; f++)
				{
					Resource resource = Resource.Load(fractalVariant.m_aModelFragments[f]);
					VObject asset = resource.GetResource().ToVObject();
					fragmentDummy.SetObject(asset, "");

					vector mins, maxs;
					fragmentDummy.GetBounds(mins, maxs);
					fragment_mins.Insert(mins);
					fragment_maxs.Insert(maxs);
				}

				for (int f = 0; f < numAnchorFrags; f++)
				{
					Resource resource = Resource.Load(fractalVariant.m_aModelAnchorFragments[f]);
					VObject asset = resource.GetResource().ToVObject();
					fragmentDummy.SetObject(asset, "");

					vector mins, maxs;
					fragmentDummy.GetBounds(mins, maxs);
					fragment_mins.Insert(mins);
					fragment_maxs.Insert(maxs);
				}

				// Now do bounding box overlaps and find out which fragments overlap
				for (int f = 0; f < numTotal; f++)
				{
					SCR_FragmentLinkage fragLinkage = null;

					vector fragMins = fragment_mins[f];
					vector fragMaxs = fragment_maxs[f];
					for (int f2 = 0; f2 < numTotal; f2++)
					{
						if (f == f2)
							continue;

						vector fragOtherMins = fragment_mins[f2];
						vector fragOtherMaxs = fragment_maxs[f2];

						if (IntersectionBoxBox(fragMins, fragMaxs, fragOtherMins, fragOtherMaxs))
							continue;

						// Frag linkage object not created yet, so create
						if (!fragLinkage)
						{
							fragLinkage = new SCR_FragmentLinkage();
							fragLinkage.m_bIsAnchor = f >= numFrags;
							fragLinkage.m_iIndex = f;
							fragLinkage.m_aOtherIndexes = {};
							m_aLinks.Insert(fragLinkage);
						}

						fragLinkage.m_aOtherIndexes.Insert(f2);
					}
				}

				delete fragmentDummy;
			}

			//------------------------------------------------------------------------------------------------
			//!
			//! \param[in] mins1
			//! \param[in] maxs1
			//! \param[in] mins2
			//! \param[in] maxs2
			//! \return
			bool IntersectionBoxBox(vector mins1, vector maxs1, vector mins2, vector maxs2)
			{
				return (mins1[0] > maxs2[0] || mins1[1] > maxs2[1] || mins1[2] > maxs2[2] || maxs1[0] < mins2[0] || maxs1[1] < mins2[1] || maxs1[2] < mins2[2]);
			}
		#endif // WORKBENCH
	#endif // ENABLE_DESTRUCTION
}

[BaseContainerProps(), SCR_FragmentLinkageTitle()]
class SCR_FragmentLinkage
{
	[Attribute("0", UIWidgets.None, "Whether the fragment is an anchor")]
	bool m_bIsAnchor;

	[Attribute("-1", UIWidgets.EditBox, "Index of the fragment")]
	int m_iIndex;

	[Attribute("", UIWidgets.EditBox, "List of indexes of the surrounding fragments")]
	ref array<int> m_aOtherIndexes;
}
