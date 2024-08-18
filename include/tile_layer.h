/**************************************************************************************************
 * 
 * Copyright 2022 https://github.com/fe-dagostino
 * 
 * This program is free software: you can redistribute it and/or modify it under the terms 
 * of the GNU Affero General Public License as published by the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with this program. 
 * If not, see <https://www.gnu.org/licenses/>
 *
 *************************************************************************************************/
#ifndef TILE_LAYER_H
#define TILE_LAYER_H

#include <widgets/ure_layer.h>

class TileLayer : public ure::widgets::Layer
{
public:
  /***/
  TileLayer( ure::ViewPort& rViewPort, const ure::Size& tile_size, ure::word_t zoom, const std::string& url ) noexcept;

  /***/
  constexpr ure::word_t  zoom() const
  { return m_zoom_level; }

protected:
  /***/
  virtual bool     on_draw( [[maybe_unused]] const ure::Recti& rect ) noexcept override;

  /***/
  virtual ure::void_t  on_mouse_move( [[maybe_unused]] ure::Window* pWindow, ure::double_t dPosX, ure::double_t dPosY ) noexcept override
  {
    printf( "Tile Layer: x:%f y:%f\n", dPosX, dPosY );
  }
  

private:
  const ure::Size   m_tile_size;         /* Size of single tile */
  const ure::int_t  m_zoom_level;
  const ure::word_t m_max_tiles;
  const ure::Size   m_tile_area;         /* Size of the full area covered by all tiles */ 
  const std::string m_url;

};

#endif // TILE_LAYER_H
