/*
 * Copyright (c) 2018, Hugo Hernan Saez
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Crimild.hpp>
#include <Crimild_SDL.hpp>

#include "Messaging/Messages.hpp"
#include "Components/Grid.hpp"
#include "Components/Player.hpp"
#include "Components/Consumable.hpp"

namespace crimild {

	class MessageHandlerComponent :
		public NodeComponent,
		public crimild::Messenger {
		CRIMILD_IMPLEMENT_RTTI( crimild::MessageHandlerComponent )

	public:
		MessageHandlerComponent( void ) { }
		virtual ~MessageHandlerComponent( void ) { }
	};

}

using namespace hunger;
using namespace hunger::messaging;

using namespace crimild;
using namespace crimild::sdl;
using namespace crimild::messaging;

#ifdef CRIMILD_PLATFORM_EMSCRIPTEN
#define SIM_LIFETIME static
#else
#define SIM_LIFETIME
#endif

SharedPointer< Group > createGrid( void )
{
	const auto WIDTH = 100;
	const auto HEIGHT = 100;
	
	auto grid = crimild::alloc< Group >();

	auto g = crimild::alloc< Geometry >();
	g->attachPrimitive( crimild::alloc< ConePrimitive >( Primitive::Type::LINES, HEIGHT, 0.5f * WIDTH ) );
	auto gridMaterial = crimild::alloc< Material >();
	gridMaterial->setDiffuse( RGBAColorf( 0.0f, 0.0f, 0.0f, 1.0f ) );
	gridMaterial->setProgram( Renderer::getInstance()->getShaderProgram( Renderer::SHADER_PROGRAM_UNLIT_DIFFUSE ) );
	g->getComponent< MaterialComponent >()->attachMaterial( gridMaterial );
	grid->attachNode( g );

	auto plane = crimild::alloc< Geometry >();
	plane->attachPrimitive( crimild::alloc< QuadPrimitive >( 10000.0f, 10000.0f ) );
	plane->local().rotate().fromAxisAngle( Vector3f::UNIT_X, Numericf::HALF_PI );
	plane->local().setTranslate( 0.0f, 0.25f * HEIGHT, 0.0f );
	auto planeMaterial = crimild::alloc< Material >();
	planeMaterial->setDiffuse( RGBAColorf( 1.0f, 1.0f, 1.0f, 1.0f ) );
	planeMaterial->setProgram( Renderer::getInstance()->getShaderProgram( Renderer::SHADER_PROGRAM_UNLIT_DIFFUSE ) );
	plane->getComponent< MaterialComponent >()->attachMaterial( planeMaterial );
	grid->attachNode( plane );

	grid->attachComponent< Grid >( WIDTH, HEIGHT );
	grid->local().rotate().fromAxisAngle( Vector3f::UNIT_X, Numericf::PI );
	return grid;
}

SharedPointer< Camera > createCamera( void )
{
	auto camera = crimild::alloc< Camera >();
    camera->local().setTranslate( -2.0f * Vector3f::UNIT_X + 120.0f * Vector3f::UNIT_Y + 50.0f * Vector3f::UNIT_Z );
	camera->local().lookAt( -2.0f * Vector3f::UNIT_X - 10.0f * Vector3f::UNIT_Z );

	return camera;
}

SharedPointer< Node > createInGameUI( void )
{
	auto fontFileName = FileSystem::getInstance().pathForResource( "assets/fonts/Verdana.txt" );
	auto font = crimild::alloc< Font >( fontFileName );

	auto btnMenu = crimild::alloc< Text >();
	btnMenu->setFont( font );
	btnMenu->setSize( 0.025f );
	btnMenu->setTextColor( RGBAColorf( 0.0f, 0.0f, 0.0f, 1.0f ) );
	btnMenu->setText( "MENU" );
	btnMenu->setHorizontalAlignment( Text::HorizontalAlignment::LEFT );
	btnMenu->local().setTranslate( -0.75f, 0.525f, 0.0f );
	btnMenu->attachComponent< UIResponder >( []( Node * ) -> crimild::Bool {
		MessageQueue::getInstance()->broadcastMessage( QuitGame {} );
		return true;
	});

	auto inGameUI = crimild::alloc< Group >();
	inGameUI->attachNode( btnMenu );
	auto weakInGameUI = crimild::get_ptr( inGameUI );
	inGameUI->attachComponent< MessageHandlerComponent >()->registerMessageHandler< GameOver >( [ weakInGameUI ]( GameOver const & ) {
		weakInGameUI->setEnabled( false );		
	});
	return inGameUI;
}

SharedPointer< Node > createGameOverUI( void )
{
	auto fontFileName = FileSystem::getInstance().pathForResource( "assets/fonts/Verdana.txt" );
	auto font = crimild::alloc< Font >( fontFileName );

	auto ui = crimild::alloc< Group >();

	auto lblTitle = crimild::alloc< Text >();
	lblTitle->setFont( font );
	lblTitle->setSize( 0.25f );
	lblTitle->setTextColor( RGBAColorf( 1.0f, 0.0f, 0.0f, 1.0f ) );
	lblTitle->setText( "Game Over" );
	lblTitle->setHorizontalAlignment( Text::HorizontalAlignment::CENTER );
	lblTitle->local().setTranslate( 0.0f, 0.1f, 0.0f );
	ui->attachNode( lblTitle );

	auto btnPlay = crimild::alloc< Text >();
	btnPlay->setFont( font );
	btnPlay->setSize( 0.05f );
	btnPlay->setTextColor( RGBAColorf( 1.0f, 0.0f, 0.0f, 1.0f ) );
	btnPlay->setText( "RESTART" );
	btnPlay->setHorizontalAlignment( Text::HorizontalAlignment::CENTER );
	btnPlay->local().setTranslate( 0.0f, -0.1f, 0.0f );
	btnPlay->attachComponent< UIResponder >( []( Node * ) -> crimild::Bool {
		MessageQueue::getInstance()->broadcastMessage( StartGame { } );
		return true;
	});
	ui->attachNode( btnPlay );

	auto btnQuit = crimild::alloc< Text >();
	btnQuit->setFont( font );
	btnQuit->setSize( 0.05f );
	btnQuit->setTextColor( RGBAColorf( 1.0f, 0.0f, 0.0f, 1.0f ) );
	btnQuit->setText( "MAIN MENU" );
	btnQuit->setHorizontalAlignment( Text::HorizontalAlignment::CENTER );
	btnQuit->local().setTranslate( 0.0f, -0.2f, 0.0f );
	btnQuit->attachComponent< UIResponder >( []( Node * ) -> crimild::Bool {
		MessageQueue::getInstance()->broadcastMessage( QuitGame {} );
		return true;
	});
	ui->attachNode( btnQuit );

	auto weakUI = crimild::get_ptr( ui );
	ui->attachComponent< MessageHandlerComponent >()->registerMessageHandler< GameOver >( [ weakUI ]( GameOver const & ) {
		weakUI->setEnabled( true );
	});

	// required, since we're disable the node from the start
	ui->perform( UpdateRenderState() );
	ui->perform( UpdateWorldState() );
	ui->perform( StartComponents() );
	ui->setEnabled( false );

	return ui;
}

SharedPointer< Group > createGameUI( void )
{
	auto ui = crimild::alloc< Group >();
	ui->attachNode( createInGameUI() );
	ui->attachNode( createGameOverUI() );
	ui->local().setTranslate( 0.0f, 0.0f, -1.0f );
	return ui;
}

SharedPointer< Group > createGameScene( void )
{
    auto scene = crimild::alloc< Group >();

	auto grid = createGrid();
	scene->attachNode( grid );

    auto camera = createCamera();
	camera->attachNode( createGameUI() );
    scene->attachNode( camera );

	auto light = crimild::alloc< Light >( Light::Type::POINT );
	camera->attachNode( light );

    return scene;
}

SharedPointer< Group > createMainMenuScene( void )
{
	auto scene = crimild::alloc< Group >();

	{
		auto background = crimild::alloc< Geometry >();
		background->attachPrimitive( crimild::alloc< QuadPrimitive >( 100, 100 ) );
		auto m = crimild::alloc< Material >();
		m->setDiffuse( RGBAColorf::ONE );
		m->setProgram( Renderer::getInstance()->getShaderProgram( Renderer::SHADER_PROGRAM_UNLIT_DIFFUSE ) );
		background->getComponent< MaterialComponent >()->attachMaterial( m );
		background->local().setTranslate( 0.0f, 0.0f, -5.0f );
		scene->attachNode( background );
	}

	auto camera = crimild::alloc< Camera >();
	camera->local().setTranslate( 0.0f, 0.0f, 20.0f );
	scene->attachNode( camera );

	auto ui = crimild::alloc< Group >();

	auto fontFileName = FileSystem::getInstance().pathForResource( "assets/fonts/Verdana.txt" );
	auto font = crimild::alloc< Font >( fontFileName );

	auto lblTitle = crimild::alloc< Text >();
	lblTitle->setFont( font );
	lblTitle->setSize( 3.0f );
	lblTitle->setTextColor( RGBAColorf( 0.0f, 0.0f, 0.0f, 1.0f ) );
	lblTitle->setText( "THE HUNGER" );
	lblTitle->setHorizontalAlignment( Text::HorizontalAlignment::CENTER );
	lblTitle->local().setTranslate( 0.0f, 2.0f, 0.0f );
	ui->attachNode( lblTitle );

	auto btnPlay = crimild::alloc< Text >();
	btnPlay->setFont( font );
	btnPlay->setSize( 1.0f );
	btnPlay->setTextColor( RGBAColorf( 0.0f, 0.0f, 0.0f, 1.0f ) );
	btnPlay->setText( "Play" );
	btnPlay->setHorizontalAlignment( Text::HorizontalAlignment::CENTER );
	btnPlay->local().setTranslate( 0.0f, -1.0f, 0.0f );
	btnPlay->attachComponent< UIResponder >( []( Node * ) -> crimild::Bool {
		MessageQueue::getInstance()->broadcastMessage( StartGame { } );
		return true;
	});
	ui->attachNode( btnPlay );

	auto btnQuit = crimild::alloc< Text >();
	btnQuit->setFont( font );
	btnQuit->setSize( 1.0f );
	btnQuit->setTextColor( RGBAColorf( 0.0f, 0.0f, 0.0f, 1.0f ) );
	btnQuit->setText( "Quit" );
	btnQuit->setHorizontalAlignment( Text::HorizontalAlignment::CENTER );
	btnQuit->local().setTranslate( 0.0f, -3.0f, 0.0f );
	btnQuit->attachComponent< UIResponder >( []( Node * ) -> crimild::Bool {
		crimild::concurrency::sync_frame( [] {
			Simulation::getInstance()->stop();
		});
		return true;
	});
	ui->attachNode( btnQuit );
	
	scene->attachNode( ui );

	return scene;
}

int main( int argc, char **argv )
{
	crimild::init();

	SIM_LIFETIME auto sim = crimild::alloc< SDLSimulation >( "LD42", crimild::alloc< Settings >( argc, argv ) );

	sim->registerMessageHandler< StartGame >( []( StartGame const & ) {
		crimild::concurrency::sync_frame( [] {
			auto sim = Simulation::getInstance();
			sim->setScene( nullptr );
			sim->setScene( createGameScene() );
		});
	});

	sim->registerMessageHandler< QuitGame >( []( QuitGame const & ) {
		crimild::concurrency::sync_frame( [] {
			auto sim = Simulation::getInstance();
			sim->setScene( nullptr );
			sim->setScene( createMainMenuScene() );
		});
	});

	sim->setScene( createMainMenuScene() );

	return sim->run();
}

