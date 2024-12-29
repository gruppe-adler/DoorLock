[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
modded class SCR_WorldEntityDelegateEditorComponentClass: SCR_BaseEditorComponentClass
{
	static override array<typename> Requires(IEntityComponentSource src)
	{
		return {SCR_MenuLayoutEditorComponent};
	}
};

/** @ingroup Editor_Components
Manager of world entity representation
*/
modded class SCR_WorldEntityDelegateEditorComponent: SCR_BaseEditorComponent
{
	[Attribute(params: "et")]
	protected ResourceName m_EntityPrefab;
	
	protected SCR_MenuLayoutEditorComponent m_MenuLayout;
	protected SCR_EditableWorldEntityComponent m_Delegate;
	protected IEntity m_TraceEntity;
	protected bool m_bHasDelegate;
	protected vector m_vLocalOffset;
	
	/*!
	Perform first action on given object for every user
	\param owner Entity with the action configured
	*/
	override void PerformFirstAction(IEntity owner)
	{
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		RplId rplId = rplComponent.Id();
		
		Rpc(PerformFirstActionServer, rplId);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override protected void PerformFirstActionServer(RplId rplId)
	{
		PerformFirstActionBroadcast(rplId);
		Rpc(PerformFirstActionBroadcast, rplId);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	override protected void PerformFirstActionBroadcast(RplId rplId)
	{
		RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(rplId));
		IEntity owner = rplComponent.GetEntity();
		BaseActionsManagerComponent actionsManager = BaseActionsManagerComponent.Cast(owner.FindComponent(BaseActionsManagerComponent));
		ScriptedUserAction action = ScriptedUserAction.Cast(actionsManager.GetFirstAction());
		action.PerformAction(owner, GetOwner());
	}
	
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{
		IEntity traceEntity = m_MenuLayout.GetTraceEntity();
		
		if (traceEntity != m_TraceEntity)
		{
			//--- Check if world entity deserves a delegate
			m_bHasDelegate = false;
			if (traceEntity)
			{
				SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.GetEditableEntity(traceEntity);
				if (!editableEntity || !editableEntity.HasEntityState(EEditableEntityState.INTERACTIVE))
				{
					BaseActionsManagerComponent actionsManager = BaseActionsManagerComponent.Cast(traceEntity.FindComponent(BaseActionsManagerComponent));
					
					/* Print("GRAD_WorldEntityDelegateEditorComponent entered");
					 originally: if (actionsManager && actionsManager.GetActionsCount() == 1)
					 GRAD changed: removed limitation that action is only shown if one exists
					 only allow multiple actions for doors */
					DoorComponent doorComponent = DoorComponent.Cast(traceEntity.FindComponent(DoorComponent));
					if (actionsManager && actionsManager.GetActionsCount() == 1 || actionsManager && doorComponent)
					{
						ScriptedUserAction action = ScriptedUserAction.Cast(actionsManager.GetFirstAction());
						m_bHasDelegate = action && action.CanBePerformed(GetManager());
						
						if (m_bHasDelegate)
						{
							//--- Get action offset
							array<UserActionContext> contexts = {};
							actionsManager.GetContextList(contexts);
							vector transform[4];
							contexts[0].GetTransformationModel(transform);
							m_vLocalOffset = transform[3];
						}
					}
				}
			}
			
			m_TraceEntity = traceEntity;
			
			if (m_bHasDelegate)
			{
				m_Delegate.RestoreParentEntity();
				m_Delegate.SetOwnerScripted(GenericEntity.Cast(m_TraceEntity));
			}
			else
			{
				m_Delegate.RemoveParentEntity();
				m_Delegate.SetOwnerScripted(null);
			}
		}
		
		//--- Update position to bounding center
		if (m_TraceEntity && m_bHasDelegate)
		{
			vector pos = m_TraceEntity.CoordToParent(m_vLocalOffset);
			m_Delegate.GetOwner().SetOrigin(pos);
			
			//-- Set delegate entity as hovered when just hovering over mesh
			if (traceEntity == m_TraceEntity)
			{
				SCR_HoverEditableEntityFilter hoverFilter = SCR_HoverEditableEntityFilter.Cast(SCR_HoverEditableEntityFilter.GetInstance(EEditableEntityState.HOVER));
				hoverFilter.SetEntityUnderCursor(m_Delegate, true);
			}
		}
	}
	override protected void EOnEditorActivate()
	{
		IEntity delegateEntity = GetGame().SpawnEntityPrefab(Resource.Load(m_EntityPrefab));
		m_Delegate = SCR_EditableWorldEntityComponent.Cast(SCR_EditableEntityComponent.GetEditableEntity(delegateEntity));
		if (!m_Delegate)
		{
			Print(string.Format("Cannot create world entity delegate from prefab @\"%1\", it doesn't have SCR_EditableWorldEntityComponent!"), LogLevel.WARNING);
			RplComponent.DeleteRplEntity(delegateEntity, false);
			return;
		}
		
		SetEventMask(GetOwner(), EntityEvent.FRAME);
		m_MenuLayout = SCR_MenuLayoutEditorComponent.Cast(SCR_MenuLayoutEditorComponent.GetInstance(SCR_MenuLayoutEditorComponent, true));
	}
	override protected void EOnEditorDeactivate()
	{
		if (m_Delegate)
			m_Delegate.Delete();
		ClearEventMask(GetOwner(), EntityEvent.FRAME);
	}
};
