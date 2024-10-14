
class PS_MapMarkerReplayEntityComponent : SCR_ScriptedWidgetComponent
{
	protected RplId m_iId;
	protected FactionKey m_iFactionKey;
	
	protected SCR_MapEntity m_MapEntity;
	protected ImageWidget m_wMarkerIcon;
	protected TextWidget m_wEntityNameText;
	
	protected int m_iMoveTime;
	protected int m_iMoveDelta;
	
	float positionXOld;
	float positionZOld;
	float rotationYOld;
	float positionX;
	float positionZ;
	float rotationY;
	
	float m_fEntitySize = 4.0;
	float m_fMinMarkerSize = 24.0;
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		m_wMarkerIcon = ImageWidget.Cast(w.FindAnyWidget("MarkerIcon"));
		m_wEntityNameText = TextWidget.Cast(w.FindAnyWidget("EntityNameText"));
		
		if (!GetGame().InPlayMode())
			return;
		
		m_wEntityNameText.SetVisible(false);
	}
	
	void Update()
	{
		float posX, posZ, rotation;
		GetWorldPositionAndRotation(posX, posZ, rotation);
		
		float screenX, screenY, screenXEnd, screenYEnd;
		m_MapEntity.WorldToScreen(posX, posZ, screenX, screenY, true);
		m_MapEntity.WorldToScreen(posX + m_fEntitySize, posZ + m_fEntitySize, screenXEnd, screenYEnd, true);
		float screenXD = GetGame().GetWorkspace().DPIUnscale(screenX);
		float screenYD = GetGame().GetWorkspace().DPIUnscale(screenY);
		float sizeXD = GetGame().GetWorkspace().DPIUnscale(screenXEnd - screenX);
		float sizeYD = GetGame().GetWorkspace().DPIUnscale(screenY - screenYEnd); // Y flip
		
		if (sizeXD < m_fMinMarkerSize) sizeXD = m_fMinMarkerSize;
		if (sizeYD < m_fMinMarkerSize) sizeYD = m_fMinMarkerSize;
		
		FrameSlot.SetPos(m_wRoot, screenXD, screenYD);
		FrameSlot.SetPos(m_wEntityNameText, -sizeXD/2, -sizeYD);
		FrameSlot.SetSize(m_wEntityNameText, sizeXD, sizeYD/2);
		FrameSlot.SetPos(m_wMarkerIcon, -sizeXD/2, -sizeYD/2);
		FrameSlot.SetSize(m_wMarkerIcon, sizeXD, sizeYD);
		m_wMarkerIcon.SetRotation(rotation);
	}
	
	void GetWorldPositionAndRotation(out float posX, out float posZ, out float rotation)
	{
		int worldTime = GetGame().GetWorld().GetWorldTime();
		int worldTimeDelta = worldTime - m_iMoveTime;
		float lerpWeight = ((float)worldTimeDelta) / ((float)m_iMoveDelta + 1.0);
		if (lerpWeight > 1.0) lerpWeight = 1.0;
		posX = positionX * lerpWeight + positionXOld * (1.0 - lerpWeight);
		posZ = positionZ * lerpWeight + positionZOld * (1.0 - lerpWeight);
		
		float deltaAngle = rotationY - rotationYOld;
		if (deltaAngle >  180) deltaAngle = deltaAngle - 360;
		if (deltaAngle < -180) deltaAngle = deltaAngle + 360;
		rotation = rotationYOld + deltaAngle * lerpWeight;
	}
	
	void Init(RplId id, FactionKey factionKey)
	{
		m_iId = id;
		m_iFactionKey = factionKey;
		m_iMoveTime = GetGame().GetWorld().GetWorldTime();
		
		FactionManager factionManager = GetGame().GetFactionManager();
		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByKey(factionKey));
		if (faction) {
			m_wMarkerIcon.SetColor(faction.GetOutlineFactionColor());
			m_wEntityNameText.SetColor(faction.GetOutlineFactionColor());
		}
	}
	
	void Move(float positionXnew, float positionZnew, float rotationYnew)
	{
		positionXOld = positionX;
		positionZOld = positionZ;
		rotationYOld = rotationY;
		positionX = positionXnew;
		positionZ = positionZnew;
		rotationY = rotationYnew;
		
		int worldTime = GetGame().GetWorld().GetWorldTime();
		m_iMoveDelta = worldTime - m_iMoveTime;
		m_iMoveTime = worldTime;
	}
	
	void DamageState(EDamageState state)
	{
		if (state == EDamageState.DESTROYED)
		{
			m_wMarkerIcon.LoadImageFromSet(0, "{ED7A1DA5BC4CCBA0}UI/Icons/Replay_Atlas.imageset", "DEAD");
			m_wRoot.SetOpacity(0.3);
		}
	}
	
	void Posses(PS_ReplayPlayer player)
	{
		m_wEntityNameText.SetVisible(true);
		m_wEntityNameText.SetText(player.m_sPlayerName);
	}
}