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

#ifndef HUNGER_COMPONENTS_GRID_
#define HUNGER_COMPONENTS_GRID_

#include <Crimild.hpp>

namespace hunger {

	class Grid :
		public crimild::NodeComponent,
		public crimild::Messenger {
		CRIMILD_IMPLEMENT_RTTI( hunger::Grid )
		
	public:
		Grid( crimild::Int32 width, crimild::Int32 height );
		virtual ~Grid( void );

		virtual void onAttach( void ) override;
		virtual void start( void ) override;
		
		crimild::Int32 getWidth( void ) const { return _width; }
		crimild::Int32 getHeight( void ) const { return _height; }
		
		crimild::Bool isEmpty( crimild::Vector2i pos ) const;
		void setEmpty( crimild::Vector2i pos, crimild::Bool empty );
		
		crimild::Bool move( crimild::Vector2i &pos );
		
		crimild::Vector3f gridPosToWorld( const crimild::Vector2i gridPos ) const;
		
	private:
		void spawnPlayer( void );
		void spawnConsumable( crimild::Bool startComponents = true );
		
	private:
		crimild::Int32 _width;
		crimild::Int32 _height;
		crimild::containers::Array< crimild::Bool > _state;
	};
	
}

#endif

