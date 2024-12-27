[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
modded class SCR_UserContextAction: SCR_BaseContextAction
{
	protected SCR_EditableEntityComponent m_HoveredEntity;
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		Print("GRAD_UserContextAction: canBeShown entered");
		if (!hoveredEntity)
			return false;
		
		Print("GRAD_UserContextAction: hovering");
		
		//--- DIRTY HACK - get the entity itself, not the composition the entity is in. ToDo: CLEANER!
		if (!IsServer())
		{
			hoveredEntity = SCR_BaseEditableEntityFilter.GetFirstEntity(EEditableEntityState.HOVER);
			m_HoveredEntity = hoveredEntity;
			
			//~ Tried to set new hovered entity but it was null
			if (!hoveredEntity)
				return false;
		}
		
		BaseActionsManagerComponent actionsManager = BaseActionsManagerComponent.Cast(hoveredEntity.GetOwnerScripted().FindComponent(BaseActionsManagerComponent));
		
		// GRAD EDIT: removed  || actionsManager.GetActionsCount() != 1
		if (!actionsManager) {
			Print("GRAD_UserContextAction: no actionManager found");
			return false;
		}
		
		ScriptedUserAction action = ScriptedUserAction.Cast(actionsManager.GetFirstAction());
		if (!action || !action.CanBePerformed(SCR_EditorManagerEntity.GetInstance())) {
			Print("GRAD_UserContextAction: action not found or cant be performed");
			return false;
		}
		
		//--- Update info with action name
		SCR_OverridableUIInfo info = SCR_OverridableUIInfo.Cast(m_Info);
		if (info)
			info.SetName(action.GetActionName());
		
		return true;
	}
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (!hoveredEntity)
			return;
		
		//--- DIRTY HACK - use cached entity from CanBeShown (at this moment entity under cursor may differ). ToDo: CLEANER!
		if (!IsServer() && hoveredEntity)
		{
			hoveredEntity = m_HoveredEntity;
			
			//~ Tried to set new hovered entity but it was null
			if (!hoveredEntity)
				return;
		}
		
		IEntity owner = hoveredEntity.GetOwnerScripted();
		if (!owner)
			return;
		
		BaseActionsManagerComponent actionsManager = BaseActionsManagerComponent.Cast(owner.FindComponent(BaseActionsManagerComponent));
		// GRAD EDIT: removed  || actionsManager.GetActionsCount() != 1
		if (!actionsManager)
			return;
		
		ScriptedUserAction action = ScriptedUserAction.Cast(actionsManager.GetFirstAction());
		if (!action || !action.CanBePerformed(SCR_EditorManagerEntity.GetInstance()))
			return;
		
		SCR_WorldEntityDelegateEditorComponent manager = SCR_WorldEntityDelegateEditorComponent.Cast(SCR_WorldEntityDelegateEditorComponent.GetInstance(SCR_WorldEntityDelegateEditorComponent));
		if (manager)
			manager.PerformFirstAction(owner);
	}
};