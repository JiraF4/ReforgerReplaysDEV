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
	
	int m_iCurrentReplayPosition;
	int m_iCurrentTime;
	int m_iLastTimeAwait;
	string m_sReplayFileName;
	FileHandle m_fReplayFile;
	ref map<RplId, PS_MapMarkerReplayEntityComponent> m_hEntitiesMarkers = new map<RplId, PS_MapMarkerReplayEntityComponent>();
	ref array<PS_MapLineComponent> m_hLinesMarkers = new array<PS_MapLineComponent>();
	ref array<PS_ExplosionMarker> m_hExplosionMarkers = new array<PS_ExplosionMarker>();
	ref map<int, PS_ReplayPlayer> m_hPlayers = new map<int, PS_ReplayPlayer>();
	
	override void OnMenuOpen()
	{	
		m_sReplayFileName = "$profile:Replays/ReplayRead.bin";
		m_iCurrentTime = 0;
		m_iCurrentReplayPosition = 0;
		if (m_MapEntity)
		{	
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (!gameMode)
				return;
		
			SCR_MapConfigComponent configComp = SCR_MapConfigComponent.Cast(gameMode.FindComponent(SCR_MapConfigComponent));
			if (!configComp)
				return;
		
			MapConfiguration mapConfigFullscreen = m_MapEntity.SetupMapConfig(EMapEntityMode.FULLSCREEN, configComp.GetGadgetMapConfig(), GetRootWidget());
			m_MapEntity.OpenMap(mapConfigFullscreen);
		}
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
	
	override void OnMenuUpdate(float tDelta)
	{
		int markersCount = m_hEntitiesMarkers.Count();
		for (int i = 0; i < markersCount; i++)
		{
			PS_MapMarkerReplayEntityComponent handler = m_hEntitiesMarkers.GetElement(i);
			handler.Update();
		}
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
		
		
		FileHandle replayFile = FileIO.OpenFile(m_sReplayFileName, FileMode.READ);
		if (replayFile.GetLength() <= m_iCurrentReplayPosition) return;
		replayFile.Seek(m_iCurrentReplayPosition);
		
		m_iCurrentTime += tDelta * 1000;
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
	}
};


























