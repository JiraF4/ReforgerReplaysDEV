class PS_ExplosionMarker : SCR_ScriptedWidgetComponent
{
	protected SCR_MapEntity m_MapEntity;
	
	ImageWidget m_wMarkerIcon;
	
	float positionX;
	float positionZ;
	
	float m_fImpulseDistance;
	float m_lifeTime = 1.5;
	float m_lifeTimeMax = 1.5;
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		m_wMarkerIcon = ImageWidget.Cast(w.FindAnyWidget("MarkerIcon"));
		
		if (!GetGame().InPlayMode())
			return;
	}
	
	void SetImpule(float positionXNew, float positionZNew, float impulseDistanceNew)
	{
		positionX = positionXNew;
		positionZ = positionZNew;
		m_fImpulseDistance = impulseDistanceNew;
		
		Update(0.0);
	}
	
	void Update(float tDelta)
	{
		m_lifeTime = m_lifeTime - tDelta;
		
		float screenX, screenY, screenXEnd, screenYEnd;
		m_MapEntity.WorldToScreen(positionX, positionZ, screenX, screenY, true);
		m_MapEntity.WorldToScreen(positionX + m_fImpulseDistance, positionZ + m_fImpulseDistance, screenXEnd, screenYEnd, true);
		float screenXD = GetGame().GetWorkspace().DPIUnscale(screenX);
		float screenYD = GetGame().GetWorkspace().DPIUnscale(screenY);
		float sizeXD = GetGame().GetWorkspace().DPIUnscale(screenXEnd - screenX);
		float sizeYD = GetGame().GetWorkspace().DPIUnscale(screenY - screenYEnd); // Y flip
		FrameSlot.SetPos(m_wRoot, screenXD, screenYD);
		FrameSlot.SetPos(m_wMarkerIcon, -sizeXD/2, -sizeYD/2);
		FrameSlot.SetSize(m_wMarkerIcon, sizeXD, sizeYD);
		
		m_wRoot.SetOpacity(m_lifeTime / m_lifeTimeMax);
	}
	
	bool IsInvisible()
	{
		return m_lifeTime <= 0.0;
	}
}