#ifndef RANDOM_RESOURCES
#define RANDOM_RESOURCES

#include "paritlce_generator.h"
#include <vector>

class RandomResources{
public:
  RandomResources() = default;

  auto AddParticleGenerator( ParticlGeneratorSetup setup ) -> RandomResources&{ particle_generators_.emplace_back( std::move(setup) ); return *this; }
  auto GetParticleGenerators() -> std::vector<ParticleGenerator>& { return particle_generators_; }
  auto SampleRp() -> double { return psi_rp_generator_.SampleNumber(); }
  template<typename Obj>
  auto Register( Obj& obj ){ 
    obj.SetRequest( [this]() -> RandomResources& { return *this; } );
  }

private:
  RandomNumberGenerator< std::uniform_real_distribution<double> > psi_rp_generator_{std::uniform_real_distribution<double>{ -M_PI, M_PI }};
  std::vector<ParticleGenerator> particle_generators_{};
};

#endif // RANDOM_RESOURCES
