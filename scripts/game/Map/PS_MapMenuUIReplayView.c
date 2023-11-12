modded enum ChimeraMenuPreset
{
	MapMenuUIReplayView
}

class PS_MapMenuUIReplayView: ChimeraMenuBase
{	
	protected SCR_MapEntity m_MapEntity;	
	
	ResourceName m_sMarkerPrefab = "{F59F55D675A3E952}UI/layouts/Map/MapMarkerReplayCharacter.layout";
	ResourceName m_sMarkerVehiclePrefab = "{F6E59FDAD044D625}UI/layouts/Map/MapMarkerReplayVehicle.layout";
	ResourceName m_sLinePrefab = "{DCB4940B2B2FE884}UI/layouts/Map/ProjectileLine.layout";
	ResourceName m_sExplosionPrefab = "{022311366521B245}UI/layouts/Map/ExplosionMarker.layout";
	
	int m_iCurrentTime;
	int m_iLastTimeAwait;
	float m_fSpeedScale = 1;
	string m_sReplayFileName;
	FileHandle m_fReplayFile;
	int m_iCurrentReplayPosition;
	bool m_bFastForward;
	
	ref map<RplId, PS_MapMarkerReplayEntityComponent> m_hEntitiesMarkers = new map<RplId, PS_MapMarkerReplayEntityComponent>();
	ref array<PS_MapLineComponent> m_hLinesMarkers = new array<PS_MapLineComponent>();
	ref array<PS_ExplosionMarker> m_hExplosionMarkers = new array<PS_ExplosionMarker>();
	ref map<int, PS_ReplayPlayer> m_hPlayers = new map<int, PS_ReplayPlayer>();
	
	
	SCR_InputButtonComponent m_bNavigationButtonSlower;
	SCR_InputButtonComponent m_bNavigationButtonFaster;
	SCR_InputButtonComponent m_bNavigationButtonToGame;
	
	FrameWidget m_wFrameTimeLine;
	TextWidget m_wSpeedText;
	TextWidget m_wTimeLineText;
	ImageWidget m_wProgressLine;
	ButtonWidget m_wProgressButton;
	EditBoxWidget m_wFilePickEditBox;
	string m_sOldPath = "";
	
	ref PS_ReplayReader replayReader;
	
	override void OnMenuOpen()
	{	
		m_bNavigationButtonSlower = SCR_InputButtonComponent.Cast(GetRootWidget().FindAnyWidget("NavigationButtonSlower").FindHandler(SCR_InputButtonComponent));
		m_bNavigationButtonFaster = SCR_InputButtonComponent.Cast(GetRootWidget().FindAnyWidget("NavigationButtonFaster").FindHandler(SCR_InputButtonComponent));
		m_bNavigationButtonToGame = SCR_InputButtonComponent.Cast(GetRootWidget().FindAnyWidget("NavigationButtonToGame").FindHandler(SCR_InputButtonComponent));
		
		m_bNavigationButtonSlower.m_OnClicked.Insert(Action_Slower);
		m_bNavigationButtonFaster.m_OnClicked.Insert(Action_Faster);
		m_bNavigationButtonToGame.m_OnClicked.Insert(Action_ToGame);
		GetGame().GetInputManager().AddActionListener("ReplaySlower", EActionTrigger.DOWN, Action_Slower);
		GetGame().GetInputManager().AddActionListener("ReplayFaster", EActionTrigger.DOWN, Action_Faster);
		GetGame().GetInputManager().AddActionListener("ReplayToGame", EActionTrigger.DOWN, Action_ToGame);
		
		m_wFrameTimeLine = FrameWidget.Cast(GetRootWidget().FindAnyWidget("FrameTimeLine"));
		m_wSpeedText = TextWidget.Cast(GetRootWidget().FindAnyWidget("SpeedText"));
		m_wTimeLineText = TextWidget.Cast(GetRootWidget().FindAnyWidget("TimeLineText"));
		m_wProgressLine = ImageWidget.Cast(GetRootWidget().FindAnyWidget("ProgressLine"));
		m_wProgressButton = ButtonWidget.Cast(GetRootWidget().FindAnyWidget("ProgressButton"));
		m_wFilePickEditBox = EditBoxWidget.Cast(GetRootWidget().FindAnyWidget("FilePickEditBox"));
		
		SCR_ButtonBaseComponent button = SCR_ButtonBaseComponent.Cast(m_wProgressButton.FindHandler(SCR_ButtonBaseComponent));
		button.m_OnClicked.Insert(ProgressBarClick);
		
		m_iCurrentTime = 0;
		m_iCurrentReplayPosition = 0;
		if (m_MapEntity)
		{	
			OpenMap();
		}
		
		
	}
	void OpenMap()
	{
		GetGame().GetCallqueue().CallLater(OpenMapWrap, 0); // Need two frames
	}
	void OpenMapWrap()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_MapConfigComponent configComp = SCR_MapConfigComponent.Cast(gameMode.FindComponent(SCR_MapConfigComponent));
		if (!configComp)
			return;
		
		MapConfiguration mapConfigFullscreen = m_MapEntity.SetupMapConfig(EMapEntityMode.FULLSCREEN, configComp.GetGadgetMapConfig(), GetRootWidget());
		m_MapEntity.OpenMap(mapConfigFullscreen);
		GetGame().GetCallqueue().CallLater(OpenMapWrapZoomChange, 0);
	}
	void OpenMapWrapZoomChange()
	{
		// What i do with my life...
		GetGame().GetCallqueue().CallLater(OpenMapWrapZoomChangeWrap, 0); // Another two frames
	}
	void OpenMapWrapZoomChangeWrap()
	{
		m_MapEntity.ZoomOut();
	}
	
	override void OnMenuClose()
	{		
		if (m_MapEntity)
			m_MapEntity.CloseMap();
	}
	
	override void OnMenuInit()
	{
		if (!m_MapEntity)
			m_MapEntity = SCR_MapEntity.GetMapInstance();
	}
	
	void Action_Slower()
	{
		if (m_fSpeedScale <= 1) {
			m_fSpeedScale = 0;
			m_wSpeedText.SetText("x" + m_fSpeedScale.ToString());
			return;
		}
		m_fSpeedScale = m_fSpeedScale / 2;
		m_wSpeedText.SetText("x" + m_fSpeedScale.ToString());
	}
	
	void Action_Faster()
	{
		if (m_fSpeedScale <= 0) {
			m_fSpeedScale = 1;
			m_wSpeedText.SetText("x" + m_fSpeedScale.ToString());
			return;
		}
		if (m_fSpeedScale >= 64) return;
		m_fSpeedScale = m_fSpeedScale * 2;
		m_wSpeedText.SetText("x" + m_fSpeedScale.ToString());
	}
	
	void Action_ToGame()
	{
		int gameStartTime = replayReader.GetFirstPossessTime();
		Print(m_iCurrentTime.ToString() + "/" + gameStartTime.ToString());
		if (m_iCurrentTime > gameStartTime) return;
		FastForwardTo(gameStartTime);
	}
	
	void ProgressBarClick(SCR_ButtonBaseComponent button)
	{
		if (!replayReader) return;
		// mouse pos 
		int mousePosX, mousePosY;
		WidgetManager.GetMousePos(mousePosX, mousePosY);
		mousePosX = GetGame().GetWorkspace().GetWidth() - mousePosX;
		float progressSizeX = FrameSlot.GetSizeX(m_wFrameTimeLine);
		mousePosX = progressSizeX - GetGame().GetWorkspace().DPIUnscale(mousePosX);
		float progressPosition = mousePosX / progressSizeX;
		int time = replayReader.GetReplayTime() * progressPosition;
		FastForwardTo(time);
	}
	
	void FastForwardTo(int time)
	{
		m_iCurrentReplayPosition = 0;
		m_iCurrentTime = time;
		m_iLastTimeAwait = time;
		m_bFastForward = true;
		ClearMarkers();
	}
	
	void ClearMarkers()
	{
		Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
		
		for (int i = 0; i < m_hEntitiesMarkers.Count(); i++)
		{
			PS_MapMarkerReplayEntityComponent entity = m_hEntitiesMarkers.GetElement(i);
			mapFrame.RemoveChild(entity.GetRootWidget());
		}
		for (int i = 0; i < m_hExplosionMarkers.Count(); i++)
		{
			PS_ExplosionMarker explosion = m_hExplosionMarkers[i];
			mapFrame.RemoveChild(explosion.GetRootWidget());
		}
		for (int i = 0; i < m_hLinesMarkers.Count(); i++)
		{
			PS_MapLineComponent line = m_hLinesMarkers[i];
			mapFrame.RemoveChild(line.GetRootWidget());
		}
		m_hEntitiesMarkers.Clear();
		m_hExplosionMarkers.Clear();
		m_hLinesMarkers.Clear();
		m_hPlayers.Clear();
	}
	
	override void OnMenuUpdate(float tDelta)
	{
		string path = "$profile:Replays/" + m_wFilePickEditBox.GetText() + ".bin";
		path.Replace("\"", "");
		if (path != m_sOldPath)
		{
			m_sOldPath = path;
			if (FileIO.FileExists(m_sOldPath))
			{
				FastForwardTo(0);
				replayReader = new PS_ReplayReader();
				m_sReplayFileName = m_sOldPath;
				replayReader.ReadFile(m_sReplayFileName);
			}
		}
		if (!replayReader) return;
		
		tDelta *= m_fSpeedScale;
		
		// Update entities
		int markersCount = m_hEntitiesMarkers.Count();
		for (int i = 0; i < markersCount; i++)
		{
			PS_MapMarkerReplayEntityComponent handler = m_hEntitiesMarkers.GetElement(i);
			handler.Update();
		}
		// Update explosions
		for (int i = 0; i < m_hExplosionMarkers.Count(); i++)
		{
			PS_ExplosionMarker explosion = m_hExplosionMarkers[i];
			explosion.Update(tDelta);
			if (explosion.IsInvisible())
			{
				Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
				mapFrame.RemoveChild(explosion.GetRootWidget());
				m_hExplosionMarkers.Remove(i);
				i--;
			}
		}
		// Update lines
		for (int i = 0; i < m_hLinesMarkers.Count(); i++)
		{
			PS_MapLineComponent line = m_hLinesMarkers[i];
			line.Update(tDelta);
			if (line.IsInvisible())
			{
				Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
				mapFrame.RemoveChild(line.GetRootWidget());
				m_hLinesMarkers.Remove(i);
				i--;
			}
		}
		
		m_iCurrentTime += tDelta * 1000;
		
		int currentTimeClamp = m_iCurrentTime;
		int replayTime = replayReader.GetReplayTime();
		if (currentTimeClamp > replayTime) currentTimeClamp = replayTime;
		
		float replayTime24h = ((float) replayTime) / 1000 / 60 / 60;
		float currentTimeClamp24h = ((float) currentTimeClamp) / 1000 / 60 / 60;
		int h, m, s;
		TimeAndWeatherManagerEntity.TimeToHoursMinutesSeconds(replayTime24h, h, m, s);
		string replayTimeString = h.ToString(2) + ":" + m.ToString(2) + ":" + s.ToString(2);
		TimeAndWeatherManagerEntity.TimeToHoursMinutesSeconds(currentTimeClamp24h, h, m, s);
		string currentTimeString = h.ToString(2) + ":" + m.ToString(2) + ":" + s.ToString(2);
		m_wTimeLineText.SetText(currentTimeString + "/" + replayTimeString);
		
		float timeProgress = ((float) currentTimeClamp) / ((float) replayTime);
		
		
		float progressY = GetGame().GetWorkspace().DPIUnscale(28);
		float progressX = GetGame().GetWorkspace().DPIUnscale(1020.0 * timeProgress);
		FrameSlot.SetSize(m_wProgressLine, 1020.0 * timeProgress, 28);
		
		while (m_iCurrentTime > m_iLastTimeAwait) 
		{
			m_iCurrentReplayPosition++;
			if (replayReader.m_aStates.Count() < m_iCurrentReplayPosition) break;
			ReplayState state = replayReader.m_aStates[m_iCurrentReplayPosition - 1];
			switch (state.type)
			{
				case PS_EReplayType.WorldTime:
					ReplayWorldTime worldTime = ReplayWorldTime.Cast(state);
					if (m_iLastTimeAwait < worldTime.WorldTime)
						m_iLastTimeAwait = worldTime.WorldTime;
					break;
				case PS_EReplayType.EntityMove:
					EntityMove entityMove = EntityMove.Cast(state);
					if (m_hEntitiesMarkers.Contains(entityMove.EntityId))
					{
						PS_MapMarkerReplayEntityComponent handler = m_hEntitiesMarkers[entityMove.EntityId];
						handler.Move(entityMove.PositionX, entityMove.PositionZ, entityMove.RotationY);
					}
					break;
				case PS_EReplayType.CharacterPossess:
					CharacterPossess characterPossess = CharacterPossess.Cast(state);
					if (m_hEntitiesMarkers.Contains(characterPossess.EntityId))
					{
						if (m_hPlayers.Contains(characterPossess.PlayerId))
						{
							PS_MapMarkerReplayCharacterComponent handler = PS_MapMarkerReplayCharacterComponent.Cast(m_hEntitiesMarkers[characterPossess.EntityId]);
							PS_ReplayPlayer player = m_hPlayers[characterPossess.PlayerId];
							handler.Posses(player);
						}
					}
					break;
				case PS_EReplayType.CharacterRegistration:
					CharacterRegistration characterRegistration = CharacterRegistration.Cast(state);
					if (!m_hEntitiesMarkers.Contains(characterRegistration.EntityId))
					{
						Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
						Widget characterWidget = Widget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sMarkerPrefab, mapFrame));
						PS_MapMarkerReplayCharacterComponent characterHandler = PS_MapMarkerReplayCharacterComponent.Cast(characterWidget.FindHandler(PS_MapMarkerReplayCharacterComponent));
						characterHandler.Init(characterRegistration.EntityId, characterRegistration.EntityFactionKey);
						m_hEntitiesMarkers[characterRegistration.EntityId] = characterHandler;
					}
					break;
				case PS_EReplayType.PlayerRegistration:
					PlayerRegistration playerRegistration = PlayerRegistration.Cast(state);
					if (!m_hPlayers.Contains(playerRegistration.PlayerId))
					{
						PS_ReplayPlayer player = PS_ReplayPlayer.Cast(GetGame().SpawnEntity(PS_ReplayPlayer));
						player.m_iPlayerId = playerRegistration.PlayerId;
						player.m_sPlayerName = playerRegistration.PlayerName;
						m_hPlayers[playerRegistration.PlayerId] = player;
					}
					break;
				case PS_EReplayType.EntityDamageStateChanged:
					EntityDamageStateChanged entityDamageStateChanged = EntityDamageStateChanged.Cast(state);
					if (m_hEntitiesMarkers.Contains(entityDamageStateChanged.EntityId))
					{
						PS_MapMarkerReplayEntityComponent handler = m_hEntitiesMarkers[entityDamageStateChanged.EntityId];
						handler.DamageState(entityDamageStateChanged.DamageState);
					}
					break;
				case PS_EReplayType.VehicleRegistration:
					VehicleRegistration vehicleRegistration = VehicleRegistration.Cast(state);
					if (!m_hEntitiesMarkers.Contains(vehicleRegistration.EntityId))
					{
						Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
						Widget characterWidget = Widget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sMarkerVehiclePrefab, mapFrame));
						PS_MapMarkerReplayVehicleComponent vehicleHandler = PS_MapMarkerReplayVehicleComponent.Cast(characterWidget.FindHandler(PS_MapMarkerReplayVehicleComponent));
						
						vehicleHandler.SetVehicleType(vehicleRegistration.VehicleType);
						vehicleHandler.Init(vehicleRegistration.EntityId, vehicleRegistration.EntityFactionKey);
					
						m_hEntitiesMarkers[vehicleRegistration.EntityId] = vehicleHandler;
					}
					break;
				case PS_EReplayType.ProjectileShoot:
					ProjectileShoot projectileShoot = ProjectileShoot.Cast(state);
					if (m_hEntitiesMarkers.Contains(projectileShoot.EntityId) && !m_bFastForward)
					{
						PS_MapMarkerReplayEntityComponent entityHandler = m_hEntitiesMarkers[projectileShoot.EntityId];
						
						float posX, posZ, rotation;
						entityHandler.GetWorldPositionAndRotation(posX, posZ, rotation);
					
						Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
						Widget lineWidget = Widget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sLinePrefab, mapFrame));
						PS_MapLineComponent lineHandler = PS_MapLineComponent.Cast(lineWidget.FindHandler(PS_MapLineComponent));
						lineHandler.SetPosition(posX, posZ, projectileShoot.PositionX, projectileShoot.PositionZ);
						m_hLinesMarkers.Insert(lineHandler);
					}
					break;
				case PS_EReplayType.Explosion:
					Explosion explosion = Explosion.Cast(state);
					if (!m_bFastForward) {
						Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
						Widget explosionWidget = Widget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sExplosionPrefab, mapFrame));
						PS_ExplosionMarker explosionHandler = PS_ExplosionMarker.Cast(explosionWidget.FindHandler(PS_ExplosionMarker));
						explosionHandler.SetImpule(explosion.PositionX, explosion.PositionZ, explosion.ImpulseDistance);
						m_hExplosionMarkers.Insert(explosionHandler);
					}
					break;
				case PS_EReplayType.EntityDelete:
					EntityDelete entityDelete = EntityDelete.Cast(state);
					if (m_hEntitiesMarkers.Contains(entityDelete.EntityId))
					{
						PS_MapMarkerReplayEntityComponent entityHandler = m_hEntitiesMarkers[entityDelete.EntityId];
						Widget entitieWidget = entityHandler.GetRootWidget();
						m_hEntitiesMarkers.Remove(entityDelete.EntityId);
						Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
						mapFrame.RemoveChild(entitieWidget);
					}
					break;
			}
		}
		m_bFastForward = false;
				
		/*
		FileHandle replayFile = FileIO.OpenFile(m_sReplayFileName, FileMode.READ);
		if (replayFile.GetLength() <= m_iCurrentReplayPosition) return;
		replayFile.Seek(m_iCurrentReplayPosition);
		
		int i = 0;
		while (m_iCurrentTime > m_iLastTimeAwait) 
		{
			i = i + 1;
			if (i >= 30000) return;
			PS_EReplayType replayType;
			if (replayFile.GetLength() <= m_iCurrentReplayPosition) return;
			replayFile.Read(replayType, 1);
			m_iCurrentReplayPosition += 1;
			switch(replayType)
			{
				case PS_EReplayType.WorldTime:
					replayFile.Read(m_iLastTimeAwait, 4);
					m_iCurrentReplayPosition += 4;
					break;
				case PS_EReplayType.EntityMove:
					RplId characterId;
					float positionX;
					float positionZ;
					float RotationY;
					
					replayFile.Read(characterId, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(positionX, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(positionZ, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(RotationY, 4);
					m_iCurrentReplayPosition += 4;
					
					if (m_hEntitiesMarkers.Contains(characterId))
					{
						PS_MapMarkerReplayEntityComponent handler = m_hEntitiesMarkers[characterId];
						handler.Move(positionX, positionZ, RotationY);
					}
					break;
				case PS_EReplayType.CharacterPossess:
					RplId characterId;
					int playerId;
					
					
					replayFile.Read(characterId, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(playerId, 4);
					m_iCurrentReplayPosition += 4;
					
					if (m_hEntitiesMarkers.Contains(characterId))
					{
						if (m_hPlayers.Contains(playerId))
						{
							PS_MapMarkerReplayCharacterComponent handler = PS_MapMarkerReplayCharacterComponent.Cast(m_hEntitiesMarkers[characterId]);
							PS_ReplayPlayer player = m_hPlayers[playerId];
							handler.Posses(player);
						}
					}
					break;
				case PS_EReplayType.CharacterRegistration:
					RplId characterId;
					int factionKeyLength;
					FactionKey factionKey;
					
					replayFile.Read(characterId, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(factionKeyLength, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(factionKey, factionKeyLength);
					m_iCurrentReplayPosition += factionKeyLength;
					
					if (!m_hEntitiesMarkers.Contains(characterId))
					{
						Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
						Widget characterWidget = Widget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sMarkerPrefab, mapFrame));
						PS_MapMarkerReplayCharacterComponent characterHandler = PS_MapMarkerReplayCharacterComponent.Cast(characterWidget.FindHandler(PS_MapMarkerReplayCharacterComponent));
						characterHandler.Init(characterId, factionKey);
						m_hEntitiesMarkers[characterId] = characterHandler;
					}
					break;
				case PS_EReplayType.PlayerRegistration:
					int playerId;
					int playerNameLength;
					string playerName;
					
					replayFile.Read(playerId, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(playerNameLength, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(playerName, playerNameLength);
					m_iCurrentReplayPosition += playerNameLength;
					
					if (!m_hPlayers.Contains(playerId))
					{
						PS_ReplayPlayer player = PS_ReplayPlayer.Cast(GetGame().SpawnEntity(PS_ReplayPlayer));
						player.m_iPlayerId = playerId;
						player.m_sPlayerName = playerName;
						m_hPlayers[playerId] = player;
					}
					break;
				case PS_EReplayType.EntityDamageStateChanged:
					RplId characterId;
					EDamageState state;
					
					replayFile.Read(characterId, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(state, 4);
					m_iCurrentReplayPosition += 4;
					
					if (m_hEntitiesMarkers.Contains(characterId))
					{
						PS_MapMarkerReplayEntityComponent handler = m_hEntitiesMarkers[characterId];
						handler.DamageState(state);
					}
					break;
				case PS_EReplayType.VehicleRegistration:
					RplId vehicleId;
					int vehicleNameLength;
					string vehicleName;
					EVehicleType vehicleType;
					int factionKeyLength;
					FactionKey factionKey;
				
					replayFile.Read(vehicleId, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(vehicleNameLength, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(vehicleName, vehicleNameLength);
					m_iCurrentReplayPosition += vehicleNameLength;
					replayFile.Read(vehicleType, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(factionKeyLength, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(factionKey, factionKeyLength);
					m_iCurrentReplayPosition += factionKeyLength;
					
					if (!m_hEntitiesMarkers.Contains(vehicleId))
					{
						Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
						Widget characterWidget = Widget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sMarkerVehiclePrefab, mapFrame));
						PS_MapMarkerReplayVehicleComponent vehicleHandler = PS_MapMarkerReplayVehicleComponent.Cast(characterWidget.FindHandler(PS_MapMarkerReplayVehicleComponent));
						
						vehicleHandler.SetVehicleType(vehicleType);
						vehicleHandler.Init(vehicleId, factionKey);
					
						m_hEntitiesMarkers[vehicleId] = vehicleHandler;
					}
				
					break;
				case PS_EReplayType.ProjectileShoot:
					RplId entityId;
					float positionx;
					float positionz;
				
					replayFile.Read(entityId, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(positionx, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(positionz, 4);
					m_iCurrentReplayPosition += 4;
				
					if (m_hEntitiesMarkers.Contains(entityId))
					{
						PS_MapMarkerReplayEntityComponent entityHandler = m_hEntitiesMarkers[entityId];
						
						float posX, posZ, rotation;
						entityHandler.GetWorldPositionAndRotation(posX, posZ, rotation);
					
						Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
						Widget lineWidget = Widget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sLinePrefab, mapFrame));
						PS_MapLineComponent lineHandler = PS_MapLineComponent.Cast(lineWidget.FindHandler(PS_MapLineComponent));
						lineHandler.SetPosition(posX, posZ, positionx, positionz);
						m_hLinesMarkers.Insert(lineHandler);
					}
					break;
				case PS_EReplayType.Explosion:
					float positionx;
					float positionz;
					float impulse;
				
					replayFile.Read(positionx, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(positionz, 4);
					m_iCurrentReplayPosition += 4;
					replayFile.Read(impulse, 4);
					m_iCurrentReplayPosition += 4;
				
					Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
					Widget explosionWidget = Widget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sExplosionPrefab, mapFrame));
					PS_ExplosionMarker explosionHandler = PS_ExplosionMarker.Cast(explosionWidget.FindHandler(PS_ExplosionMarker));
					explosionHandler.SetImpule(positionx, positionz, impulse);
					m_hExplosionMarkers.Insert(explosionHandler);
					break;
				case PS_EReplayType.EntityDelete:
					RplId entityId;
				
					replayFile.Read(entityId, 4);
					m_iCurrentReplayPosition += 4;
					
					if (m_hEntitiesMarkers.Contains(entityId))
					{
						PS_MapMarkerReplayEntityComponent entityHandler = m_hEntitiesMarkers[entityId];
						Widget entitieWidget = entityHandler.GetRootWidget();
						m_hEntitiesMarkers.Remove(entityId);
						Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
						mapFrame.RemoveChild(entitieWidget);
					}
					break;
			}
		}
		*/
	}
};


























