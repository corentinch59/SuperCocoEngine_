#ifndef BULLETFORGE_DEATHSYSTEM_HPP
#define BULLETFORGE_DEATHSYSTEM_HPP

#pragma once

#include <entt/fwd.hpp>

namespace BulletForge
{
	class DeathSystem
	{
	public :
		DeathSystem(entt::registry* registry);

		void DeathNote();

	private:
		entt::registry* m_registry;
	};
}

#endif