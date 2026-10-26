/* Port of SpriteSystem/SpritesheetObj.cs: the data of one sprite (frame
 * images, animation frames, hitboxes) plus the CharacterData / Hitbox
 * structs the C# game code manipulates.
 */
#ifndef SPRITE_SYSTEM_SPRITESHEET_OBJ_H
#define SPRITE_SYSTEM_SPRITESHEET_OBJ_H

#include "../DS2DEngine/Types.h"
#include <string>
#include <vector>

namespace SpriteSystem
{

struct ImageData
{
	int Index;
	Vector2 SSPos;
	Vector2 Anchor;
	int Width;
	int Height;
};

struct Hitbox
{
	int X;
	int Y;
	int Width;
	int Height;
	float Rotation;
	int Type;
};

struct FrameData
{
	int Frame;          /* XML 'frame' attribute: a label, order animates */
	int Index;          /* image index shown by this frame */
	int Duration;      /* in animation ticks (see AnimationDelay) */
	std::string Label;
	std::vector<Hitbox> hitboxList;
};

struct CharacterData
{
	std::string Child;
	int ChildX;
	int ChildY;
	/* childRotation exists in the XML but the original engine ignores it
	 * (SpriteLibrary.cs reads only childX/childY). */
};

class SpritesheetObj
{
public:
	SpritesheetObj(const std::string &spriteName, const std::string &spritesheetName);

	void AddImageData(int index, Vector2 ssPos, Vector2 anchor, int width, int height);
	void AddFrameData(int frame, int index, int duration, const std::string &label);
	void AddHitboxToLastFrameData(int x, int y, int width, int height, float rotation, int type);
	void SortImageData();

	const std::vector<ImageData> &GetImageDataList() const { return m_imageDataList; }
	const std::vector<FrameData> &GetFrameDataList() const { return m_frameDataList; }
	const std::string &GetSpritesheetName() const { return m_spritesheetName; }
	const std::string &GetSpriteName() const { return m_spriteName; }

private:
	std::string m_spriteName;
	std::string m_spritesheetName;
	std::vector<ImageData> m_imageDataList;
	std::vector<FrameData> m_frameDataList;
};

} // namespace SpriteSystem

#endif
