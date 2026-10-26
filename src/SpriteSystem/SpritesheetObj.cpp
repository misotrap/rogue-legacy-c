#include "SpritesheetObj.h"
#include <algorithm>

namespace SpriteSystem
{

SpritesheetObj::SpritesheetObj(const std::string &spriteName, const std::string &spritesheetName)
	: m_spriteName(spriteName), m_spritesheetName(spritesheetName)
{
}

void SpritesheetObj::AddImageData(int index, Vector2 ssPos, Vector2 anchor, int width, int height)
{
	ImageData id;
	id.Index = index;
	id.SSPos = ssPos;
	id.Anchor = anchor;
	id.Width = width;
	id.Height = height;
	m_imageDataList.push_back(id);
}

void SpritesheetObj::AddFrameData(int frame, int index, int duration, const std::string &label)
{
	FrameData fd;
	fd.Frame = frame;
	fd.Index = index;
	fd.Duration = duration;
	fd.Label = label;
	m_frameDataList.push_back(fd);
}

void SpritesheetObj::AddHitboxToLastFrameData(int x, int y, int width, int height, float rotation, int type)
{
	if (m_frameDataList.empty())
		return;
	Hitbox hb;
	hb.X = x;
	hb.Y = y;
	hb.Width = width;
	hb.Height = height;
	hb.Rotation = rotation;
	hb.Type = type;
	m_frameDataList.back().hitboxList.push_back(hb);
}

void SpritesheetObj::SortImageData()
{
	/* ImageDataComparator: sort by Index (SpritesheetObj.cs). */
	std::sort(m_imageDataList.begin(), m_imageDataList.end(),
		[](const ImageData &a, const ImageData &b) { return a.Index < b.Index; });
}

} // namespace SpriteSystem
