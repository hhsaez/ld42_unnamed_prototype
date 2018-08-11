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
		_state.each( []( crimild::Bool &s ) {
			s = false;
		});
	}

	virtual ~Grid( void)
	{

	}

	crimild::Int32 getWidth( void ) const { return _width; }
	crimild::Int32 getHeight( void ) const { return _height; }

	crimild::Bool isEmpty( crimild::Vector2i pos ) const
	{
		return !_state[ pos.y() * _width + pos.x() ];
	}

	void setEmpty( crimild::Vector2i pos, crimild::Bool empty )
	{
		_state[ pos.y() * _width + pos.x() ] = !empty;
	}

	crimild::Bool move( crimild::Vector2i &pos )
	{
		pos.x() = ( getWidth() + pos.x() ) % getWidth();
		pos.y() = ( getHeight() + pos.y() ) % getHeight();
		/*
		if ( gridPos.y() < 0 || gridPos.y() >= getHeight() ) {
			return false;
		}
		*/
		
		if ( !isEmpty( pos ) ) {
			return false;
		}

		setEmpty( pos, false );

		return true;
	}

private:
	crimild::Int32 _width;
	crimild::Int32 _height;
	containers::Array< crimild::Bool > _state;
};

class Player :
	public NodeComponent,
	public Messenger {
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
			g->attachPrimitive( crimild::alloc< BoxPrimitive >( 2.0f, 2.0f, 2.0f ) );
			g->getComponent< MaterialComponent >()->attachMaterial( m );
			g->local().setTranslate( Vector3f::POSITIVE_INFINITY );
			parent->attachNode( g );
			_tail.push( TailNode { Vector2i( -1, -1 ), crimild::get_ptr( g ) } );
		}
	}

	virtual void start( void ) override
	{
		auto grid = Grid::getInstance();

		auto x = Random::generate< crimild::Int32 >( grid->getWidth() );
		auto y = Random::generate< crimild::Int32 >( grid->getHeight() );

		_gridPos = Vector2i( x, y );
		_speed = 10.0f;

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

		registerMessageHandler< messaging::KeyReleased >( [ this ]( messaging::KeyReleased const &m ) {
			if ( m.key == CRIMILD_INPUT_KEY_LEFT ) {
				switch ( _direction ) {
					case Direction::UP:
						_direction = Direction::LEFT;
						break;
						
					case Direction::DOWN:
						_direction = Direction::RIGHT;
						break;
						
					case Direction::LEFT:
						_direction = Direction::DOWN;
						break;
						
					case Direction::RIGHT:
						_direction = Direction::UP;
						break;
				}
			}
			else if ( m.key == CRIMILD_INPUT_KEY_RIGHT ) {
				switch ( _direction ) {
					case Direction::UP:
						_direction = Direction::RIGHT;
						break;
						
					case Direction::DOWN:
						_direction = Direction::LEFT;
						break;
						
					case Direction::LEFT:
						_direction = Direction::UP;
						break;
						
					case Direction::RIGHT:
						_direction = Direction::DOWN;
						break;
				}
			}
		});
	}

	virtual void update( const Clock &c ) override
	{
		if ( c.getDeltaTime() > 1.0f ) {
			return;
		}

		if ( _speed < 60.0f ) {
			_speed += c.getDeltaTime();
		}
		
		const auto FIXED_TIME = 1.0f / _speed;

		_t += c.getDeltaTime();
		while ( _t >= FIXED_TIME ) {
			if ( !step() ) {
				setEnabled( false );
				return;
			}
			_t -= FIXED_TIME;
		}
	}

private:
	crimild::Bool step( void )
	{
		auto grid = Grid::getInstance();

		auto prevPos = _gridPos;

		const crimild::Int32 SPEED = 1;
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
		
		if ( !grid->move( _gridPos ) ) {
			Log::debug( CRIMILD_CURRENT_CLASS_NAME, "Game Over!" );
			return false;
		}

		auto u = Numericf::TWO_PI * _gridPos.x() / ( grid->getWidth() - 1.0f );
		auto v = 0.75f * _gridPos.y() / ( grid->getHeight() - 1.0f );
		auto r = 0.5f * grid->getWidth();
		auto h = grid->getHeight();
		auto x = r * ( 1.0f - v ) * std::cos( u );
		auto y = h * ( v - 0.5f );
		auto z = r * ( 1.0f - v ) * -std::sin( u );

		auto t = _tail.pop();
		if ( t.pos.x() >= 0 && t.pos.y() >= 0 ) {
			grid->setEmpty( t.pos, true );
		}
		t.pos = _gridPos;
		t.node->local().setTranslate( x, y, z );
		_tail.push( t );

		return true;
	}

private:
	crimild::Real32 _t = 0.0f;
	crimild::Real32 _speed = 10.0f;
	crimild::Vector2i _gridPos;
	Direction _direction;

	struct TailNode {
		Vector2i pos;
		Node *node;
	};
	
	containers::Queue< TailNode > _tail;
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
	auto grid = Grid::getInstance();
	auto gridCenter = Vector3f::ZERO;
	
	auto camera = crimild::alloc< Camera >();
    camera->local().setTranslate( gridCenter - 2.0f * Vector3f::UNIT_X + 120.0f * Vector3f::UNIT_Y + 50.0f * Vector3f::UNIT_Z );
	camera->local().lookAt( gridCenter - 2.0f * Vector3f::UNIT_X - 10.0f * Vector3f::UNIT_Z );

	return camera;
}

SharedPointer< Group > loadGame( void )
{
    auto scene = crimild::alloc< Group >();

	auto grid = createGrid();
	scene->attachNode( grid );

	auto player = createPlayer();
	grid->attachNode( player );

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

	sim->registerMessageHandler< messaging::KeyReleased >( []( messaging::KeyReleased const &m ) {
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

