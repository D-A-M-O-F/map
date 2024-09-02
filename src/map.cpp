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

#include "map.h"
#include "tile_layer.h"

#include <ure_utils.h>
#include <ure_image.h>
#include <ure_texture.h>
#include <ure_resources_collector.h>
#include <ure_scene_graph.h>
#include <ure_scene_layer_node.h>


#include <core/utils.h>
  
Map::Map( int argc, char** argv )
  : m_bFullScreen(false), m_position{0,0}, m_size{1024,768}, m_fb_size{0,0},
    m_tile_size{256,256}, m_maxLevels( 19 ), m_curLevel(0)
{
  m_rc = std::make_unique<ure::ResourcesCollector>();

  init(argc, argv);
}

void Map::run()
{
  ure::utils::log( "OglGui::Run(): ENTER" );
  ure::Application::get_instance()->run();
  ure::utils::log( "OglGui::Run(): EXIT"  );
}

void Map::dispose()
{
  if ( m_pWindow != nullptr )
  {
    m_pWindow->destroy();

    delete m_pWindow;
    m_pWindow = nullptr;
  }

  if ( m_pViewPort != nullptr )
  {
    delete m_pViewPort;
    m_pViewPort = nullptr;
  }

  ure::Application::get_instance()->finalize();
}

void Map::init( [[__maybe_unused__]] int argc, [[__maybe_unused__]] char** argv ) noexcept
{
  const std::string sShadersPath( "./resources/shaders/" );
  const std::string sMediaPath  ( "./resources/media/" );
  const std::string sTilesURL   ( "https://tile.openstreetmap.org/%u/%u/%u.png" ); 

  ure::Application::initialize( core::unique_ptr<ure::ApplicationEvents>(this,false), sShadersPath );

  //
  m_pWindow = new(std::nothrow) ure::Window();
  if ( m_pWindow == nullptr )
  {
    ure::utils::log( "Unable to start application" );
    return ;
  }

  // Add this as listener for WindowEvents, that means map object implements 
  // ure::WindowEvents interface and will receive all notification from ure::Window
  m_pWindow->connect( this );
  
  std::unique_ptr<ure::window_options> options = std::make_unique<ure::window_options>( 
                                                                                        (m_bFullScreen==true)?"main":"", 
                                                                                        "Map Application",
                                                                                        m_position, 
                                                                                        m_size 
                                                                                      );
  
  m_pWindow->create( std::move(options), static_cast<ure::enum_t>(ure::Window::processing_flag_t::epfCalling) );
  m_pWindow->set_swap_interval(1);

  //////////////////////////////////////////////////////////////////////////

  const ure::Renderer* renderer = m_pWindow->get_renderer();

  std::string  sVendor;
  std::string  sRenderer;
  std::string  sVersion;
  std::string  sShaderVersion;
  
  renderer->get_vendor  ( sVendor );
  ure::utils::log( core::utils::format( "Vendor:         [%s]", sVendor.c_str() )        );
  renderer->get_renderer( sRenderer );
  ure::utils::log( core::utils::format( "Renderer:       [%s]", sRenderer.c_str() )      );
  renderer->get_version( sVersion );
  ure::utils::log( core::utils::format( "Version:        [%s]", sVersion.c_str() )       );
  renderer->get_shader_version( sShaderVersion );
  ure::utils::log( core::utils::format( "Shader Version: [%s]", sShaderVersion.c_str() ) );

  //////////////////////////////////////////////////////////////////////////

  std::unique_ptr<ure::SceneGraph> scene_graph = std::make_unique<ure::SceneGraph>();
  if ( scene_graph == nullptr )
  {
    ure::utils::log( "Unable to allocate SceneGraph" );
    return ;
  }

  glm::mat4 mProjection = glm::perspectiveFov(45.0f, (float)m_size.width, (float)m_size.height, 0.1f, 1000.0f);

  m_pViewPort   = new(std::nothrow) ure::ViewPort( std::move(scene_graph), mProjection );
  if ( m_pViewPort == nullptr )
  {
    ure::utils::log( "Unable to allocate ViewPort" );
    return ;
  }

  load_resources();

  add_camera();

  add_zoom_levels( sTilesURL );
}

void Map::load_resources() noexcept(true)
{
  ure::Image    bkImage; 

  bkImage.load( ure::Image::loader_t::eStb, "./resources/media/images/0-0-0.png" );

  std::unique_ptr<ure::Texture>  txt = std::make_unique<ure::Texture>( std::move(bkImage) );

  auto result = m_rc->attach<ure::Texture>("0-0-0", std::move(txt) );  
  if ( result.first == false )
  {
    // todo
  }
}

void Map::add_camera() noexcept(true)
{
  ure::camera_ptr camera = std::make_shared<ure::Camera>( true );
  if ( camera == nullptr )
    return ;

  /* Initially we are going to use 2D in 3D space, that means no view matrix to be applied in the MVP */
  camera->set_view_matrix( glm::mat4(1) );

  if ( m_pViewPort->has_scene_graph() )
  {
    m_pViewPort->get_scene().add_scene_node( new ure::SceneCameraNode( "MainCamera", camera ) );
  }
}

void Map::add_zoom_levels( const std::string& url ) noexcept(true)
{
  for ( ure::int_t zl = 0; zl < max_levels(); zl++ )
  {
    std::shared_ptr<ure::widgets::Layer> layer = std::make_shared<TileLayer>( *m_pViewPort, m_tile_size, zl, url );
    
    m_pWindow->connect(layer->get_windows_events());

    layer->set_visible( (zl==0) );
    layer->set_enabled( (zl==0) );

    layer->set_position( -1.0f*m_size.width/2, -1.0f*m_size.height/2, true );
    
    auto texture = m_rc->find<ure::Texture>("0-0-0");
    
    if ( texture.has_value() )
      layer->set_background( texture.value(), ure::widgets::Widget::BackgroundOptions::eboAsIs );

    ure::SceneLayerNode* pNode = new(std::nothrow) ure::SceneLayerNode( core::utils::format("Layer%d", zl ), layer );
    
    ure::float_t mx = m_size.width/2;
    ure::float_t my = m_size.height/2;
    glm::mat4 mModel =  glm::ortho( -1.0f*(float)m_size.width/2, (float)m_size.width/2, (float)m_size.height/2, -1.0f*(float)m_size.height/2, 1.0f, 100.0f );
    //glm::mat4 mModel = glm::mat4(1); //glm::ortho( -1.0f*mx, mx, my, -1.0f*my, 0.1f, 1000.0f );

    pNode->set_model_matrix( mModel );
    pNode->get_model_matrix().translate( 0, 0, 0 );

    m_pViewPort->get_scene().add_scene_node( pNode );  
  }
}

/////////////////////////////////////////////////////
// ure::WindowEvents implementation
/////////////////////////////////////////////////////

ure::void_t  Map::on_mouse_scroll( [[maybe_unused]] ure::Window* pWindow, [[maybe_unused]] ure::double_t dOffsetX, [[maybe_unused]] ure::double_t dOffsetY ) noexcept 
{
  ure::SceneLayerNode* current_layer_node = m_pViewPort->get_scene().get_scene_node<ure::SceneLayerNode>("SceneNode", core::utils::format("Layer%d", m_curLevel ) );
  ure::SceneLayerNode* new_layer_node     = nullptr;

  // Increase level
  if ( dOffsetY > 0.0f )
  {
    if ( m_curLevel < m_maxLevels )
      ++m_curLevel;
  }

  // Decrease level
  if ( dOffsetY < 0.0f )
  {
    if ( m_curLevel > 0 )
      --m_curLevel;
  }

  new_layer_node = m_pViewPort->get_scene().get_scene_node<ure::SceneLayerNode>("SceneNode", core::utils::format("Layer%d", m_curLevel ) );

  if ( current_layer_node )
  {
    current_layer_node->get_object<TileLayer>()->set_enabled(false);
    current_layer_node->get_object<TileLayer>()->set_visible(false);
  }

  if ( new_layer_node )
  {
    new_layer_node->get_object<TileLayer>()->set_enabled(true);
    new_layer_node->get_object<TileLayer>()->set_visible(true);
  }

  if ( ( current_layer_node ) && ( new_layer_node ) )
  {
    new_layer_node->get_model_matrix() = current_layer_node->get_model_matrix();
  }

  printf("scroll current level [%d] %f  %f \n", m_curLevel, dOffsetX, dOffsetY );
}

ure::void_t Map::on_mouse_move( [[maybe_unused]] ure::Window* pWindow, [[maybe_unused]] ure::double_t x, [[maybe_unused]] ure::double_t y ) noexcept 
{
  if ( m_move_map )
  {
    ure::Position_d  _delta_pos = m_mouse_last_pos - ure::Position_d( x, y );
    
    ure::SceneLayerNode* _tile_layer = m_pViewPort->get_scene().get_scene_node<ure::SceneLayerNode>("SceneNode", core::utils::format("Layer%d", m_curLevel ) );
    _tile_layer->get_model_matrix().translate( -1.0f*_delta_pos.x, -1.0f*_delta_pos.y, 0 );
    
    printf( "delta x:%f delta y:%f\n", _delta_pos.x, _delta_pos.y );
  }
  m_mouse_last_pos = { x, y };
  printf( "x:%f y:%f\n", x, y );
  
}

ure::void_t Map::on_mouse_button_pressed( [[maybe_unused]] ure::Window* pWindow, [[maybe_unused]] ure::WindowEvents::mouse_button_t button, [[maybe_unused]] ure::int_t mods ) noexcept 
{ 
  switch (button)
  {
    case ure::WindowEvents::mouse_button_t::BUTTON_LEFT:
      m_move_map  = true;
    break;
  
    default:
    break;
  }
}

ure::void_t Map::on_mouse_button_released( [[maybe_unused]] ure::Window* pWindow, [[maybe_unused]] ure::WindowEvents::mouse_button_t button, [[maybe_unused]] ure::int_t mods ) noexcept
{ 
  switch (button)
  {
    case ure::WindowEvents::mouse_button_t::BUTTON_LEFT:
      m_move_map  = false;
    break;
  
    default:
    break;
  }
}


/////////////////////////////////////////////////////
// ure::ApplicationEvents implementation
/////////////////////////////////////////////////////

ure::void_t Map::on_initialize() 
{
  ure::ResourcesFetcher::initialize( );
}

ure::void_t Map::on_initialized() 
{
}

ure::void_t Map::on_finalize() 
{
  ure::ResourcesFetcher::get_instance()->finalize();
}

ure::void_t Map::on_finalized() 
{

}

ure::void_t Map::on_run()
{
  if ( m_pWindow->check( ure::Window::window_flag_t::eWindowShouldClose ) )
  {
    ure::Application::get_instance()->exit(true);
  }

  m_pWindow->get_framebuffer_size( m_fb_size );
    
  ///////////////
  m_pViewPort->set_area( 0, 0, m_fb_size.width, m_fb_size.height );
  m_pViewPort->use();

  // Update background color
  m_pViewPort->get_scene().set_background( 0.2f, 0.2f, 0.2f, 0.0f );

  ///////////////
  m_pViewPort->clear_buffer( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

  ///////////////
  m_pViewPort->render();

  ///////////////
  m_pWindow->swap_buffers();
  
  ///////////////
  // Will process messages that requires to be executed on main thread.
  m_pWindow->process_message();

  ure::Application::get_instance()->poll_events();  
}

ure::void_t Map::on_initialize_error(/* @todo */) 
{

}

ure::void_t Map::on_error( [[maybe_unused]] int32_t error, [[maybe_unused]] const std::string& description )
{

}

ure::void_t Map::on_finalize_error(/* @todo */) 
{

}


/////////////////////////////////////////////////////
// ure::ResourcesFetcherEvents implementation
/////////////////////////////////////////////////////

ure::void_t Map::on_download_succeeded( [[maybe_unused]] const std::string& name, [[maybe_unused]] const std::type_info& type, [[maybe_unused]] const ure::byte_t* data, [[maybe_unused]] ure::uint_t length ) noexcept(true)
{
  if ( m_rc->contains( name ) == false )
  {
    if ( typeid(ure::Texture) == type )
    {
      ure::Image    bkImage; 

      if ( bkImage.create( ure::Image::loader_t::eStb, data, length ) == true )
      {
        std::unique_ptr<ure::Texture>  txt = std::make_unique<ure::Texture>( std::move(bkImage) );

        auto result = m_rc->attach<ure::Texture>( name,  std::move(txt) );  
        if ( result.first == false )
        {
          // @todo
        }

      }
      else
      {
        // @todo
      }
    }
    //ure::ResourcesCollector::get_instance()->attach( name,  )
  }
}

ure::void_t Map::on_download_failed   ( [[maybe_unused]] const std::string& name ) noexcept(true)
{

}

