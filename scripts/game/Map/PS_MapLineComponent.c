class PS_MapLineComponent : SCR_ScriptedWidgetComponent
{
	protected SCR_MapEntity m_MapEntity;
	
	ImageWidget m_wLineImage;
	
	float positionXStart;
	float positionZStart;
	float positionYEnd;
	float positionZEnd;
	
	float m_lifeTime = 0.5;
	float m_lifeTimeMax = 0.5;
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		m_wLineImage = ImageWidget.Cast(w.FindAnyWidget("LineImage"));
		
		if (!GetGame().InPlayMode())
			return;
	}
	
	void SetPosition(float positionXStartNew, float positionZStartNew, float positionYEndNew, float positionZEndNew)
	{
		positionXStart = positionXStartNew;
		positionZStart = positionZStartNew;
		positionYEnd = positionYEndNew;
		positionZEnd = positionZEndNew;
		Update(0.0);
	}
	
	void Update(float tDelta)
	{
		m_lifeTime = m_lifeTime - tDelta;
		
		vector lineVector = vector.Zero;
		lineVector[0] = positionXStart - positionYEnd;
		lineVector[1] = positionZStart - positionZEnd;
		float worldLen = lineVector.Length();
		
		float screenX, screenY, screenXEnd, screenYEnd;
		m_MapEntity.WorldToScreen(positionXStart, positionZStart, screenX, screenY, true);
		m_MapEntity.WorldToScreen(positionYEnd, positionZEnd, screenXEnd, screenYEnd, true);
		
		float screenXD = GetGame().GetWorkspace().DPIUnscale(screenX);
		float screenYD = GetGame().GetWorkspace().DPIUnscale(screenY);
		float sizeXD = GetGame().GetWorkspace().DPIUnscale(screenXEnd);
		float sizeYD = GetGame().GetWorkspace().DPIUnscale(screenYEnd);
		
		lineVector[0] = sizeXD - screenXD;
		lineVector[1] = sizeYD - screenYD;
		float len = lineVector.Length();
		lineVector = lineVector / len;
		float angle = Math.Atan2(lineVector[1], lineVector[0]);
		
		m_wLineImage.SetRotation(angle * Math.RAD2DEG);
		FrameSlot.SetPos(m_wRoot, screenXD, screenYD);
		FrameSlot.SetSize(m_wLineImage, len, 2 * (len/worldLen));
		m_wRoot.SetOpacity(m_lifeTime / m_lifeTimeMax);
	}
	
	bool IsInvisible()
	{
		return m_lifeTime <= 0.0;
	}
}