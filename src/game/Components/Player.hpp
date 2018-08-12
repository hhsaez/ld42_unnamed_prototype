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

#ifndef HUNGER_COMPONENTS_PLAYER_
#define HUNGER_COMPONENTS_PLAYER_

#include <Crimild.hpp>

namespace crimild {

	class TrailPositionParticleGenerator : public ParticleSystemComponent::ParticleGenerator {
		CRIMILD_IMPLEMENT_RTTI( crimild::TrailPositionParticleGenerator )

	private:
		using Trail = containers::Array< Vector3f >;
		
	public:
		TrailPositionParticleGenerator( void );
		virtual ~TrailPositionParticleGenerator( void );

		void setTrail( const Trail &trail ) { _trail = trail; }
		const Trail &getTrail( void ) const { return _trail; }

		virtual void configure( Node *node, ParticleData *particles ) override;
        virtual void generate( Node *node, crimild::Real64 dt, ParticleData *particles, ParticleId startId, ParticleId endId ) override;

	private:
		Trail _trail;
		
		ParticleAttribArray *_positions = nullptr;
	};

}

namespace hunger {

	class Player :
		public crimild::NodeComponent,
		public crimild::DynamicSingleton< Player >,
		public crimild::Messenger {
		CRIMILD_IMPLEMENT_RTTI( hunger::Player )

	public:
		Player( void );
		virtual ~Player( void );

		virtual void onAttach( void ) override;
		virtual void start( void ) override;
		virtual void update( const crimild::Clock &c ) override;

		crimild::Node *getHead( void ) { return _head; }

	private:
		crimild::Bool step( void );

	private:
		crimild::Real32 _t = 0.0f;
		crimild::Real32 _speed = 10.0f;

		enum class Direction {
			UP,
			DOWN,
			LEFT,
			RIGHT,
		};

		Direction _direction;

		struct TailNode {
			crimild::Vector2i pos;
			crimild::Node *node;
		};

		crimild::Node *_head = nullptr;
		crimild::containers::Queue< TailNode > _tail;

	private:
		void renderTail( void );

	private:
		crimild::Geometry *_renderer = nullptr;
		crimild::NodePositionParticleGenerator *_posGenerator = nullptr;
	};

}

#endif

