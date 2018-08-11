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

#include "Consumable.hpp"
#include "Grid.hpp"
#include "GridObject.hpp"
#include "Player.hpp"

using namespace hunger;

using namespace crimild;

Consumable::Consumable( void )
{

}

Consumable::~Consumable( void )
{

}

void Consumable::onAttach( void )
{
	_size = Random::generate< crimild::Int32 >( 1, 6 );
	
	auto parent = getNode< Group >();
	
	auto g = crimild::alloc< Geometry >();
	g->attachPrimitive( crimild::alloc< SpherePrimitive >( _size ) );
	parent->attachNode( g );
	
	auto m = crimild::alloc< Material >();
	m->setDiffuse( RGBAColorf( 0.0f, 1.0f, 0.0f, 1.0f ) );
	g->getComponent< MaterialComponent >()->attachMaterial( m );
}

void Consumable::start( void )
{
	auto gridObject = getComponent< GridObject >();
	auto grid = gridObject->getGrid();
	auto gridPos = gridObject->getPosition();

	getNode()->local().setTranslate( grid->gridPosToWorld( gridPos ) );
}

void Consumable::update( const Clock & )
{
	auto player = Player::getInstance();
	auto playerHead = player->getHead();
	if ( playerHead == nullptr ) {
		return;
	}

	if ( getNode()->getWorldBound()->testIntersection( playerHead->getWorldBound() ) ) {
		broadcastMessage( messaging::ConsumableDestroyed { } );
		crimild::concurrency::sync_frame( [ this ] {
			getNode()->detachFromParent();
		});
	}
	
}

