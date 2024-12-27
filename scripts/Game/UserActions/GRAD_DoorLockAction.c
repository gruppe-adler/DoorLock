//! User action that ought to be attached to an entity with door component.
//! When performed either locks or unlcoks the door based on the previous state of the door.
// Script File
class GRAD_DoorLockAction : ScriptedUserAction
{
	DoorComponent m_door;
	GRAD_DoorLockComponent m_lockComponent;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) {
		
		DoorComponent doorComp = DoorComponent.Cast(pOwnerEntity.FindComponent(DoorComponent));
		
		if (!doorComp) {
			// Print("No doorComp found in DoorLockAction");
			return;
		} else {
			m_door = doorComp;
			// Print("m_door filled with doorComp");
			
			m_lockComponent = GRAD_DoorLockComponent.Cast(pOwnerEntity.FindComponent(GRAD_DoorLockComponent));
			if (!m_lockComponent) {
				// Print("m_lockComponent NOT initially set");
				return;
			} else {
				// PrintFormat("m_lockComponent initially set to %1", m_lockComponent.GetLockState());
			}
		}
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (!m_lockComponent)
			return false;
		
		if (m_lockComponent.GetLockState()) { 
			outName = "Unlock";
			return true;
		} else { 
			outName = "Lock";
			return true;
		};		
	}
	
	//------------------------------------------------------------------------------------------------
    override bool CanBePerformedScript(IEntity user)
    {		
		if (IsInside(user)) {
			PrintFormat("Lock user is inside");
			return true;
		}
		
		string lockOwnerString = m_lockComponent.GetLockOwner();
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		
		if (character) {
			if (character.GetFactionKey() == lockOwnerString) {
				PrintFormat("Lock Owner matches User (%1)", lockOwnerString);
				return true;
			}
		} else {
			bool isGM = SCR_EditorManagerEntity.IsOpenedInstance();
			if (isGM) {
				if (lockOwnerString == "GM") {
					PrintFormat("Lock Owner matches User (GM)");
					return true;
				}
			}
		}
		// no one owns so first come first serve
		if (lockOwnerString == "") {
			return true;
		}
		return false;
    }
	
	//------------------------------------------------------------------------------------------------
	//! Checks whether or not an entity is inside of the building, using a trace in each world axis
	//! \param[in] entity
	protected bool IsInside(notnull IEntity entity)
	{
		IEntity owner = GetOwner();
		BaseWorld world = owner.GetWorld();
		vector start = entity.GetOrigin();
		
		TraceParam param = new TraceParam();
		param.Flags = TraceFlags.ENTS;
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		param.Include = owner; // Include only the building for performance reasons
		protected static const vector TRACE_DIRECTIONS[3] = { vector.Right, vector.Up, vector.Forward };
		
		bool result;
		for (int i = 0; i < 3; i++)
		{
			float lengthMultiplier = 1;
			if (i == 1)
				lengthMultiplier = 100; // Vertical traces can and must be long to detect roof, where there is no floor, also they are internally optimized
			
			result = PerformTrace(param, start, TRACE_DIRECTIONS[i], world, lengthMultiplier);
			
			if (result)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool PerformTrace(notnull TraceParam param, vector start, vector direction, notnull BaseWorld world, float lengthMultiplier = 1)
	{
		param.Start = start - direction * lengthMultiplier;
		param.End = start + direction * lengthMultiplier;
		world.TraceMove(param, TraceFilter);
		
		return param.TraceEnt != null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool TraceFilter(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0")
	{
		return e == GetOwner();
	}
	
    //------------------------------------------------------------------------------------------------
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
		if (!pOwnerEntity) {
			//	ShowHint("no owner", "No owner");
			return;
		}
		
		if (Math.AbsFloat(m_door.GetControlValue()) < 1) {
			ShowHint("Please close door first", "Locking impossible");
		}
		
		if (!m_lockComponent) {
			// Print("m_lockComponent NOT set at runtime");
			return;
		} else {
			m_lockComponent.ToggleLockState(pUserEntity);
			// Print("Toggling door lock");
			string lockOwnerString = m_lockComponent.GetLockOwner();
			bool isGM = SCR_EditorManagerEntity.IsOpenedInstance();
			SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
			
			if (lockOwnerString == "") {
				if (isGM) {
					m_lockComponent.SetLockOwner("GM");
				} else {
					if (character) {
						m_lockComponent.SetLockOwner(character.GetFactionKey());
					}
				}
			}
		}
    }

  
    // Helper method to show hints
    private void ShowHint(string message, string title, bool isPersistent = false)
    {
        SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
        if (!hintManager)
            return;

        hintManager.ShowCustomHint(message, title, 12, isPersistent, EHint.UNDEFINED, false);
    }
};
