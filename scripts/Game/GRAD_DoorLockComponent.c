[ComponentEditorProps(category: "Custom", description: "Door Lock Component")]
class GRAD_DoorLockComponentClass : ScriptComponentClass {
}

class GRAD_DoorLockComponent : ScriptComponent
{
    // Variables exposed to the editor
    [Attribute("1", UIWidgets.CheckBox, "Door Lock State", "" )]
    bool m_isLocked;

    // Called when the game initializes the component
    override void EOnInit(IEntity owner)
    {
        super.EOnInit(owner);
        Print("Door Lock Component Initialized with: " + m_isLocked.ToString());
    }
	
	bool GetLockState() 
	{
		return m_isLocked;
	}
}