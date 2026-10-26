#include "InputManager.h"

namespace InputSystem
{

void InputManager::Update(uint32_t buttons)
{
	m_previous = m_current;
	m_current = buttons;
}

bool InputManager::Pressed(int inputMapType) const
{
	return (m_current & (uint32_t) inputMapType) != 0;
}

bool InputManager::JustPressed(int inputMapType) const
{
	uint32_t bit = (uint32_t) inputMapType;
	return (m_current & bit) != 0 && (m_previous & bit) == 0;
}

bool InputManager::Released(int inputMapType) const
{
	uint32_t bit = (uint32_t) inputMapType;
	return (m_current & bit) == 0 && (m_previous & bit) != 0;
}

} // namespace InputSystem
