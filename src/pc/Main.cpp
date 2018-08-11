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

#include "Components/Grid.hpp"
#include "Components/Player.hpp"
#include "Components/Consumable.hpp"

using namespace hunger;

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

	auto m = crimild::alloc< Material >();
	m->setSpecular( RGBAColorf::ZERO );
	m->setShininess( 0.0f );

	// debug grid
	auto g = crimild::alloc< Geometry >();
	g->attachPrimitive( crimild::alloc< ConePrimitive >( Primitive::Type::LINES, HEIGHT, 0.5f * WIDTH ) );
	g->getComponent< MaterialComponent >()->attachMaterial( m );
	grid->attachNode( g );

	// temp plane for background
	auto plane = crimild::alloc< Geometry >();
	plane->attachPrimitive( crimild::alloc< QuadPrimitive >( 10000.0f, 10000.0f ) );
	plane->local().rotate().fromAxisAngle( Vector3f::UNIT_X, Numericf::HALF_PI );
	plane->local().setTranslate( 0.0f, 0.25f * HEIGHT, 0.0f );
	plane->getComponent< MaterialComponent >()->attachMaterial( m );
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

SharedPointer< Group > loadGame( void )
{
    auto scene = crimild::alloc< Group >();

	auto grid = createGrid();
	scene->attachNode( grid );

    auto camera = createCamera();
    scene->attachNode( camera );

	auto light = crimild::alloc< Light >( Light::Type::POINT );
	camera->attachNode( light );

    return scene;
}

int main( int argc, char **argv )
{
	crimild::init();

	SIM_LIFETIME auto sim = crimild::alloc< SDLSimulation >( "LD42", crimild::alloc< Settings >( argc, argv ) );

	sim->getRenderer()->getScreenBuffer()->setClearColor( RGBAColorf( 0.5f, 0.5f, 0.5f, 1.0f ) );

	sim->setScene( loadGame() );

	sim->registerMessageHandler< KeyReleased >( []( KeyReleased const &m ) {
		switch ( m.key ) {
			case CRIMILD_INPUT_KEY_R: {
				crimild::concurrency::sync_frame( [] {
					auto sim = Simulation::getInstance();
					sim->setScene( nullptr );
					auto scene = loadGame();
					sim->setScene( scene );
				});
				break;
			}
		}
	});
	
	return sim->run();
}

