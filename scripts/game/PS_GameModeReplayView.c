class PS_GameModeReplayViewClass: SCR_BaseGameModeClass
{
};

class PS_GameModeReplayView : SCR_BaseGameMode
{
	override void OnGameStart()
	{	
		super.OnGameStart();
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.MapMenuUIReplayView);
	}
};

