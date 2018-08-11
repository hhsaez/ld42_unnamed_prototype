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

#include "Grid.hpp"
#include "Player.hpp"
#include "Consumable.hpp"
#include "GridObject.hpp"

using namespace hunger;
using namespace hunger::messaging;

using namespace crimild;

Grid::Grid( crimild::Int32 width, crimild::Int32 height )
	: _width( width ),
	  _height( height ),
	  _state( _width * _height )
{
	_state.each( []( crimild::Bool &s ) {
		s = false;
	});
}

Grid::~Grid( void)
{
	
}

void Grid::onAttach( void )
{
	spawnPlayer();
	for ( crimild::Int32 i = 0; i < 5; i++ ) {
		spawnConsumable();
	}
}

void Grid::start( void )
{
	registerMessageHandler< ConsumableDestroyed >( [ this ]( ConsumableDestroyed const & ) {
		crimild::concurrency::sync_frame( [ this ] {
			spawnConsumable();
		});
	});
}

crimild::Bool Grid::isEmpty( crimild::Vector2i pos ) const
{
	return !_state[ pos.y() * _width + pos.x() ];
}

void Grid::setEmpty( crimild::Vector2i pos, crimild::Bool empty )
{
	_state[ pos.y() * _width + pos.x() ] = !empty;
}

crimild::Bool Grid::move( crimild::Vector2i &pos )
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

Vector3f Grid::gridPosToWorld( const Vector2i gridPos ) const
{
	auto u = Numericf::TWO_PI * gridPos.x() / ( getWidth() - 1.0f );
	auto v = 0.75f * gridPos.y() / ( getHeight() - 1.0f );
	auto r = 0.5f * getWidth();
	auto h = getHeight();
	auto x = r * ( 1.0f - v ) * std::cos( u );
	auto y = h * ( v - 0.5f );
	auto z = r * ( 1.0f - v ) * -std::sin( u );
	
	return Vector3f( x, y, z );
}

void Grid::spawnPlayer( void )
{
	auto player = crimild::alloc< Group >();
	player->attachComponent< Player >();
	player->attachComponent< GridObject >( this );

	auto parent = getNode< Group >();
	parent->attachNode( player );
}

void Grid::spawnConsumable( crimild::Bool startComponents )
{
	auto consumable = crimild::alloc< Group >();
	consumable->attachComponent< Consumable >();
	consumable->attachComponent< GridObject >( this );
	
	auto parent = getNode< Group >();
	parent->attachNode( consumable );
	if ( startComponents ) {
		consumable->perform( UpdateRenderState() );
		consumable->perform( UpdateWorldState() );
		consumable->startComponents();
	}
}

