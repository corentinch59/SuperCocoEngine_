#ifndef SUPERCOCO_TEXT_HPP
#define SUPERCOCO_TEXT_HPP

#pragma once

#include <SuperCoco/Export.hpp>
#include <SuperCoco/Renderable.hpp>
#include <SDL2/SDL.h>
#include <SuperCoco/Vector2.hpp>
#include <memory>
#include <string>

namespace Sce
{
	class Texture;
	class Font;

	class SUPER_COCO_API Text : public IRenderable
	{
	public :
		Text();
		Text(std::shared_ptr<Font> font, std::string txt, const SDL_Color& color, Vector2f origin);

		virtual void Render(Renderer& renderer, const Matrixf& transformMatrix) const override;

	private:
		std::unique_ptr<Texture> m_texture;
		std::shared_ptr<Font> m_font;
		SDL_Color m_color;
		std::string m_text;
		Vector2f m_origin;
	};
}

#endif