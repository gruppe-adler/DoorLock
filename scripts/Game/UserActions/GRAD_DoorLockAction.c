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
        string lockOwnerString = m_lockComponent.GetLockOwner();
		bool isGM = SCR_EditorManagerEntity.IsOpenedInstance();
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (character) {
			if (character.GetFactionKey() == lockOwnerString) {
				PrintFormat("Lock Owner matches User (%1)", lockOwnerString);
				return true;
			}
		} else {
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
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
		if (!pOwnerEntity) {
			//	ShowHint("no owner", "No owner");
			return;
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
