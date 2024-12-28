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
        return m_lockComponent != null;
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
	// to improve performance we will do all checks on perform and not in canPerform
	// this also allows to inform the user only on interaction and not by disabling actions "by default"
	//------------------------------------------------------------------------------------------------
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
		if (!pOwnerEntity || !m_door) {
			Print("no owner or no m_door in DoorLockAction");
			return;
		}
		
		if (Math.AbsFloat(m_door.GetControlValue()) > 0) {
			ShowHint("Please close door first, dumbass", "Locking impossible");
			return;
		}
		
		if (!m_lockComponent) {
			ShowHint("I see no lock :/ (report to mod author)", "No lock");
			return;
		} else {
			SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
			bool isGM = SCR_EditorManagerEntity.IsOpenedInstance();
			
			HandleOwner(isGM, character, pUserEntity);
			string lockOwnerString = m_lockComponent.GetLockOwner();
			
			bool m_islocked = m_lockComponent.GetLockState();
			bool m_canunlock = CanUseLock(lockOwnerString, isGM, character, pUserEntity, m_islocked);
			
			if (m_canunlock) {
				m_lockComponent.ToggleLockState(pUserEntity);
			} else {
				ShowHint("I dont have a key for this door", "No key");
			}
		}
    }
	
	private void HandleOwner(bool isGM, SCR_ChimeraCharacter character, IEntity user) {
		// get initial lock owner which might get overriden in this method
		string lockOwnerString = m_lockComponent.GetLockOwner();
		
		// no one owns so first come first serve
		if (lockOwnerString == "") {
			if (isGM || character) {
				if (isGM) {
					m_lockComponent.SetLockOwner("GM");
					ShowHint("Gamemaster is lock owner now", "Lock owner set");
				}
				// override if GM is remote controlling?
				if (character) {
					string m_factionKey = character.GetFactionKey();
					m_lockComponent.SetLockOwner(m_factionKey);
					string m_text = m_factionKey + "is lock owner now";
					ShowHint(m_text, "Lock owner set");
				}
			} else {
				Print("User is neither GM nor character, what is it?");
			}
		} else {
			PrintFormat("lockOwnerString is %1 in HandleOwner", lockOwnerString);
		}
	}
	
	private bool CanUseLock(string lockOwnerString, bool isGM, SCR_ChimeraCharacter character, IEntity user, bool islocked) {		
		if (character) {
			PrintFormat("Checking if player is inside");
			if (IsInside(character)) {
				PrintFormat("Lock user is inside");
				return true;
			}
			if (character.GetFactionKey() == lockOwnerString) {
				PrintFormat("Lock Owner matches User (%1)", lockOwnerString);
				return true;
			}
		} else {
			if (isGM) {
				if (islocked) {
					ShowHint("Unlocked door. Owner is " + lockOwnerString, "Unlocked");
					return true;
				} else {
					ShowHint("Locked door. Owner is GM now", "Locked");
					m_lockComponent.SetLockOwner("GM");
					return true;
				}
			}
		}
		return false;
	}

  
    // Helper method to show hints
    private void ShowHint(string message, string title, bool isSilent = false)
    {
        SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
        if (!hintManager)
            return;

        hintManager.ShowCustomHint(message, title, 12, isSilent, EHint.UNDEFINED, false);
    }
};
