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

  ure::Application::initialize( core::unique_ptr<ure::ApplicationEvents>(this,false), sShadersPath, sMediaPath );

  //
  m_pWindow = new(std::nothrow) ure::Window();
  if ( m_pWindow == nullptr )
  {
    ure::utils::log( "Unable to start application" );
    return ;
  }

  // Add this as listener for WindowEvents
  m_pWindow->connect( this );
  
  std::unique_ptr<ure::window_options> options = 
        std::make_unique<ure::window_options>( 
              (m_bFullScreen==true)?"main":"", 
                "Map Application",
                m_position, 
                m_size 
        );
  
  m_pWindow->create( std::move(options), static_cast<ure::enum_t>(ure::Window::ProcessingFlags::epfCalling) );
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

  ure::SceneGraph* pSceneGraph = new(std::nothrow) ure::SceneGraph();
  if ( pSceneGraph == nullptr )
  {
    ure::utils::log( "Unable to allocate SceneGraph" );
    return ;
  }

  glm::mat4 mProjection = glm::perspectiveFov(45.0f, (float)m_size.width, (float)m_size.height, 0.1f, 100.0f);

  m_pViewPort   = new(std::nothrow) ure::ViewPort( pSceneGraph, mProjection );
  if ( m_pViewPort == nullptr )
  if ( pSceneGraph == nullptr )
  {
    ure::utils::log( "Unable to allocate ViewPort" );
    return ;
  }

  load_resources();

  add_camera();

  add_zoom_levels( sTilesURL );
}

void Map::load_resources() noexcept
{
  ure::Image    bkImage; 

  bkImage.load( ure::Image::loader_t::eStb, "./resources/media/images/0-0-0.png" );

  std::unique_ptr<ure::Texture>  txt = std::make_unique<ure::Texture>( std::move(bkImage) );

  auto result = ure::ResourcesCollector::get_instance()->attach<ure::Texture>("0-0-0", std::move(txt) );  
  if ( result.first == false )
  {
    // todo
  }
}

void Map::add_camera() noexcept
{
  ure::Camera*  pCamera =  new ure::Camera( true );

  glm::vec3 cameraPosition = glm::vec3(0,0,1);  // Camera is at (0,0,1), in World Space
  glm::vec3 cameraTarget   = glm::vec3(0,0,0);  // and looks at the origin
  glm::vec3 upVector       = glm::vec3(0,1,0);  // Head is up (set to 0,-1,0 to look upside-down)

  glm::mat4 CameraMatrix = glm::lookAt(
                                          cameraPosition, // the position of your camera, in world space
                                          cameraTarget,   // where you want to look at, in world space
                                          upVector        // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
                                      );
  pCamera->set_view_matrix( CameraMatrix );

  m_pViewPort->get_scene()->add_scene_node( new ure::SceneCameraNode( "MainCamera", pCamera ) );
}

void Map::add_zoom_levels( const std::string& url ) noexcept
{
  for ( ure::int_t zl = 0; zl < max_levels(); zl++ )
  {
    ure::widgets::Layer* pLayer = new(std::nothrow) TileLayer( *m_pViewPort, m_tile_size, zl, url );
    
    m_pWindow->connect(pLayer);

    pLayer->set_visible( (zl==0) );
    pLayer->set_enabled( (zl==0) );

    pLayer->set_position( -1.0f*m_size.width/2, -1.0f*m_size.height/2, true );

    ure::Texture* pTexture = ure::ResourcesCollector::get_instance()->find<ure::Texture>("0-0-0");
    
    pLayer->set_background( pTexture, ure::widgets::Widget::BackgroundOptions::eboAsIs );

    ure::SceneLayerNode* pNode = new(std::nothrow) ure::SceneLayerNode( core::utils::format("Layer%d", zl ), pLayer );
    
    ure::float_t mx = m_size.width/2;
    ure::float_t my = m_size.height/2;
    //glm::mat4 mModel =  glm::ortho( -1.0f*(float)m_size.width/2, (float)m_size.width/2, (float)m_size.height/2, -1.0f*(float)m_size.height/2, 1.0f, 100.0f );
    glm::mat4 mModel =  glm::ortho( -1.0f*mx, mx, my, -1.0f*my, 0.1f, 100.0f );

    pNode->set_model_matrix( mModel );
    pNode->get_model_matrix().translate( 0, 0, 0 );

    m_pViewPort->get_scene()->add_scene_node( pNode );  
  }
}

/////////////////////////////////////////////////////
// ure::WindowEvents implementation
/////////////////////////////////////////////////////

ure::void_t  Map::on_mouse_scroll( [[maybe_unused]] ure::Window* pWindow, [[maybe_unused]] ure::double_t dOffsetX, [[maybe_unused]] ure::double_t dOffsetY ) noexcept 
{
  ure::SceneLayerNode* current_layer_node = m_pViewPort->get_scene()->get_scene_node<ure::SceneLayerNode>("SceneNode", core::utils::format("Layer%d", m_curLevel ) );
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

  new_layer_node = m_pViewPort->get_scene()->get_scene_node<ure::SceneLayerNode>("SceneNode", core::utils::format("Layer%d", m_curLevel ) );

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


  printf("scroll current level [%d] %f  %f \n", m_curLevel, dOffsetX, dOffsetY );
}

/////////////////////////////////////////////////////
// ure::ApplicationEvents implementation
/////////////////////////////////////////////////////

ure::void_t Map::on_initialize() 
{
  ure::ResourcesFetcher::initialize( core::unique_ptr<ure::ResourcesFetcherEvents>(this,false) );
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
  if ( m_pWindow->check( ure::Window::WindowFlags::eWindowShouldClose ) )
  {
    ure::Application::get_instance()->quit(true);
  }

  m_pWindow->get_framebuffer_size( m_fb_size );
    
  ///////////////
  m_pViewPort->set_area( 0, 0, m_fb_size.width, m_fb_size.height );
  m_pViewPort->use();

  // Update background color
  m_pViewPort->get_scene()->set_background( 0.2f, 0.2f, 0.2f, 0.0f );

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

ure::void_t Map::on_download_succeeded( [[maybe_unused]] const std::string& name, [[maybe_unused]] const std::type_info& type, [[maybe_unused]] const ure::byte_t* data, [[maybe_unused]] ure::uint_t length ) 
{
  if ( ure::ResourcesCollector::get_instance()->contains( name ) == false )
  {
    if ( typeid(ure::Texture) == type )
    {
      ure::Image    bkImage; 

      if ( bkImage.create( ure::Image::loader_t::eStb, data, length ) == true )
      {
        std::unique_ptr<ure::Texture>  txt = std::make_unique<ure::Texture>( std::move(bkImage) );

        auto result = ure::ResourcesCollector::get_instance()->attach<ure::Texture>( name,  std::move(txt) );  
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

ure::void_t Map::on_download_failed   ( [[maybe_unused]] const std::string& name ) 
{

}

