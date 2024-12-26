[ComponentEditorProps(category: "Custom", description: "Door Lock Component")]
class GRAD_DoorLockComponentClass : ScriptComponentClass {
}

class GRAD_DoorLockComponent : ScriptComponent
{
    // Variables exposed to the editor
    [Attribute("0", UIWidgets.CheckBox, "Door Lock State", "" )]
    bool m_isLocked;

    // Called when the game initializes the component
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        Print("Door Lock Component Initialized with: " + m_isLocked.ToString());
    }
	
	bool GetLockState() 
	{
		return m_isLocked;
	}
	
	void ToggleLockState() 
	{
		if (m_isLocked) {
       		ShowHint("door unlocked", "Unlocked");
		} else {
        	ShowHint("door locked", "Locked");
		}
		m_isLocked = !m_isLocked;
		Print("Door Lock Component set to: " + m_isLocked.ToString());
	}
	
	// Helper method to show hints
    private void ShowHint(string message, string title, bool isPersistent = false)
    {
        SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
        if (!hintManager)
            return;

        hintManager.ShowCustomHint(message, title, 12, isPersistent, EHint.UNDEFINED, false);
    }
}