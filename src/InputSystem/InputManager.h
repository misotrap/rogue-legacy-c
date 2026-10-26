/* Port of the InputSystem project's InputManager (subset): Game.GlobalInput
 * in C#. One gamepad, polled from the platform layer each frame.
 */
#ifndef INPUT_SYSTEM_INPUT_MANAGER_H
#define INPUT_SYSTEM_INPUT_MANAGER_H

#include <cstdint>

namespace InputSystem
{

class InputManager
{
public:
	/* Called once per frame with the platform button state. */
	void Update(uint32_t buttons);

	bool Pressed(int inputMapType) const;      /* held this frame */
	bool JustPressed(int inputMapType) const;  /* pressed since last frame */
	bool Released(int inputMapType) const;

private:
	uint32_t m_current = 0;
	uint32_t m_previous = 0;
};

} // namespace InputSystem

#endif
