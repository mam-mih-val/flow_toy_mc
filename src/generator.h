#ifndef GENERATOR_H
#define GENERATOR_H

#include "paritlce_generator.h"
#include "random_number_generator.h"
#include "random_resources.h"
#include <algorithm>
#include <math.h>
#include <random>
#include <vector>
#include <iostream>

struct Collision{
  double reaction_plane{};
  std::vector<Particle> particles;
};


class Generator{
public:
  Generator() = default;
  
  auto GenerateCollision() -> Collision {
    auto& resources = request_();
    auto psi_rp = resources.SampleRp();
    Collision collision;
    collision.reaction_plane = psi_rp;
    for( auto& gen : resources.GetParticleGenerators() ){
      auto particles = gen.Sample();
      collision.particles.insert( collision.particles.end(), particles.begin(), particles.end() );
    }
    ConserveMomentum(collision);
    RotateZ(collision, psi_rp);
    return collision;
  }

  auto SetRequest( std::function< RandomResources& ()> func ){ request_ = std::move(func); }

private:
  auto ConserveMomentum( Collision& col ) const -> void{
    auto rez_x = double{};
    auto rez_y = double{};
    auto rez_z = double{};
    std::for_each( col.particles.begin(), col.particles.end(), [&rez_x, &rez_y, &rez_z]( const auto& p ) mutable{ 
      rez_x+=p.momentum.X();
      rez_y+=p.momentum.Y();
      rez_z+=p.momentum.Z();
    } );
    rez_x/=col.particles.size();
    rez_y/=col.particles.size();
    rez_z/=col.particles.size();
    std::for_each( col.particles.begin(), col.particles.end(), [&rez_x, &rez_y, &rez_z]( auto& p ) { 
      p.momentum.SetPx( p.momentum.X() - rez_x ); 
      p.momentum.SetPy( p.momentum.Y() - rez_y ); 
      p.momentum.SetPz( p.momentum.Z() - rez_z ); 
    } );
  }
  auto RotateZ( Collision& col , double angle ) const -> void{
    std::for_each( col.particles.begin(), col.particles.end(), [angle]( auto& p ) mutable{ 
      auto px = p.momentum.Px();
      auto py = p.momentum.Py();
      auto new_px = px * cos( angle ) - py*sin(angle);
      auto new_py = px * sin( angle ) + py*cos(angle);
      p.momentum.SetPx( new_px );
      p.momentum.SetPy( new_py );
    } );
  }


  std::function< RandomResources&() > request_{};
};

#endif // GENERATOR_H