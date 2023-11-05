class PS_ReplayReader
{
	ref array<ref ReplayState> m_aStates;
	int m_iFilePosition = 0;
	string m_sFilePath;
	int m_iReplayTime = 1;
	bool m_bLoadEnded = false;
	
	int GetReplayTime()
	{
		return m_iReplayTime;
	}
	bool IsLoadEnded()
	{
		return m_bLoadEnded;
	}
	
	void ReadFile(string path)
	{
		m_sFilePath = path;
		m_bLoadEnded = false;
		m_aStates = new array<ref ReplayState>();
		GetGame().GetCallqueue().CallLater(ReadFileTimed);
	}
	
	void ReadFileTimed()
	{
		int maxRead = 30000;
		int readStartPosition = m_iFilePosition;
		FileHandle replayFile = FileIO.OpenFile(m_sFilePath, FileMode.READ);
		replayFile.Seek(readStartPosition);
		while (m_iFilePosition - readStartPosition < maxRead) 
		{
			if (replayFile.IsEOF()) {
				LoadEnd();
				return;
			}
			PS_EReplayType replayType;
			replayFile.Read(replayType, 1);
			m_iFilePosition += 1;
			switch(replayType)
			{
				case PS_EReplayType.WorldTime:
					int worldTime;
					replayFile.Read(worldTime, 4);
					m_iFilePosition += 4;
					
					m_iReplayTime = worldTime;
					m_aStates.Insert(new ReplayWorldTime(worldTime));
					
					break;
				case PS_EReplayType.EntityMove:
					RplId characterId;
					float positionX;
					float positionZ;
					float rotationY;
					replayFile.Read(characterId, 4);
					replayFile.Read(positionX, 4);
					replayFile.Read(positionZ, 4);
					replayFile.Read(rotationY, 4);
					m_iFilePosition += 16;
				
					m_aStates.Insert(new EntityMove(characterId, positionX, positionZ, rotationY));
					
					break;
				case PS_EReplayType.CharacterPossess:
					RplId characterId;
					int playerId;
					replayFile.Read(characterId, 4);
					replayFile.Read(playerId, 4);
					m_iFilePosition += 8;
					
					m_aStates.Insert(new CharacterPossess(characterId, playerId));
					
					break;
				case PS_EReplayType.CharacterRegistration:
					RplId characterId;
					int factionKeyLength;
					FactionKey factionKey;
					replayFile.Read(characterId, 4);
					replayFile.Read(factionKeyLength, 4);
					replayFile.Read(factionKey, factionKeyLength);
					m_iFilePosition += 8 + factionKeyLength;
					
					m_aStates.Insert(new CharacterRegistration(characterId, factionKey));
					
					break;
				case PS_EReplayType.VehicleRegistration:
					RplId vehicleId;
					int vehicleNameLength;
					string vehicleName;
					EVehicleType vehicleType;
					int factionKeyLength;
					FactionKey factionKey;
					replayFile.Read(vehicleId, 4);
					replayFile.Read(vehicleNameLength, 4);
					replayFile.Read(vehicleName, vehicleNameLength);
					replayFile.Read(vehicleType, 4);
					replayFile.Read(factionKeyLength, 4);
					replayFile.Read(factionKey, factionKeyLength);
					m_iFilePosition += 16 + vehicleNameLength + factionKeyLength;
					
					m_aStates.Insert(new VehicleRegistration(vehicleId, vehicleName, vehicleType, factionKey));
					
					break;
				case PS_EReplayType.PlayerRegistration:
					int playerId;
					int playerNameLength;
					string playerName;
					replayFile.Read(playerId, 4);
					replayFile.Read(playerNameLength, 4);
					replayFile.Read(playerName, playerNameLength);
					m_iFilePosition += 8 + playerNameLength;
					
					m_aStates.Insert(new PlayerRegistration(playerId, playerName));
					
					break;
				case PS_EReplayType.EntityDamageStateChanged:
					RplId entityId;
					EDamageState state;
					replayFile.Read(entityId, 4);
					replayFile.Read(state, 4);
					m_iFilePosition += 8;
					
					m_aStates.Insert(new EntityDamageStateChanged(entityId, state));
					
					break;
				case PS_EReplayType.ProjectileShoot:
					RplId entityId;
					float positionx;
					float positionz;
					replayFile.Read(entityId, 4);
					replayFile.Read(positionx, 4);
					replayFile.Read(positionz, 4);
					m_iFilePosition += 12;
					
					m_aStates.Insert(new ProjectileShoot(entityId, positionx, positionz));
					
					break;
				case PS_EReplayType.Explosion:
					float positionx;
					float positionz;
					float impulse;
					replayFile.Read(positionx, 4);
					replayFile.Read(positionz, 4);
					replayFile.Read(impulse, 4);
					m_iFilePosition += 12;
					
					m_aStates.Insert(new Explosion(positionx, positionz, impulse));
					
					break;
				case PS_EReplayType.EntityDelete:
					RplId entityId;
					replayFile.Read(entityId, 4);
					m_iFilePosition += 4;
					
					m_aStates.Insert(new EntityDelete(entityId));
					
					break;
			}
		}
		
		if (m_iFilePosition < replayFile.GetLength()) GetGame().GetCallqueue().CallLater(ReadFileTimed);
		else LoadEnd();
	}
	
	void LoadEnd()
	{
		m_bLoadEnded = true;
	}
}