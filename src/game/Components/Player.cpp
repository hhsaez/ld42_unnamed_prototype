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

using namespace hunger;
using namespace hunger::messaging;

using namespace crimild;
using namespace crimild::messaging;

Player::Player( void )
{
	
}

Player::~Player( void )
{
	
}

void Player::onAttach( void )
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
			setEnabled( false );
			return;
		}
		_t -= FIXED_TIME;
	}
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

