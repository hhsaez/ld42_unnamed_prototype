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

#include "Player.hpp"
#include "Grid.hpp"
#include "GridObject.hpp"
#include "Consumable.hpp"

#include "Messaging/Messages.hpp"

using namespace hunger;
using namespace hunger::messaging;

using namespace crimild;
using namespace crimild::messaging;

TrailPositionParticleGenerator::TrailPositionParticleGenerator( void )
{

}

TrailPositionParticleGenerator::~TrailPositionParticleGenerator( void )
{

}

void TrailPositionParticleGenerator::configure( Node *node, ParticleData *particles )
{
	_positions = particles->createAttribArray< Vector3f >( ParticleAttrib::POSITION );
	assert( _positions != nullptr );
}

void TrailPositionParticleGenerator::generate( Node *node, crimild::Real64 dt, ParticleData *particles, ParticleId startId, ParticleId endId )
{
	auto ps = _positions->getData< Vector3f >();
	
    for ( ParticleId i = startId; i < endId; i++ ) {
		auto idx = Random::generate< crimild::Real32 >( _trail.size() );
		auto lo = crimild::Int32( idx );
		auto hi = crimild::Int32( idx ) + 1;
		auto p = _trail[ lo ];
		if ( hi < _trail.size() ) {
			auto x = idx - lo / ( hi - lo );
			Vector3f dir = _trail[ hi ] - _trail[ lo ];
			dir.normalize();
			p += x * dir;
		}
		if ( particles->shouldComputeInWorldSpace() ) {
			node->getWorld().applyToPoint( p, p );
		}
		ps[ i ] = p;
    }
}


Player::Player( void )
{
	
}

Player::~Player( void )
{
	
}

void Player::onAttach( void )
{
	auto parent = getNode< Group >();
	
	const auto TAIL_SIZE = 500;
	
	for ( crimild::Size i = 0; i < TAIL_SIZE; i++ ) {
		auto n = crimild::alloc< Node >();
		n->local().setTranslate( Vector3f::POSITIVE_INFINITY );
		parent->attachNode( n );
		_tail.push( TailNode { Vector2i( -1, -1 ), crimild::get_ptr( n ) } );
	}

	/*
	auto g = crimild::alloc< Geometry >();
	auto m = crimild::alloc< Material >();
	m->setProgram( Renderer::getInstance()->getShaderProgram( Renderer::SHADER_PROGRAM_UNLIT_DIFFUSE ) );
	m->setDiffuse( RGBAColorf( 1.0f, 0.0f, 1.0f, 1.0f ) );
	g->getComponent< MaterialComponent >()->attachMaterial( m );
	parent->attachNode( g );
	_renderer = crimild::get_ptr( g );
	*/


	auto particleSystem = crimild::alloc< Group >();
	auto particles = crimild::alloc< ParticleData >( 5000 );
	particles->setComputeInWorldSpace( false );
	auto ps = crimild::alloc< ParticleSystemComponent >( particles );
	ps->setEmitRate( 100 );
	// generators
	//auto posGenerator = crimild::alloc< BoxPositionParticleGenerator >();
	//posGenerator->setOrigin( Vector3f::ZERO );
	//posGenerator->setSize( Vector3f::ONE );
	auto posGenerator = crimild::alloc< NodePositionParticleGenerator >();
	posGenerator->setTargetNode( getNode() );
	_posGenerator = crimild::get_ptr( posGenerator );
	ps->addGenerator( posGenerator );
	/*
	auto velocityGenerator = crimild::alloc< RandomVector3fParticleGenerator >();
	velocityGenerator->setParticleAttribType( ParticleAttrib::VELOCITY );
	velocityGenerator->setMinValue( Vector3f( 0.0f, 1.0f, 0.0f ) );
	velocityGenerator->setMaxValue( Vector3f( 0.0f, 2.0f, 0.0f ) );
	ps->addGenerator( velocityGenerator );
	auto accelGenerator = crimild::alloc< DefaultVector3fParticleGenerator >();
	accelGenerator->setParticleAttribType( ParticleAttrib::ACCELERATION );
	accelGenerator->setValue( Vector3f::ZERO );
	ps->addGenerator( accelGenerator );
	*/
	auto colorGenerator = crimild::alloc< ColorParticleGenerator >();
	colorGenerator->setMinStartColor( RGBAColorf( 0.55f, 0.55f, 0.55f, 1.0f ) );
	colorGenerator->setMaxStartColor( RGBAColorf( 0.35f, 0.35f, 0.35f, 1.0f ) );
	colorGenerator->setMinEndColor( RGBAColorf( 0.15f, 0.15f, 0.15f, 1.0f ) );
	colorGenerator->setMaxEndColor( RGBAColorf( 0.10f, 0.10f, 0.10f, 1.0f ) );
	ps->addGenerator( colorGenerator );
	auto scaleGenerator = crimild::alloc< RandomReal32ParticleGenerator >();
	scaleGenerator->setParticleAttribType( ParticleAttrib::UNIFORM_SCALE );
	scaleGenerator->setMinValue( 2.0f );
	scaleGenerator->setMaxValue( 10.0f );
	ps->addGenerator( scaleGenerator );
	auto timeGenerator = crimild::alloc< TimeParticleGenerator >();
	timeGenerator->setMinTime( 120.0f );
	timeGenerator->setMaxTime( 200.0f );
	ps->addGenerator( timeGenerator );
	// updaters
	//ps->addUpdater( crimild::alloc< EulerParticleUpdater >() );
	ps->addUpdater( crimild::alloc< TimeParticleUpdater >() );
	// renderers
    auto renderer = crimild::alloc< PointSpriteParticleRenderer >();
	renderer->getMaterial()->getCullFaceState()->setEnabled( false );
	ps->addRenderer( renderer );

	particleSystem->attachComponent( ps );
	parent->attachNode( particleSystem );
}

void Player::start( void )
{
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
	
	registerMessageHandler< KeyReleased >( [ this ]( KeyReleased const &m ) {
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

	registerMessageHandler< ConsumableDestroyed >( [ this ]( ConsumableDestroyed const &m ) {
		
	});
}

void Player::update( const Clock &c )
{
	if ( c.getDeltaTime() > 1.0f ) {
		return;
	}
	
	if ( _speed < 60.0f ) {
		_speed += 0.1f * c.getDeltaTime();
	}
	
	const auto FIXED_TIME = 1.0f / _speed;
	
	_t += c.getDeltaTime();
	while ( _t >= FIXED_TIME ) {
		if ( !step() ) {
			broadcastMessage( GameOver { } );
			setEnabled( false );
			return;
		}
		_t -= FIXED_TIME;
	}

	renderTail();
}

crimild::Bool Player::step( void )
{
	auto gridObject = getComponent< GridObject >();
	auto grid = gridObject->getGrid();
	auto gridPos = gridObject->getPosition();
	auto prevPos = gridPos;

	const crimild::Int32 SPEED = 1;
	switch ( _direction ) {
		case Direction::UP:
			gridPos.y() -= 1;
			break;
			
		case Direction::DOWN:
			gridPos.y() += 1;
			break;
			
		case Direction::LEFT:
			gridPos.x() -= 1;
			break;
			
		case Direction::RIGHT:
			gridPos.x() += 1;
			break;
	}
	
	if ( !grid->move( gridPos ) ) {
		Log::debug( CRIMILD_CURRENT_CLASS_NAME, "Game Over!" );
		return false;
	}

	gridObject->setPosition( gridPos );
	auto worldPos = grid->gridPosToWorld( gridPos );

	auto t = _tail.pop();
	if ( t.pos.x() >= 0 && t.pos.y() >= 0 ) {
		grid->setEmpty( t.pos, true );
	}
	t.pos = gridPos;
	t.node->local().setTranslate( worldPos );
	_head = t.node;
	_tail.push( t );
	
	return true;
}

void Player::renderTail( void )
{
	_posGenerator->setTargetNode( getNode() );
	if ( getHead() != nullptr ) {
		_posGenerator->setTargetNode( getHead() );
	}
	
	if ( _tail.empty() ) {
		return;
	}

#if 0
	//const auto VERTEX_COUNT = _tail.size() * 2;
	containers::Array< Vector3f > trail;
	//auto vbo = crimild::alloc< VertexBufferObject >( VertexFormat::VF_P3, VERTEX_COUNT );
	//auto ibo = crimild::alloc< IndexBufferObject >( VERTEX_COUNT );
	//auto prevPos = _tail.front().node->getLocal().getTranslate();
	_tail.each( [ this, /*ibo, vbo, &prevPos,*/ &trail ]( TailNode &t, crimild::Size i ) {
		if ( t.pos.x() >= 0 && t.pos.y() >= 0 ) {
			auto pos = t.node->getLocal().getTranslate();
			trail.add( pos );
			//vbo->setPositionAt( i * 2 + 0, prevPos );
			//vbo->setPositionAt( i * 2 + 1, pos );
			//prevPos = pos;
			//ibo->setIndexAt( i * 2 + 0, i * 2 + 0 );
			//ibo->setIndexAt( i * 2 + 1, i * 2 + 1 );
		}
	});

	_posGenerator->setTrail( trail );	

	//auto primitive = crimild::alloc< Primitive >( Primitive::Type::LINES );
	//primitive->setVertexBuffer( vbo );
	//primitive->setIndexBuffer( ibo );

	//_renderer->detachAllPrimitives();
	//_renderer->attachPrimitive( primitive );
#endif
}

