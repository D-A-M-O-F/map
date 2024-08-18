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

#ifndef MAP_H
#define MAP_H

#include <ure_application.h>
#include <ure_resources_fetcher.h>
#include <ure_window.h>
#include <ure_view_port.h>
#include <ure_position.h>
#include <ure_size.h>


class Map : public ure::ApplicationEvents, public ure::WindowEvents, public ure::ResourcesFetcherEvents
{
public:
  /***/
  Map( int argc, char** argv );

  /***/
  void run();

  /***/
  void dispose();

  /***/
  constexpr ure::int_t max_levels() const
  { return m_maxLevels; }
  

protected:
  /***/
  void init( [[__maybe_unused__]] int argc, [[__maybe_unused__]] char** argv ) noexcept;
  /***/
  void load_resources() noexcept;
  /***/
  void add_camera() noexcept;
  /***/
  void add_zoom_levels( const std::string& url ) noexcept;

// ure::WindowEvents implementation
protected:
  /***/
  virtual ure::void_t  on_mouse_scroll( [[maybe_unused]] ure::Window* pWindow, [[maybe_unused]] ure::double_t dOffsetX, [[maybe_unused]] ure::double_t dOffsetY ) noexcept override;
  /***/
  virtual ure::void_t  on_mouse_move( [[maybe_unused]] ure::Window* pWindow, [[maybe_unused]] ure::double_t dPosX, [[maybe_unused]] ure::double_t dPosY ) noexcept override
  {
    printf( "x:%f y:%f\n", dPosX, dPosY );
  }
// ure::ApplicationEvents implementation  
protected:
  /***/
  virtual ure::void_t on_initialize() override;

  /***/
  virtual ure::void_t on_initialized() override;

  /***/
  virtual ure::void_t on_finalize() override;

  /***/
  virtual ure::void_t on_finalized() override;

  virtual ure::void_t on_run() override; 

  /***/
  virtual ure::void_t on_initialize_error(/* @todo */) override;
  /***/
  virtual ure::void_t on_error( [[maybe_unused]] int32_t error, [[maybe_unused]] const std::string& description ) override;
  /***/
  virtual ure::void_t on_finalize_error(/* @todo */) override;

// ure::ResourcesFetcherEvents implementation
protected:  
  /***/
  virtual ure::void_t on_download_succeeded( [[maybe_unused]] const std::string& name, [[maybe_unused]] const std::type_info& type, [[maybe_unused]] const ure::byte_t* data, [[maybe_unused]] ure::uint_t length ) override;
  /***/
  virtual ure::void_t on_download_failed   ( [[maybe_unused]] const std::string& name ) override;

private:
  bool                        m_bFullScreen;
  ure::position_t<ure::int_t> m_position;
  ure::Size                   m_size;
  ure::Size                   m_fb_size;    // Frame Buffer Size
  ure::Size                   m_tile_size;

  ure::Window*                m_pWindow;
  ure::ViewPort*              m_pViewPort;

  const ure::int_t            m_maxLevels;
  ure::int_t                  m_curLevel;
};

#endif // MAP_H
