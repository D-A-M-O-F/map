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

#include "tile_layer.h"

#include "ure_resources_collector.h"
#include "ure_resources_fetcher.h"

#include <core/utils.h>
  
TileLayer::TileLayer( ure::ViewPort& rViewPort, const ure::Size& tile_size, ure::word_t zoom, const std::string& url ) noexcept
  : ure::widgets::Layer( rViewPort ), m_tile_size(tile_size), 
    m_zoom_level( zoom ), m_max_tiles( 1 << zoom ),
    m_tile_area( m_tile_size.width*m_max_tiles, m_tile_size.height*m_max_tiles ),
    m_url(url)
{
  
}

bool     TileLayer::on_draw( [[maybe_unused]] const ure::Recti& rect ) noexcept 
{ 
  for ( ure::word_t y = 0; y < m_max_tiles; ++y )
  {
    for ( ure::word_t x = 0; x < m_max_tiles; ++x )
    {
      std::string name     = core::utils::format( "%u-%u-%u", m_zoom_level, x, y  );
      std::string resource = core::utils::format( m_url.c_str(), m_zoom_level, x,y );

      ure::Texture* texture = ure::ResourcesCollector::get_instance()->find<ure::Texture>(name);
      if ( texture == nullptr )
      {
        ure::ResourcesFetcher::get_instance()->fetch( name, typeid(ure::Texture), resource );
      }
      else
      {
        // Default Vertices coordinates 
        ure::Position   pos  = get_position();
        ure::Size       size = get_size();
        ure::double_t   xr   = 1.0f;
        ure::double_t   yr   = 1.0f;

        if (get_parent()!=nullptr)
        {
          pos += get_parent()->get_position();
        }

        if ( m_tile_area.width < size.width )
        {
           xr =  ure::double_t(size.width) / ure::double_t(m_tile_area.width);
        }

        if ( m_tile_area.height < size.height )
        {
           yr =  ure::double_t(size.height) / ure::double_t(m_tile_area.height);
        }

        /*--------------------------------*/
        std::vector<glm::vec2>  vertices;

        vertices.reserve(4);

        vertices.emplace( vertices.end(), glm::vec2( pos.x + xr * ((x * m_tile_size.width)                    ), pos.y + yr * ((y * m_tile_size.height) + m_tile_size.height )) );
        vertices.emplace( vertices.end(), glm::vec2( pos.x + xr * ((x * m_tile_size.width) + m_tile_size.width), pos.y + yr * ((y * m_tile_size.height) + m_tile_size.height )) );
        vertices.emplace( vertices.end(), glm::vec2( pos.x + xr * ((x * m_tile_size.width)                    ), pos.y + yr * ((y * m_tile_size.height)                      )) );
        vertices.emplace( vertices.end(), glm::vec2( pos.x + xr * ((x * m_tile_size.width) + m_tile_size.width), pos.y + yr * ((y * m_tile_size.height)                      )) );

        /*--------------------------------*/
        std::vector<glm::vec2>  texture_coordinates;

        texture_coordinates.reserve(4);

        texture_coordinates.emplace( texture_coordinates.end(), glm::vec2( 0.0f, 1.0f )  );
        texture_coordinates.emplace( texture_coordinates.end(), glm::vec2( 1.0f, 1.0f )  );
        texture_coordinates.emplace( texture_coordinates.end(), glm::vec2( 0.0f, 0.0f )  );
        texture_coordinates.emplace( texture_coordinates.end(), glm::vec2( 1.0f, 0.0f )  );

        /*--------------------------------*/

        draw_rect( vertices, texture_coordinates, *texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
      }
    }
  
  }

  return true; 
}
