/*
 * Copyright (c) 2013, Hernan Saez
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

using namespace crimild;
using namespace crimild::audio;
using namespace crimild::sdl;
using namespace crimild::animation;
using namespace crimild::messaging;

#ifdef CRIMILD_PLATFORM_EMSCRIPTEN
#define SIM_LIFETIME static
#else
#define SIM_LIFETIME
#endif

enum class Direction {
	UP,
	DOWN,
	LEFT,
	RIGHT,
};

class Grid :
	public NodeComponent,
	public DynamicSingleton< Grid > {
	CRIMILD_IMPLEMENT_RTTI( Grid )

public:
	Grid( crimild::Int32 width, crimild::Int32 height )
	    : _width( width ),
		_height( height ),
		_state( _width * _height )
	{

	}

	virtual ~Grid( void)
	{

	}

	crimild::Int32 getWidth( void ) const { return _width; }
	crimild::Int32 getHeight( void ) const { return _height; }

	crimild::Bool isEmpty( crimild::Vector2i pos ) const { return !_state[ pos.y() * _width + pos.x() ]; }
	void setEmpty( crimild::Vector2i pos, crimild::Bool empty ) { _state[ pos.y() * _width + pos.x() ] = empty; }

private:
	crimild::Int32 _width;
	crimild::Int32 _height;
	containers::Array< crimild::Bool > _state;
};

class Player : public NodeComponent {
	CRIMILD_IMPLEMENT_RTTI( Player )

public:
	Player( void )
	{

	}

	virtual ~Player( void )
	{

	}

	virtual void onAttach( void ) override
	{
		auto parent = getNode< Group >();

		const auto TAIL_SIZE = 50;

		auto m = crimild::alloc< Material >();
		m->setDiffuse( RGBAColorf( 0.0f, 0.0f, 1.0f, 1.0f ) );
		
		for ( crimild::Size i = 0; i < TAIL_SIZE; i++ ) {
			auto g = crimild::alloc< Geometry >();
			g->attachPrimitive( crimild::alloc< BoxPrimitive >( 1.0f, 1.0f, 1.0f ) );
			g->getComponent< MaterialComponent >()->attachMaterial( m );
			g->local().setTranslate( Vector3f::POSITIVE_INFINITY );
			parent->attachNode( g );
			_tail.push( crimild::get_ptr( g ) );
		}
	}

	virtual void start( void ) override
	{
		auto grid = Grid::getInstance();

		auto x = Random::generate< crimild::Int32 >( grid->getWidth() );
		auto y = Random::generate< crimild::Int32 >( grid->getHeight() );

		_gridPos = Vector2i( x, y );

		auto d = Random::generate< crimild::Int32 >( 4 );
		switch ( d ) {
			case 0:
				_direction = Direction::UP;
				break;

			case 1:
				_direction = Direction::DOWN;
				break;

			case 2:
				_direction = Direction::LEFT;
				break;

			case 3:
				_direction = Direction::RIGHT;
				break;
		}
	}

	virtual void update( const Clock &c ) override
	{
		auto grid = Grid::getInstance();

		auto input = Input::getInstance();

		if ( input->isKeyDown( CRIMILD_INPUT_KEY_UP ) ) _direction = Direction::UP;
		if ( input->isKeyDown( CRIMILD_INPUT_KEY_DOWN ) ) _direction = Direction::DOWN;
		if ( input->isKeyDown( CRIMILD_INPUT_KEY_LEFT ) ) _direction = Direction::LEFT;
		if ( input->isKeyDown( CRIMILD_INPUT_KEY_RIGHT ) ) _direction = Direction::RIGHT;
		
		switch ( _direction ) {
			case Direction::UP:
				_gridPos.y() -= 1;
				break;

			case Direction::DOWN:
				_gridPos.y() += 1;
				break;

			case Direction::LEFT:
				_gridPos.x() -= 1;
				break;

			case Direction::RIGHT:
				_gridPos.x() += 1;
				break;
		}

		if ( _gridPos.x() < 0 ) _gridPos.x() = grid->getWidth() - 1;
		if ( _gridPos.x() >= grid->getWidth() ) _gridPos.x() = 0;
		if ( _gridPos.y() < 0 ) _gridPos.y() = grid->getHeight() - 1;
		if ( _gridPos.y() >= grid->getHeight() ) _gridPos.y() = 0;

		auto u = Numericf::TWO_PI * _gridPos.x() / ( grid->getWidth() - 1.0f );
		auto v = _gridPos.y() / ( grid->getHeight() - 1.0f );
		auto r = 0.5f * grid->getWidth();
		auto h = grid->getHeight();
		auto x = r * ( 1.0f - v ) * std::cos( u );
		auto y = h * ( v - 0.5f );
		auto z = r * ( 1.0f - v ) * -std::sin( u );

		auto t = _tail.pop();
		t->local().setTranslate( x, y, z );//_gridPos.x(), 0, _gridPos.y() );
		_tail.push( t );
	}

private:
	crimild::Vector2i _gridPos;
	Direction _direction;
	containers::Queue< Node * > _tail;
};

SharedPointer< Group > createPlayer( void )
{
	auto player = crimild::alloc< Group >();
	player->attachComponent< Player >();

	return player;
}

SharedPointer< Group > createGrid( void )
{
	const auto WIDTH = 100;
	const auto HEIGHT = 100;
	
	auto g = crimild::alloc< Geometry >();
	g->attachPrimitive( crimild::alloc< ConePrimitive >( Primitive::Type::LINES, HEIGHT, 0.5f * WIDTH ) );
	//g->attachPrimitive( crimild::alloc< QuadPrimitive >( WIDTH, HEIGHT ) );
	//g->local().setTranslate( 0.5f * WIDTH, -50.0f, 0.5f * HEIGHT );

	auto m = crimild::alloc< Material >();
	m->setSpecular( RGBAColorf::ZERO );
	m->setShininess( 0.0f );
	g->getComponent< MaterialComponent >()->attachMaterial( m );
	
	auto grid = crimild::alloc< Group >();
	grid->attachNode( g );
	grid->attachComponent< Grid >( WIDTH, HEIGHT );
	grid->local().rotate().fromAxisAngle( Vector3f::UNIT_X, Numericf::PI );
	return grid;
}

SharedPointer< Camera > createCamera( void )
{
	auto grid = Grid::getInstance();
	auto gridCenter = Vector3f::ZERO;//Vector3f( 0.5f * grid->getWidth(), 0.0f, 0.5f * grid->getHeight() );
	
	auto camera = crimild::alloc< Camera >();
    camera->local().setTranslate( gridCenter + 100.0f * Vector3f::UNIT_Y + 100.0f * Vector3f::UNIT_Z );
	camera->local().lookAt( gridCenter );

	return camera;
}

int main( int argc, char **argv )
{
	crimild::init();

	SIM_LIFETIME auto sim = crimild::alloc< SDLSimulation >( "LD42", crimild::alloc< Settings >( argc, argv ) );

	sim->getRenderer()->getScreenBuffer()->setClearColor( RGBAColorf( 0.5f, 0.5f, 0.5f, 1.0f ) );

    auto scene = crimild::alloc< Group >();

	auto grid = createGrid();
	scene->attachNode( grid );

	auto player = createPlayer();
	grid->attachNode( player );

    auto camera = createCamera();
    scene->attachNode( camera );

	auto light = crimild::alloc< Light >( Light::Type::POINT );
	camera->attachNode( light );

    sim->setScene( scene );

	return sim->run();
}

