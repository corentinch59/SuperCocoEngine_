#ifndef BULLETFORGE_HEALTHCOMPONENT_HPP
#define BULLETFORGE_HEALTHCOMPONENT_HPP

#pragma once

#include <vector>
#include <functional>

namespace BulletForge
{
	class HealthComponent
	{
	public:
		explicit HealthComponent(float initialHealth);

		float GetHealth() const;

		void ModifyHealth(float value);
		void SetOnDeathCallback(std::function<void()> callback);
	private:

		std::function<void()> m_onDeath;
		float m_maxHealth;
		float m_currentHealth;
	};

}

#endif